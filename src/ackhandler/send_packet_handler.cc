#include "ackhandler/packet.h"
#include "ackhandler/send_packet_handler.h"
#include "frame/ack_frame.h"
#include <algorithm>

kuic::ackhandler::send_packet_handler::send_packet_handler(kuic::congestion::rtt &_rtt)
    : last_sent_packet_number(0)
    , last_sent_retransmittable_packet_time(kuic::special_clock({ 0, 0 }))
    , last_sent_handshake_packet_time(kuic::special_clock({ 0, 0 }))
    , next_packet_send_time(kuic::special_clock({ 0, 0 }))
    , largest_acked(0)
    , largest_received_packet_with_ack(0)
    , lowest_packet_not_confirmed_acked(0)
    , largest_sent_before_rto(0)
    , bytes_in_flight(0)
    , congestion(kuic::congestion::cubic_sender(
                _rtt,
                kuic::initial_congestion_window,
                kuic::default_max_congestion_window))
    , _rtt(_rtt)
    , handshake_complete(false)
    , handshake_count(0)
    , tlp_count(0)
    , allow_tlp(false)
    , rto_count(0)
    , num_rtos(0)
    , loss_time(kuic::special_clock({ 0, 0 }))
    , alarm(kuic::special_clock({ 0, 0 })) { }

kuic::packet_number_t
kuic::ackhandler::send_packet_handler::lowest_unacked() const {
    const kuic::ackhandler::packet *packet = this->packet_history.get_first_outstanding(); 
    if (packet != nullptr) {
        return packet->packet_number;
    }
    return this->largest_acked + 1;
}

void kuic::ackhandler::send_packet_handler::set_handshake_complete() {
    std::list<std::shared_ptr<kuic::ackhandler::packet>> queue;

    std::for_each(this->retransmission_queue.begin(), this->retransmission_queue.end(),
            [&] (const std::shared_ptr<kuic::ackhandler::packet> &packet) -> void {
                if (packet->is_handshake == false) {
                    queue.push_back(packet);
                }
            });
    
    std::list<kuic::ackhandler::packet> handshake_queue;
    this->packet_history.iterate(
            [&] (const kuic::ackhandler::packet &packet) -> bool {
                if (packet.is_handshake) {
                    handshake_queue.push_back(packet);
                }
                return true;
            });
    
    std::for_each(handshake_queue.begin(), handshake_queue.end(),
            [this] (const kuic::ackhandler::packet &packet) -> void {
                this->packet_history.remove(packet.packet_number);
            });
    this->retransmission_queue = queue;
    this->handshake_complete = true;
}

void kuic::ackhandler::send_packet_handler::sent_packet(kuic::ackhandler::packet &packet) {
    if (this->sent_packet_implement(packet)) {
        this->packet_history.send_packet(packet);
        this->update_loss_detection_alarm();
    }
}

bool kuic::ackhandler::send_packet_handler::sent_packet_implement(kuic::ackhandler::packet &packet) {
    for (kuic::packet_number_t p = this->last_sent_packet_number + 1; p < packet.packet_number; p++) {
        this->skipped_packets.push_back(p);
        if (this->skipped_packets.size() > kuic::max_tracked_skipped_packets) {
            this->skipped_packets.pop_front();
        }
    }

    this->last_sent_packet_number = packet.packet_number;

    if (packet.frames.empty() == false) {
        if (packet.frames[0]->type() == kuic::frame_type_ack) {
            packet.largest_acked = reinterpret_cast<kuic::frame::ack_frame *>(packet.frames[0].get())->largest_acked();
        }
    }

    packet.frames = kuic::ackhandler::strip_non_retransmittable_frames(packet.frames);

    bool is_retransmittable = packet.frames.empty() == false;

    if (is_retransmittable) {
        if (packet.is_handshake) {
            this->last_sent_handshake_packet_time = packet.send_time;
        }
        this->last_sent_retransmittable_packet_time = packet.send_time;
        packet.included_in_bytes_in_flight = true;
        this->bytes_in_flight += packet.length;
        packet.can_be_retransmitted = true;
        if (this->num_rtos > 0) {
            this->num_rtos--;
        }
        this->allow_tlp = false;
    }

    this->congestion.on_packet_sent(packet.packet_number, packet.length, is_retransmittable);

    if (packet.send_time > this->next_packet_send_time) {
        this->next_packet_send_time = packet.send_time;
    }
    this->next_packet_send_time += this->congestion.time_until_send(this->bytes_in_flight);

    return is_retransmittable;
}

bool kuic::ackhandler::send_packet_handler::received_ack(
        kuic::frame::ack_frame &ack_frame, kuic::packet_number_t with_packet_number, bool is_handshake, kuic::special_clock rcv_time) {
    kuic::packet_number_t largest_acked = ack_frame.largest_acked();
    if (largest_acked > this->last_sent_packet_number) {
        return false;
    }
    
    if (with_packet_number != 0 && with_packet_number <= this->largest_received_packet_with_ack) {
        return true;
    }

    this->largest_received_packet_with_ack = with_packet_number;
    this->largest_acked = std::max(this->largest_acked, largest_acked);

    if (this->skipped_packets_acked(ack_frame)) {
        return false; 
    }

    if (this->maybe_update_rtt(largest_acked, ack_frame.get_delay_time(), rcv_time)) {
        this->congestion.try_exit_slowstart();
    }

    std::list<kuic::ackhandler::packet> ack_packets = this->determine_newly_acked_packets(ack_frame);
    
    kuic::bytes_count_t prior_in_flight = this->bytes_in_flight;
    for (std::list<kuic::ackhandler::packet>::iterator packet_iterator = ack_packets.begin();
            packet_iterator != ack_packets.end();
            packet_iterator++) {

        if (is_handshake && packet_iterator->is_handshake == false) {
            return false;
        }
        if (packet_iterator->largest_acked != 0) {
            this->lowest_packet_not_confirmed_acked = std::max(this->lowest_packet_not_confirmed_acked, this->largest_acked + 1);
        }
        if (this->on_packet_acked(*packet_iterator) == false) {
            return false;
        }
        if (packet_iterator->included_in_bytes_in_flight) {
            this->congestion.on_packet_acked(packet_iterator->packet_number, packet_iterator->length, prior_in_flight, rcv_time);
        }
    }

    if (this->detect_lost_packets(rcv_time, prior_in_flight) == false) {
        return false;
    }

    this->update_loss_detection_alarm();

    this->garbage_collect_skipped_packets();
    return true;
}

kuic::packet_number_t kuic::ackhandler::send_packet_handler::get_lowest_packet_not_confirmd_acked() {
    return this->lowest_packet_not_confirmed_acked;
}

std::list<kuic::ackhandler::packet>
kuic::ackhandler::send_packet_handler::determine_newly_acked_packets(kuic::frame::ack_frame &ack_frame) {
    std::list<kuic::ackhandler::packet> acked_packet;
    size_t ack_range_index = 0;
    kuic::packet_number_t lowest_acked = ack_frame.lowest_acked();
    kuic::packet_number_t largest_acked = ack_frame.largest_acked();

    this->packet_history.iterate(
            [&] (const kuic::ackhandler::packet &packet) -> bool {
                if (packet.packet_number < lowest_acked) {
                    return true;
                }
                if (packet.packet_number > largest_acked) {
                    return false;
                }

                if (ack_frame.has_missing_ranges()) {
                    std::pair<kuic::packet_number_t, kuic::packet_number_t> range = 
                        *(ack_frame.get_ranges().rbegin() + ack_range_index);

                    while (packet.packet_number > range.second && ack_range_index < ack_frame.get_ranges().size() - 1) {
                        ack_range_index++;
                        range = *(ack_frame.get_ranges().rbegin() + ack_range_index);
                    }

                    if (packet.packet_number >= range.first) {
                        if (packet.packet_number > range.second) {
                            return false;
                        }
                        acked_packet.push_back(packet);
                    }
                }
                else {
                    acked_packet.push_back(packet);
                }
                return true;
            });

    return acked_packet;
}

bool kuic::ackhandler::send_packet_handler::maybe_update_rtt(
        kuic::packet_number_t largest_acked, kuic::kuic_time_t ack_delay, kuic::special_clock rcv_time) {
    auto p = this->packet_history.get_packet(largest_acked);
    if (p != nullptr) {
        this->_rtt.update_rtt(rcv_time - p->send_time, ack_delay);
        return true;
    }
    return false;
}

void kuic::ackhandler::send_packet_handler::update_loss_detection_alarm() {
    if (this->packet_history.size() == 0) {
        this->alarm = kuic::special_clock({ 0, 0 });
        return;
    }

    if (this->handshake_complete == false) {
        this->alarm = this->last_sent_handshake_packet_time + this->compute_handshake_timeout();
    }
    else if (this->loss_time.is_zero() == false) {
        this->alarm = this->loss_time;
    }
    else {
        kuic::kuic_time_t alarm_duration = this->compute_rto_timeout();
        if (this->tlp_count < kuic::ackhandler::max_tlps) {
            kuic::kuic_time_t tlp_alarm = this->compute_tlp_timeout();
            alarm_duration = std::min(alarm_duration, tlp_alarm);
        }
        this->alarm = this->last_sent_retransmittable_packet_time + alarm_duration;
    }
}

bool kuic::ackhandler::send_packet_handler::detect_lost_packets(kuic::special_clock now, kuic::bytes_count_t prior_in_flight) {
    this->loss_time = kuic::special_clock({ 0, 0 });
    
    double max_rtt = double(std::max(this->_rtt.get_latest_rtt(), this->_rtt.get_smoothed_rtt()));
    kuic::kuic_time_t delay_until_lost = kuic::kuic_time_t((1.0 + kuic::ackhandler::time_reordering_fraction) * max_rtt);

    std::list<kuic::ackhandler::packet> lost_packets;
    this->packet_history.iterate(
            [&, this] (const kuic::ackhandler::packet &packet) -> bool {

                if (packet.packet_number > this->largest_acked) {
                    return false;
                }

                kuic::kuic_time_t time_since_sent = now - packet.send_time;

                if (time_since_sent > delay_until_lost) {
                    lost_packets.push_back(packet);
                }
                else if (this->loss_time.is_zero()) {
                    this->loss_time = now + (delay_until_lost - time_since_sent);
                }
                return true;
            });

    std::for_each(lost_packets.begin(), lost_packets.end(), 
            [&, this] (const kuic::ackhandler::packet &packet) -> void {
                if (packet.included_in_bytes_in_flight) {
                    this->bytes_in_flight -= packet.length;

                    this->congestion.on_packet_lost(packet.packet_number, packet.length, prior_in_flight);
                }
                if (packet.can_be_retransmitted) {
                    this->queue_packet_for_retransmission(packet);
                }
                this->packet_history.remove(packet.packet_number);
            });
    return true;
}

bool kuic::ackhandler::send_packet_handler::on_alarm() {
    kuic::current_clock current;
    kuic::special_clock now(current);

    if (this->handshake_complete == false) {
        this->handshake_count++;
        if (this->queue_handshake_packets_for_retransmission() == false) {
            return false;
        }
    }
    else if (this->loss_time.is_zero() == false) {
        if (this->detect_lost_packets(now, this->bytes_in_flight) == false) {
            return false;
        }
    }
    else if (this->tlp_count < kuic::ackhandler::max_tlps) {
        this->allow_tlp = true;
        this->tlp_count++;
    }
    else {
        this->rto_count++;
        this->num_rtos += 2;
        if (this->queue_rtos() == false) {
            return false;
        }
    }

    this->update_loss_detection_alarm();
    return true;
}

kuic::special_clock
kuic::ackhandler::send_packet_handler::get_alarm_timeout() {
    return this->alarm;
}

bool kuic::ackhandler::send_packet_handler::on_packet_acked(kuic::ackhandler::packet &p) {
    
    const kuic::ackhandler::packet *packet = this->packet_history.get_packet(p.packet_number);
    if (packet == nullptr) {
        return true;
    }

    if (p.is_retransmission) {
        const kuic::ackhandler::packet *parent = this->packet_history.get_packet(p.retransmission_of);
        if (parent != nullptr) {
            if (parent->retransmitted_as.size() == 1) {
                const_cast<kuic::ackhandler::packet *>(parent)->retransmitted_as.clear();
            }
            else {
                std::vector<kuic::packet_number_t> retransmitted_as(
                        parent->retransmitted_as.size() - 1, kuic::packet_number_t(0));

                std::for_each(parent->retransmitted_as.begin(), parent->retransmitted_as.end(),
                        [&] (const kuic::packet_number_t &packet_number) -> void {
                            if (packet_number != p.packet_number) {
                                retransmitted_as.push_back(packet_number);
                            }
                        });

                const_cast<kuic::ackhandler::packet *>(parent)->retransmitted_as = retransmitted_as;
            }
        }
    }

    if (p.included_in_bytes_in_flight) {
        this->bytes_in_flight -= p.length;
    }

    if (this->rto_count > 0) {
        this->verify_rto(p.packet_number);
    }

    if (this->stop_retransmission_for(p) == false) {
        return false;
    }

    this->rto_count = 0;
    this->tlp_count = 0;
    this->handshake_count = 0;

    this->packet_history.remove(p.packet_number);
    return true;
}

bool kuic::ackhandler::send_packet_handler::stop_retransmission_for(
        const kuic::ackhandler::packet &packet) {
    this->packet_history.mark_cannot_be_retransmitted(packet.packet_number);
    std::for_each(packet.retransmitted_as.begin(), packet.retransmitted_as.end(),
            [&] (const kuic::packet_number_t &packet_number) -> void {
                const kuic::ackhandler::packet *packet = this->packet_history.get_packet(packet_number);
                if (packet == nullptr) {
                    return;
                }

                this->stop_retransmission_for(*packet);
            });
    return true;
}

void kuic::ackhandler::send_packet_handler::verify_rto(kuic::packet_number_t packet_number) {
    if (packet_number <= this->largest_sent_before_rto) {
        this->_rtt.expire_smoothed_metrics();

        return;
    }

    this->congestion.on_retransmission_timeout(true);
}

std::shared_ptr<kuic::ackhandler::packet>
kuic::ackhandler::send_packet_handler::dequeue_packet_for_retransmission() {
    if (this->retransmission_queue.empty()) {
        return std::shared_ptr<kuic::ackhandler::packet>();
    }
    std::shared_ptr<kuic::ackhandler::packet> result(this->retransmission_queue.front());
    this->retransmission_queue.pop_front();
    return result;
}

size_t kuic::ackhandler::send_packet_handler::get_packet_number_length(kuic::packet_number_t p) {
    return __inl_packet_number_length_for_header(p, this->lowest_unacked());
}

kuic::send_mode_t
kuic::ackhandler::send_packet_handler::get_send_mode() {
    int num_tracked_packets = this->retransmission_queue.size() + this->packet_history.size();

    if (num_tracked_packets >= kuic::max_tracked_sent_packets) {
        return kuic::send_mode_none;
    }

    if (this->allow_tlp) {
        return kuic::send_mode_tlp;
    }

    if (this->num_rtos > 0) {
        return kuic::send_mode_rto;
    }

    kuic::bytes_count_t congestion_window = this->congestion.get_congestion_window();

    if (this->bytes_in_flight > congestion_window) {
        return kuic::send_mode_ack;
    }

    if (this->retransmission_queue.empty() == false) {
        return kuic::send_mode_retransmission;
    }


    if (num_tracked_packets >= kuic::max_outstanding_sent_packets) {
        return kuic::send_mode_ack;
    }

    return kuic::send_mode_any;
}

kuic::special_clock kuic::ackhandler::send_packet_handler::time_until_send() {
    return this->next_packet_send_time;
}

int kuic::ackhandler::send_packet_handler::should_send_num_packets() {
    if (this->num_rtos > 0) {
        return this->num_rtos;
    }

    kuic::kuic_time_t delay = this->congestion.time_until_send(this->bytes_in_flight);
    if (delay == 0 || delay > kuic::min_pacing_delay) {
        return 1;
    }

    return int(std::ceil(double(kuic::min_pacing_delay) / double(delay)));
}

bool kuic::ackhandler::send_packet_handler::queue_rtos() {
    this->largest_sent_before_rto = this->last_sent_packet_number;

    for (int i = 0; i < 2; i++) {
        const kuic::ackhandler::packet *p = this->packet_history.get_first_outstanding();
        if (p != nullptr) {
            if (this->queue_packet_for_retransmission(*p) == false) {
                return false;
            }
        }
    }
    return true;
}

bool kuic::ackhandler::send_packet_handler::queue_handshake_packets_for_retransmission() {
    std::list<kuic::ackhandler::packet> handshake_packets;
    this->packet_history.iterate(
            [&] (const kuic::ackhandler::packet &packet) -> bool {
                if (packet.can_be_retransmitted && packet.is_handshake) {
                    handshake_packets.push_back(packet);
                }
                return true;
            });
    for(std::list<kuic::ackhandler::packet>::iterator p_itr = handshake_packets.begin();
            p_itr != handshake_packets.end();
            p_itr++) {
        if (this->queue_packet_for_retransmission(*p_itr) == false) {
            return false;
        }
    }
    return true;
}

bool kuic::ackhandler::send_packet_handler::queue_packet_for_retransmission(
        const kuic::ackhandler::packet &packet) {
    if (packet.can_be_retransmitted == false) {
        return false;
    }

    this->packet_history.mark_cannot_be_retransmitted(packet.packet_number);
    
    this->retransmission_queue.push_back(
            std::make_shared<kuic::ackhandler::packet>(packet));
    return true;
}

kuic::kuic_time_t kuic::ackhandler::send_packet_handler::compute_handshake_timeout() {
    kuic::kuic_time_t duration = std::max(
            2 * this->_rtt.get_smoothed_or_initial_rtt(), kuic::ackhandler::min_tlp_timeout);

    return duration << this->handshake_count;
}

kuic::kuic_time_t kuic::ackhandler::send_packet_handler::compute_tlp_timeout() {
    return std::max(this->_rtt.get_smoothed_or_initial_rtt() * 3 / 2 , kuic::ackhandler::min_tlp_timeout);
}

kuic::kuic_time_t kuic::ackhandler::send_packet_handler::compute_rto_timeout() {
    kuic::kuic_time_t rto = 0;
    kuic::kuic_time_t rtt = this->_rtt.get_smoothed_rtt();
    if (rtt == 0) {
        rto = kuic::ackhandler::default_rto_timeout;
    }
    else {
        rto = rtt + 4 * this->_rtt.get_mean_deviation();
    }
    rto = std::max(rto, kuic::ackhandler::min_rto_timeout);
    rto <<= this->rto_count;
    return std::min(rto, kuic::ackhandler::max_rto_timeout);
}

bool kuic::ackhandler::send_packet_handler::skipped_packets_acked(kuic::frame::ack_frame &ack_frame) {
    for (std::list<kuic::packet_number_t>::iterator p_itr = this->skipped_packets.begin();
            p_itr != this->skipped_packets.end();
            p_itr++) {
        if (ack_frame.acks_packet(*p_itr)) {
            return true;
        }
    }
    return false;
}

void kuic::ackhandler::send_packet_handler::garbage_collect_skipped_packets() {
    kuic::packet_number_t lowest_unacked = this->lowest_unacked();
    int delete_index = 0;
    int i = 0;
    for (std::list<kuic::packet_number_t>::iterator p_itr = this->skipped_packets.begin();
            p_itr != this->skipped_packets.end();
            p_itr++) {
        if (*p_itr < lowest_unacked) {
            delete_index = i + 1;
        }
        i++;
    }
    while (delete_index--) {
        this->skipped_packets.pop_front();
    }
}

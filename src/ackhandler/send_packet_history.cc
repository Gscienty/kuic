#include "ackhandler/send_packet_history.h"

kuic::ackhandler::send_packet_history::send_packet_history()
    : first_outstanding(0) { }

std::list<kuic::ackhandler::packet>::iterator
kuic::ackhandler::send_packet_history::send_packet_implement(const kuic::ackhandler::packet &packet) {
    this->packet_list.push_back(packet); 
    std::list<kuic::ackhandler::packet>::iterator packet_iterator = this->packet_list.end();
    packet_iterator--;

    this->packet_map[packet.packet_number] = packet_iterator;

    if (this->first_outstanding == 0) {
        this->first_outstanding = packet.packet_number;
    }

    return packet_iterator;
}

void kuic::ackhandler::send_packet_history::send_packet(const kuic::ackhandler::packet &packet) {
    this->send_packet_implement(packet);
}

kuic::nullable<kuic::ackhandler::packet>
kuic::ackhandler::send_packet_history::get_packet(kuic::packet_number_t packet_number) {
    auto iter = this->packet_map.find(packet_number);
    if (iter != this->packet_map.end()) {
        return kuic::nullable<kuic::ackhandler::packet>(*iter->second);
    }
    return kuic::nullable<kuic::ackhandler::packet>(nullptr);
}

kuic::nullable<kuic::ackhandler::packet>
kuic::ackhandler::send_packet_history::get_first_outstanding() const {
    if (this->first_outstanding == 0) {
        return kuic::nullable<kuic::ackhandler::packet>(nullptr);
    }
    
    kuic::nullable<kuic::ackhandler::packet> result(
            new kuic::ackhandler::packet(*(this->packet_map.find(this->first_outstanding)->second)));
    return result;
}

void kuic::ackhandler::send_packet_history::mark_cannot_be_retransmitted(kuic::packet_number_t packet_number) {
    auto packet_index_iterator = this->packet_map.find(packet_number);
    if (packet_index_iterator == this->packet_map.end()) {
        return;
    }

    packet_index_iterator->second->can_be_retransmitted = false;
    if (packet_index_iterator->first == this->first_outstanding) {
        this->readjust_first_outstanding();
    }
}

void kuic::ackhandler::send_packet_history::readjust_first_outstanding() {
    if (this->first_outstanding == 0) {
        return;
    }

    std::list<kuic::ackhandler::packet>::iterator iter = this->packet_map[this->first_outstanding];
    iter++;
    while (iter != this->packet_list.end() && iter->can_be_retransmitted) {
        iter++;
    }
    if (iter != this->packet_list.end()) {
        this->first_outstanding = iter->packet_number;
    }
    else {
        this->first_outstanding = 0;
    }
}

size_t
kuic::ackhandler::send_packet_history::size() const {
    return this->packet_map.size();
}

void kuic::ackhandler::send_packet_history::remove(kuic::packet_number_t packet_number) {
    auto packet_index_iterator = this->packet_map.find(packet_number);

    if (packet_index_iterator == this->packet_map.end()) {
        return;
    }

    if (packet_index_iterator->first == this->first_outstanding) {
        this->readjust_first_outstanding();
    }
    
    this->packet_list.erase(packet_index_iterator->second);
    this->packet_map.erase(packet_index_iterator);
}

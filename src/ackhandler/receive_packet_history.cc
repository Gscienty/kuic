#include "ackhandler/receive_packet_history.h"
#include "define.h"

bool kuic::ackhandler::receive_packet_history::received_packet(kuic::packet_number_t packet_number) {
    if (this->ranges.size() >= kuic::max_tracked_received_ack_ranges) {
        return false;
    }

    if (this->ranges.empty()) {
        this->ranges.push_back(
                std::pair<kuic::packet_number_t, kuic::packet_number_t>(packet_number, packet_number));
        return true;
    }

    for (auto element_iter = this->ranges.rbegin(); element_iter != this->ranges.rend(); element_iter++) {
        if (packet_number >= element_iter->first && packet_number <= element_iter->second) {
            return  true;
        }

        bool range_extended = false;
        if (element_iter->second == packet_number - 1) {
            range_extended = true;
            element_iter->second = packet_number;
        }
        else if (element_iter->first == packet_number + 1) {
            range_extended = true;
            element_iter->first = packet_number;
        }

        if (range_extended) {
            auto prev = element_iter + 1;
            if (prev != this->ranges.rend() && prev->second + 1 == element_iter->first) {
                prev->second = element_iter->second;
                this->ranges.erase(
                        std::list<std::pair<kuic::packet_number_t, kuic::packet_number_t>>::iterator(element_iter.base()));
            }
            return true;
        }

        if (packet_number > element_iter->second) {
            this->ranges.insert(
                    std::list<std::pair<kuic::packet_number_t, kuic::packet_number_t>>::iterator(element_iter.base()),
                    std::pair<kuic::packet_number_t, kuic::packet_number_t>(packet_number, packet_number));
            return true;
        }
    }

    this->ranges.push_front(std::pair<kuic::packet_number_t, kuic::packet_number_t>(packet_number, packet_number));

    return true;
}

void kuic::ackhandler::receive_packet_history::delete_below(kuic::packet_number_t packet_number) {
    if (packet_number <= this->lowest_in_received_packet_numbers) {
        return;
    }

    this->lowest_in_received_packet_numbers = packet_number;

    std::list<std::pair<kuic::packet_number_t, kuic::packet_number_t>>::iterator next_element = this->ranges.begin();
    for (std::list<std::pair<kuic::packet_number_t, kuic::packet_number_t>>::iterator element = this->ranges.begin();
            next_element != this->ranges.end();
            element = next_element) {
        next_element = element;
        next_element++;

        if (packet_number > element->first && packet_number <= element->second) {
            element->first = packet_number;
        }
        else if (element->second < packet_number) {
            this->ranges.erase(element);
        }
        else {
            return;
        }
    }

}

std::vector<std::pair<kuic::packet_number_t, kuic::packet_number_t>> 
kuic::ackhandler::receive_packet_history::get_ack_ranges() {
    if (this->ranges.empty()) {
        return std::vector<std::pair<kuic::packet_number_t, kuic::packet_number_t>>();
    }

    std::vector<std::pair<kuic::packet_number_t, kuic::packet_number_t>> result;
    for (auto element_iter = this->ranges.rbegin(); element_iter != this->ranges.rend(); element_iter++) {
        result.push_back(*element_iter);
    }
    return result;
}

std::pair<kuic::packet_number_t, kuic::packet_number_t>
kuic::ackhandler::receive_packet_history::get_highest_ack_range() {
    std::pair<kuic::packet_number_t, kuic::packet_number_t> result(0, 0);
    if (this->ranges.empty() == false) {
        result.first = this->ranges.rbegin()->first;
        result.second = this->ranges.rbegin()->second;
    }

    return result;
}

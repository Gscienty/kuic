#include "frame/ack_frame.h"
#include "variable_integer.h"
#include "define.h"
#include <tuple>

kuic::frame_type_t
kuic::frame::ack_frame::type() const {
    return kuic::frame_type_ack;
}

kuic::frame::ack_frame
kuic::frame::ack_frame::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    seek++; // ignore frame type byte (ACK)

    kuic::frame::ack_frame frame;
    kuic::packet_number_t largest_acked = kuic::packet_number_t(kuic::variable_integer::read(buffer, len, seek));
    kuic::kuic_time_t delay = kuic::kuic_time_t(kuic::variable_integer::read(buffer, len, seek));

    frame.delay_time = (delay * 1 << kuic::frame::ack_delay_exponent) * kuic::clock_microsecond;
    int blocks_count = kuic::variable_integer::read(buffer, len, seek);

    // read first ack range
    kuic::packet_number_t block = kuic::variable_integer::read(buffer, len, seek);
    if (block > largest_acked) {
        return kuic::frame::ack_frame(kuic::invalid_value);
    }

    kuic::packet_number_t smallest = largest_acked - block;

    frame.ranges.push_back(std::pair<kuic::packet_number_t, kuic::packet_number_t>(smallest, largest_acked));
    for (int i = 0; i < blocks_count; i++) {
        kuic::packet_number_t gap = kuic::variable_integer::read(buffer, len, seek);
        
        if (smallest < gap + 2) {
            return kuic::frame::ack_frame(kuic::invalid_ack_ranges);
        }

        kuic::packet_number_t largest = smallest - gap - 2;

        kuic::packet_number_t block = kuic::variable_integer::read(buffer, len, seek);
        if (block > largest) {
            return kuic::frame::ack_frame(kuic::invalid_ack_ranges);
        }

        smallest = largest - block;
        frame.ranges.push_back(std::pair<kuic::packet_number_t, kuic::packet_number_t>(smallest, largest));
    }

    return frame;
}

bool kuic::frame::ack_frame::has_missing_ranges() const {
    return this->ranges.size() > 1;
}

std::pair<kuic::byte_t *, size_t>
kuic::frame::ack_frame::serialize() const {
    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *result = new kuic::byte_t[size];

    result[seek++] = 0x0D;
    kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->largest_acked()));
    kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->encode_ack_delay(this->delay_time)));

    int ranges_count = this->encodable_ack_ranges_count();
    kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(ranges_count - 1));

    unsigned long first_range;
    std::tie(std::ignore, first_range) = this->encode_ack_range(0);
    kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(first_range));

    for (int i = 1; i < ranges_count; i++) {
        unsigned long gap;
        unsigned long len;
        std::tie(gap, len) = this->encode_ack_range(i);
        kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(gap));
        kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(len));
    }

    return std::pair<kuic::byte_t *, size_t>(result, size);
}

size_t kuic::frame::ack_frame::length() const {
    kuic::bytes_count_t largest_acked = this->ranges[0].second;
    int ranges_count = this->encodable_ack_ranges_count();

    unsigned long length = 1 + 
        kuic::variable_integer::length(largest_acked) + 
        kuic::variable_integer::length(this->encode_ack_delay(this->delay_time));

    length += kuic::variable_integer::length(ranges_count - 1);
    kuic::bytes_count_t lowest_in_first_range = this->ranges[0].first;
    length += kuic::variable_integer::length(largest_acked - lowest_in_first_range);

    for (int i = 1; i < ranges_count; i++) {
        unsigned long gap;
        unsigned long len;
        std::tie(gap, len) = this->encode_ack_range(i);
        length += kuic::variable_integer::length(gap);
        length += kuic::variable_integer::length(len);
    }

    return length;
}

int kuic::frame::ack_frame::encodable_ack_ranges_count() const {
    unsigned long length = 1 + 
        kuic::variable_integer::length(this->largest_acked()) + 
        kuic::variable_integer::length(this->encode_ack_delay(this->delay_time));
    length += 2;
    for (size_t i = 0; i < this->ranges.size(); i++) {
        unsigned long gap;
        unsigned long len;
        std::tie(gap, len) = this->encode_ack_range(i);
        kuic::bytes_count_t range_length = kuic::variable_integer::length(gap) + kuic::variable_integer::length(len);
        if (length + range_length > kuic::max_ack_frame_size) {
            return i - 1;
        }
        length += range_length;
    }

    return this->ranges.size();
}

std::pair<unsigned long, unsigned long> kuic::frame::ack_frame::encode_ack_range(int index) const {
    if (index == 0) {
        return std::pair<unsigned long, unsigned long>(
                0, (unsigned long)(this->ranges.front().second - this->ranges.front().first));
    }
    
    return std::pair<unsigned long, unsigned long>(
            (unsigned long)(this->ranges[index - 1].first - this->ranges[index].second - 2),
            (unsigned long)(this->ranges[index].second - this->ranges[index].first));
}

kuic::packet_number_t kuic::frame::ack_frame::largest_acked() const {
    return this->ranges.begin()->second;
}

kuic::packet_number_t kuic::frame::ack_frame::lowest_acked() const {
    return this->ranges.rbegin()->first;
}

bool kuic::frame::ack_frame::acks_packet(kuic::packet_number_t p) const {
    if (p < this->lowest_acked() || p > this->largest_acked()) {
        return false;
    }

    auto index = std::find_if(
            this->ranges.begin(),
            this->ranges.end(),
            [&](const std::pair<kuic::packet_number_t, kuic::packet_number_t> &range) -> bool {
                return p >= range.first;
            });

    return p <= index->second;
}

unsigned long kuic::frame::ack_frame::encode_ack_delay(kuic::kuic_time_t delay) const {
    return (unsigned long)(delay / (1000 * (1 << kuic::frame::ack_delay_exponent)));
}

kuic::kuic_time_t kuic::frame::ack_frame::get_delay_time() const {
    return this->delay_time;
}

std::vector<std::pair<kuic::packet_number_t, kuic::packet_number_t>> &
kuic::frame::ack_frame::get_ranges() {
    return this->ranges;
}

#include "connection_id.h"
#include <random>
#include <algorithm>

kuic::connection_id
kuic::connection_id::generate_connection_id() {
    kuic::connection_id result;
    std::random_device rd;
    std::for_each(result.value, result.value + 8,
            [&] (kuic::byte_t &unit_value) -> void {
                unit_value = kuic::byte_t(rd());
            });

    return result;
}

kuic::connection_id
kuic::connection_id::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek, size_t connection_id_length) {
    if (len - seek < connection_id_length) {
        return kuic::connection_id(kuic::reader_buffer_remain_not_enough);
    }

    kuic::connection_id result;
    std::copy(buffer + seek, buffer + seek + connection_id_length, result.value);
    seek += connection_id_length;

    return result;
}

bool kuic::connection_id::operator==(const kuic::connection_id &other_connection_id) {
    for (int i = 0; i < 8; i++) {
        if (this->value[i] != other_connection_id.value[i]) {
            return false;
        }
    }
    return true;
}

const kuic::byte_t *
kuic::connection_id::bytes() const {
    return this->value;
}

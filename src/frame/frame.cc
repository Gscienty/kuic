#include "frame/frame.h"
#include <memory>
#include <tuple>
#include <algorithm>

void kuic::frame::frame::fill(
        kuic::byte_t *buffer, size_t size, size_t &seek, std::pair<kuic::byte_t *, size_t> value) {
    std::unique_ptr<kuic::byte_t> value_buffer(value.first);
    size_t value_length;
    std::tie(std::ignore, value_length) = value;

    if (size - seek < value_length) {
        return;
    }
    
    std::copy(value_buffer.get(), value_buffer.get() + value_length, buffer + seek);
    seek += value_length;
}

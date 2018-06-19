#include "variable_integer.h"
#include "eys.h"

unsigned long
kuic::variable_integer::read(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::byte_t first_byte = buffer[seek++];
    if (seek > buffer.size()) { return 0; }

    kuic::byte_t length = 1 << ((first_byte & 0xC0) >> 6);
    unsigned long result = first_byte & (0xFF - 0xC0);
    
    if (buffer.size() - seek < length - 1) { return 0; }
    while (--length) {
        result = (result << 8) + (unsigned long)(buffer[seek++]);
    }

    return result;
}

std::basic_string<kuic::byte_t> 
kuic::variable_integer::write(unsigned long value) {
    if (value <= kuic::variable_integer_max_1) {
        return eys::bigendian_serializer<kuic::byte_t, kuic::byte_t>::serialize(kuic::byte_t(value));
    }
    else if (value <= kuic::variable_integer_max_2) {
        std::basic_string<kuic::byte_t> result = 
            eys::bigendian_serializer<kuic::byte_t, unsigned short>::serialize((unsigned short)(value));
        result[0] |= 0x40;
        return result;
    }
    else if (value <= kuic::variable_integer_max_4) {
        std::basic_string<kuic::byte_t> result = 
            eys::bigendian_serializer<kuic::byte_t, unsigned int>::serialize((unsigned int)(value));
        result[0] |= 0x80;
        return result;
    }
    else if (value <= kuic::variable_integer_max_8) {
        std::basic_string<kuic::byte_t> result =
            eys::bigendian_serializer<kuic::byte_t, unsigned long>::serialize(value);
        result[0] |= 0xC0;
        return result;
    }
    else {
        return std::basic_string<kuic::byte_t>(); 
    }
}

size_t kuic::variable_integer::length(unsigned long value) {
    if (value <= kuic::variable_integer_max_1) {
        return 1;
    }
    else if (value <= kuic::variable_integer_max_2) {
        return 2;
    }
    else if (value <= kuic::variable_integer_max_4) {
        return 4;
    }
    else if (value <= kuic::variable_integer_max_8) {
        return 8;
    }
    else {
        return 0;
    }
}

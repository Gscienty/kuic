#ifndef _KUIC_LITTLE_ENDIAN_SERIALIZER_
#define _KUIC_LITTLE_ENDIAN_SERIALIZER_

#include <memory>
#include <utility>

namespace kuic {
    
    template<class T>
    inline char *__inl_little_endian_serialize(T &e) {
        char *buffer = new char[sizeof(e)];
        for (int i = 0; i < sizeof(T); i++) {
            buffer[i] = reinterpret_cast<char *>(&e)[i];
        }
        return buffer;
    }

    template<class T>
    inline T __inl_little_endian_deserialize(const char *&buffer, ssize_t &seek) {
        T result;
        for (int i = 0; i < sizeof(T); i++) {
            reinterpret_cast<char *>(&result)[i] = buffer[seek++];
        }
        return result;
    }

    template<class T>
    struct little_endian_serializer { };

    template<>
    struct little_endian_serializer<unsigned int> {
        static char *serialize(unsigned int e, size_t &size) {
            size = sizeof(unsigned int);
            return __inl_little_endian_serialize(e);
        }

        static unsigned int deserialize(const char *buffer, size_t len, ssize_t &seek) {
            if (seek + sizeof(unsigned int) > len) {
                return 0;
            }
            return __inl_little_endian_deserialize<unsigned int>(buffer, seek);
        }
    };

    template<>
    struct little_endian_serializer<unsigned short> {
        static char *serialize(unsigned short e, size_t &size) {
            size = sizeof(unsigned short);
            return __inl_little_endian_serialize(e);
        }

        static unsigned short deserialize(const char *buffer, size_t len, ssize_t &seek) {
            if (seek + sizeof(unsigned short) > len) {
                return 0;
            }
            return __inl_little_endian_deserialize<unsigned short>(buffer, seek);
        }
    };
}

#endif
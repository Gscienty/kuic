#ifndef _KUIC_LITTLE_ENDIAN_SERIALIZER_
#define _KUIC_LITTLE_ENDIAN_SERIALIZER_

#include <memory>
#include <utility>

namespace kuic {
    
    template<class T>
    inline void __inl_little_endian_serialize(T &e, char *&buffer) {
        for (int i = 0; i < sizeof(T); i++) {
            buffer[i] = reinterpret_cast<char *>(&e)[i];
        }
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
    struct little_endian_serializer {
        static char *serialize(T e, size_t &size);
        static T deserialize(const char *buffer, size_t len, ssize_t &seek);
    };

    template<>
    struct little_endian_serializer<unsigned int> {
        static char *serialize(unsigned int e, size_t &size) {
            char *buffer = new char[sizeof(e)];
            __inl_little_endian_serialize(e, buffer);
            return buffer;
        }

        static unsigned int deserialize(const char *buffer, size_t len, ssize_t &seek) {
            if (seek + sizeof(unsigned int) > len) {
                return 0;
            }
            return __inl_little_endian_deserialize<unsigned int>(buffer, seek);
        }
    };
}

#endif
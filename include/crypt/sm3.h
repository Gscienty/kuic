#ifndef _KUIC_CRYPT_SM3_
#define _KUIC_CRYPT_SM3_

#include "type.h"
#include <utility>
#include <memory>

namespace kuic {
    namespace crypt {
        const kuic::byte_t sm3_iv[] = {
            0x73, 0x80, 0x16, 0x6F,
            0x49, 0x14, 0xb2, 0xB9,
            0x17, 0x24, 0x42, 0xD7,
            0xDA, 0x8A, 0x06, 0x00,
            0xA9, 0x6F, 0x30, 0xBC,
            0x16, 0x31, 0x38, 0xAA,
            0xE3, 0x8D, 0xEE, 0x4D,
            0xB0, 0xFB, 0x0E, 0x4E
        };

        const kuic::word_t sm3_t1 = 0x1945CC79;
        const kuic::word_t sm3_t2 = 0x8A9D877A;

        class sm3 {
        private:
            static void cf_func(kuic::byte_t *iv, kuic::byte_t *b);
            static kuic::word_t get_tvec(int i);
            static kuic::word_t ff(int i, kuic::word_t x, kuic::word_t y, kuic::word_t z);
            static kuic::word_t gg(int i, kuic::word_t x, kuic::word_t y, kuic::word_t z);
            static kuic::word_t left_loop_move(int times, kuic::word_t n);
            static kuic::word_t add(const kuic::word_t &a, const kuic::word_t &b);
            static kuic::word_t p0(kuic::word_t x);
            static kuic::word_t p1(kuic::word_t x);
            static std::pair<kuic::byte_t *, size_t> ext(const kuic::byte_t *m, size_t size);
        public:
            static kuic::byte_t *hash(const kuic::byte_t *m, size_t size);
        };
    }
}

#endif
#include "crypt/sm3.h"
#include <memory>

kuic::word_t kuic::crypt::sm3::get_tvec(int i) {
    return (0 <= i && i <= 15) ? kuic::crypt::sm3_t1 : kuic::crypt::sm3_t2;
}
kuic::word_t kuic::crypt::sm3::ff(int i, kuic::word_t x, kuic::word_t y, kuic::word_t z) {
    return (0 <= i && i <= 15) ? (x ^ y ^ z) : ((x & y) | (y & z) | (x & z));
}
kuic::word_t kuic::crypt::sm3::gg(int i, kuic::word_t x, kuic::word_t y, kuic::word_t z) {
    return (0 <= i && i <= 15) ? (x ^ y ^ z) : ((x & y) | ((~x) & z));
}
kuic::word_t kuic::crypt::sm3::left_loop_move(int times, kuic::word_t n) {
    kuic::word_t result = 0x00000000;
    kuic::byte_t * result_ptr = reinterpret_cast<kuic::byte_t *>(&result);
    for (int i = 0; i < 32; i++) {
        int n_pos = (i - (times % 32)) < 0 ? (32 + (i - (times % 32))) : (i - (times % 32));
        result_ptr[3 - i / 8] |= (reinterpret_cast<kuic::byte_t *>(&n)[3 - (n_pos % 32) / 8] & (0x01 << (((n_pos % 32)) % 8))) == 0x00
            ? 0x00
            : (0x01 << i % 8);
    }
    return result;
}
kuic::word_t kuic::crypt::sm3::add(const kuic::word_t &a, const kuic::word_t &b) {
    kuic::word_t result = 0x00000000;
    bool cflag = false;
    for (int i = 3; i >= 0; i--) {
        kuic::byte_t byte_a = (a & ((0x000000FF) << (i << 3))) >> (i << 3);
        kuic::byte_t byte_b = (b & ((0x000000FF) << (i << 3))) >> (i << 3);
        kuic::byte_t byte_result = byte_a + byte_b + (cflag ? 1 : 0);
        cflag = byte_result < byte_a || byte_result < byte_b;
        result |= byte_result << (i << 3);
    }
    return result;
}
kuic::word_t kuic::crypt::sm3::p0(kuic::word_t x) {
    return x ^ kuic::crypt::sm3::left_loop_move(9, x) ^ kuic::crypt::sm3::left_loop_move(17, x);
}
kuic::word_t kuic::crypt::sm3::p1(kuic::word_t x) {
    return x ^ kuic::crypt::sm3::left_loop_move(15, x) ^ kuic::crypt::sm3::left_loop_move(23, x);
}
std::pair<kuic::byte_t *, size_t> kuic::crypt::sm3::ext(const kuic::byte_t *m, size_t size) {
    size_t zero_bytes_count = (448 - (size * 8 % 512)) / 8 - 1;

    kuic::byte_t *m_ = new kuic::byte_t[size + 1 + zero_bytes_count + 8];
    std::copy(m, m + size, m_);
    m_[size] = 0x80;
    std::fill_n(m_ + size + 1, zero_bytes_count, kuic::byte_t(0x00));
    unsigned long mask = 0x00000000000000FF;
    for (int i = 0; i < 8; i++) {
        m_[size + 1 + zero_bytes_count + 7 - i] = kuic::byte_t((mask & (size << 3)) >> (i << 3));
        mask <<= 8;
    }

    return std::pair<kuic::byte_t *, size_t>(m_, size + 1 + zero_bytes_count + 8);
}

kuic::byte_t *
kuic::crypt::sm3::hash(const kuic::byte_t *m, size_t size) {
    kuic::byte_t *m_;
    size_t m_len;
    std::tie<kuic::byte_t *, size_t>(m_, m_len) = kuic::crypt::sm3::ext(m, size);
    std::unique_ptr<kuic::byte_t []> m_ptr(m_);

    kuic::byte_t *result = new kuic::byte_t[32];
    std::copy(sm3_iv, sm3_iv + 32, result);

    int group_count = m_len * 8 / 512;
    for (int i = 0; i < group_count; i++) {
        sm3::cf_func(result, m_ptr.get() + i * 512 / 8);
    }

    return result;
}

void kuic::crypt::sm3::cf_func(kuic::byte_t *iv, kuic::byte_t *b) {
    std::unique_ptr<kuic::word_t []> w(new kuic::word_t[68]);
    std::unique_ptr<kuic::word_t []> w_(new kuic::word_t[64]);
    std::copy(b, b + 64, reinterpret_cast<kuic::byte_t *>(w.get()));

    for (int j = 16; j < 68; j++) {
        w[j] = kuic::crypt::sm3::p1(w[j - 16] ^ w[j - 9] ^ kuic::crypt::sm3::left_loop_move(15, w[j - 3]))
            ^ kuic::crypt::sm3::left_loop_move(7, w[j - 13])
            ^ w[j - 6];
    }

    for (int j = 0; j < 64; j++) {
        w_[j] = w[j] ^ w[j + 4];
    }

    std::unique_ptr<kuic::word_t []> reg(new kuic::word_t[8]);
    std::copy(iv, iv + 32, reinterpret_cast<kuic::byte_t *>(reg.get()));

    for (int j = 0; j < 64; j++) {
        kuic::word_t ss1 = kuic::crypt::sm3::left_loop_move(7,
            kuic::crypt::sm3::add(kuic::crypt::sm3::add(
                kuic::crypt::sm3::left_loop_move(12, reg[0]),
                reg[4]),
                kuic::crypt::sm3::left_loop_move(j, kuic::crypt::sm3::get_tvec(j))));

        kuic::word_t ss2 = ss1 ^ kuic::crypt::sm3::left_loop_move(12, reg[0]);

        kuic::word_t tt1 = kuic::crypt::sm3::add(kuic::crypt::sm3::add(kuic::crypt::sm3::add(
            kuic::crypt::sm3::ff(j, reg[0], reg[1], reg[2]),
            reg[3]),
            ss2),
            w_[j]);

        kuic::word_t tt2 = kuic::crypt::sm3::add(kuic::crypt::sm3::add(kuic::crypt::sm3::add(
            kuic::crypt::sm3::gg(j, reg[4], reg[5], reg[6]), reg[7]), ss1), w[j]);

        reg[3] = reg[2];
        reg[2] = kuic::crypt::sm3::left_loop_move(9, reg[1]);
        reg[1] = reg[0];
        reg[0] = tt1;
        reg[7] = reg[6];
        reg[6] = kuic::crypt::sm3::left_loop_move(19, reg[5]);
        reg[5] = reg[4];
        reg[4] = kuic::crypt::sm3::p0(tt2);
    }

    for (int i = 0; i < 8; i++) {
        reinterpret_cast<kuic::word_t *>(iv)[i] ^= reg[i];
    }
}
#include "crypt/sm4.h"
#include <utility>

kuic::word_t kuic::crypt::sm4::tau(kuic::word_t x) {
    return kuic::crypt::sm4::sbox((x & 0x000000FF))
        | (kuic::crypt::sm4::sbox((x & 0x0000FF00) >> 8) << 8)
        | (kuic::crypt::sm4::sbox((x & 0x00FF0000) >> 16) << 16)
        | (kuic::crypt::sm4::sbox((x & 0xFF000000) >> 24) << 24);
}

kuic::byte_t kuic::crypt::sm4::sbox(kuic::byte_t x) {
    return kuic::crypt::sm4_sbox[(x & 0xF0) >> 4][x & 0x0F];
};

kuic::word_t kuic::crypt::sm4::left_loop_move(int times, kuic::word_t n) {
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

kuic::word_t kuic::crypt::sm4::l(kuic::word_t x) {
    return x
        ^ kuic::crypt::sm4::left_loop_move(2, x)
        ^ kuic::crypt::sm4::left_loop_move(10, x)
        ^ kuic::crypt::sm4::left_loop_move(18, x)
        ^ kuic::crypt::sm4::left_loop_move(24, x);
}

kuic::word_t kuic::crypt::sm4::t(kuic::word_t x) {
    return kuic::crypt::sm4::l(kuic::crypt::sm4::tau(x));
}

kuic::word_t
kuic::crypt::sm4::f(
    kuic::word_t x_0,
    kuic::word_t x_1,
    kuic::word_t x_2,
    kuic::word_t x_3,
    kuic::word_t rk) {
    
    return x_0 ^ kuic::crypt::sm4::t(x_1 ^ x_2 ^ x_3 ^ rk);
}

kuic::word_t kuic::crypt::sm4::l_(kuic::word_t x) {
    return x
        ^ kuic::crypt::sm4::left_loop_move(13, x)
        ^ kuic::crypt::sm4::left_loop_move(23, x);
}

kuic::word_t kuic::crypt::sm4::t_(kuic::word_t x) {
    return kuic::crypt::sm4::l_(kuic::crypt::sm4::tau(x));
}

kuic::word_t kuic::crypt::sm4::k_gen(
    int i,
    kuic::word_t k_0,
    kuic::word_t k_1,
    kuic::word_t k_2,
    kuic::word_t k_3) {
    return k_0 ^ kuic::crypt::sm4::t_(k_1 ^ k_2 ^ k_3 ^ kuic::crypt::sm4_ck[i]);
}

kuic::byte_t *
kuic::crypt::sm4::cipher(kuic::byte_t *m, kuic::word_t *rk) {
    kuic::word_t m_group[5];
    std::copy(m, m + 16, reinterpret_cast<kuic::byte_t *>(m_group));

    for (int i = 0; i < 32; i++) {
        m_group[(i + 4) % 5] = kuic::crypt::sm4::f(
            m_group[i % 5], m_group[(i + 1) % 5], m_group[(i + 2) % 5], m_group[(i + 3) % 5], rk[i]);
    }

    kuic::word_t r_m_group[4];

    r_m_group[0] = m_group[0];
    r_m_group[1] = m_group[4];
    r_m_group[2] = m_group[3];
    r_m_group[3] = m_group[2];

    kuic::byte_t *result = new kuic::byte_t[16];
    std::copy(reinterpret_cast<kuic::byte_t *>(r_m_group), reinterpret_cast<kuic::byte_t *>(r_m_group) + 16, result);

    return result;
}

kuic::byte_t *
kuic::crypt::sm4::encrypt(kuic::byte_t *m, kuic::byte_t *key) {
    std::unique_ptr<kuic::word_t[]> k_group = kuic::crypt::sm4::extend_key(key);
    return kuic::crypt::sm4::cipher(m, k_group.get() + 4);
}

kuic::byte_t *
kuic::crypt::sm4::decrypt(kuic::byte_t *m, kuic::byte_t *key) {
    std::unique_ptr<kuic::word_t[]> k_group(kuic::crypt::sm4::extend_key(key));
    std::unique_ptr<kuic::word_t[]> r_k_group(new kuic::word_t[32]);
    for (int i = 0; i < 32; i++) {
        r_k_group[i] = k_group[32 + 3 - i];
    }
    return kuic::crypt::sm4::cipher(m, r_k_group.get());
}

kuic::byte_t *
kuic::crypt::sm4::encrypt_rk(kuic::byte_t *m, kuic::word_t *rk) {
    return kuic::crypt::sm4::cipher(m, rk);
}

kuic::byte_t *
kuic::crypt::sm4::decrypt_rk(kuic::byte_t *m, kuic::word_t *rk) {
    return kuic::crypt::sm4::cipher(m, rk);
}

std::unique_ptr<kuic::word_t[]>
kuic::crypt::sm4::extend_key(kuic::byte_t *key) {
    std::unique_ptr<kuic::word_t[]> k_group(new kuic::word_t[32 + 4]);
    for (int i = 0; i < 4; i++) {
        k_group[i] = reinterpret_cast<kuic::word_t *>(key)[i] ^ kuic::crypt::sm4_fk[i];
    }
    for (int i = 0; i < 32; i++) {
        k_group[i + 4] = kuic::crypt::sm4::k_gen(i, k_group[i], k_group[i + 1], k_group[i + 2], k_group[i + 3]);
    }
    return k_group;
}

size_t kuic::crypt::sm4::get_message_length() const {
    return 16; 
}

size_t kuic::crypt::sm4::get_key_length() const {
    return 16;
}

size_t kuic::crypt::sm4::get_cipher_length() const {
    return 16;
}

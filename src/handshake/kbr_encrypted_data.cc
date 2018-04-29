#include "handshake/kbr_encrypted_data.h"
#include "crypt/sm4.h"
#include "crypt/ecb_mode.h"
#include <utility>

kuic::handshake::kbr_encrypted_data::kbr_encrypted_data() { }

kuic::handshake::kbr_encrypted_data::kbr_encrypted_data(
        kuic::kbr_key_version_t key_version,
        kuic::kbr_encryption_type_t encryption_type)
    : version(key_version)
    , encryption_type(encryption_type) { }

void kuic::handshake::kbr_encrypted_data::set_plain_message(
        kuic::byte_t *plain_text,
        size_t plain_text_size,
        kuic::byte_t *secret_key,
        size_t secret_key_size) {
    std::unique_ptr<kuic::crypt::crypter> _crypter = 
        this->get_crypter();
    std::unique_ptr<kuic::crypt::mode> mode = 
        this->get_mode(_crypter.get());

    kuic::byte_t *cipher_buffer = nullptr;
    size_t cipher_size = 0;

    mode->set_message(plain_text, plain_text_size);
    mode->set_secret_key(secret_key, secret_key_size);

    std::tie(cipher_buffer, cipher_size) = mode->encrypt();

    this->cipher.assign(
            cipher_buffer,
            cipher_buffer + cipher_size);
}

std::unique_ptr<kuic::crypt::crypter>
kuic::handshake::kbr_encrypted_data::get_crypter() {
    switch (this->encryption_type & 0x0000FFFF) {
        case 0x00000001:    // sm4
            return std::unique_ptr<kuic::crypt::crypter>(
                    new kuic::crypt::sm4());

        default:
            return std::unique_ptr<kuic::crypt::crypter>(
                    new kuic::crypt::sm4());
    }
}

std::unique_ptr<kuic::crypt::mode>
kuic::handshake::kbr_encrypted_data::get_mode(
        kuic::crypt::crypter *_crypter) {
    switch (this->encryption_type & 0xFFFF0000) {
        case 0x00010000:
            return std::unique_ptr<kuic::crypt::mode>(
                   new kuic::crypt::ecb_mode(_crypter));
        default:
            return std::unique_ptr<kuic::crypt::mode>(
                    new kuic::crypt::ecb_mode(_crypter));
    }
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_encrypted_data::get_plain_message(
        kuic::byte_t *secret_key,
        size_t secret_key_size) {
    std::unique_ptr<kuic::crypt::crypter> _crypter = 
        this->get_crypter();
    std::unique_ptr<kuic::crypt::mode> mode = this->get_mode(
            _crypter.get());

    mode->set_message(
            this->cipher.data(),
            this->cipher.size());
    mode->set_secret_key(
            secret_key,
            secret_key_size);

    return mode->decrypt();
}

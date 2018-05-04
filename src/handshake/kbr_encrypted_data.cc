#include "handshake/kbr_encrypted_data.h"
#include "handshake/serializer.h"
#include "crypt/sm4.h"
#include "crypt/ecb_mode.h"
#include <utility>
#include <algorithm>
#include <memory>

kuic::handshake::kbr_encrypted_data::kbr_encrypted_data() { }

kuic::handshake::kbr_encrypted_data::kbr_encrypted_data(
        kuic::kbr_key_version_t key_version,
        kuic::kbr_encryption_type_t encryption_type)
    : version(key_version)
    , encryption_type(encryption_type) { }

void kuic::handshake::kbr_encrypted_data::set_plain_message(
        kuic::byte_t *plain_text, size_t plain_text_size, kuic::byte_t *secret_key, size_t secret_key_size) {
    // get current mode comb crypter
    std::unique_ptr<kuic::crypt::mode> mode = this->get_mode(this->get_crypter());
    
    // declare temporary buffer
    kuic::byte_t *cipher_buffer = nullptr;
    size_t cipher_size = 0;

    mode->set_message(plain_text, plain_text_size);
    mode->set_secret_key(secret_key, secret_key_size);

    std::tie(cipher_buffer, cipher_size) = mode->encrypt();

    this->cipher.assign(cipher_buffer, cipher_buffer + cipher_size);
}

kuic::crypt::crypter *
kuic::handshake::kbr_encrypted_data::get_crypter() {
    switch (this->encryption_type & 0x0000FFFF) {
        case 0x00000001:    // sm4
            return new kuic::crypt::sm4();

        default:
            return new kuic::crypt::sm4();
    }
}

std::unique_ptr<kuic::crypt::mode>
kuic::handshake::kbr_encrypted_data::get_mode(
        kuic::crypt::crypter *_crypter) {
    switch (this->encryption_type & 0xFFFF0000) {
        case 0x00010000:    // ecb
            return std::unique_ptr<kuic::crypt::mode>(new kuic::crypt::ecb_mode(_crypter));
        default:
            return std::unique_ptr<kuic::crypt::mode>(new kuic::crypt::ecb_mode(_crypter));
    }
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_encrypted_data::get_plain_message(
        kuic::byte_t *secret_key,
        size_t secret_key_size) {
    std::unique_ptr<kuic::crypt::mode> mode = this->get_mode(this->get_crypter());

    mode->set_message(this->cipher.data(), this->cipher.size());
    mode->set_secret_key(secret_key, secret_key_size);

    return mode->decrypt();
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_encrypted_data::serialize() const {
    size_t size = sizeof(kuic::kbr_key_version_t) + sizeof(kuic::kbr_encryption_type_t) + this->cipher.size() * sizeof(kuic::byte_t);

    kuic::byte_t *result = new kuic::byte_t[size];
    
    kuic::byte_t *serialized_buffer_ptr = nullptr;
    std::unique_ptr<kuic::byte_t []> serialized_buffer;
    size_t serialized_size = 0;

    // serialize version
    std::tie(serialized_buffer_ptr, serialized_size) = kuic::handshake::kbr_key_version_serializer::serialize(this->version);
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    std::copy(serialized_buffer.get(), serialized_buffer.get() + serialized_size, result);

    // serialize encryption_type
    std::tie(serialized_buffer_ptr, serialized_size) = kuic::handshake::kbr_key_version_serializer::serialize(this->encryption_type);
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    std::copy(serialized_buffer.get(), serialized_buffer.get() + serialized_size, result + sizeof(kuic::kbr_key_version_t));

    // copy cipher to result
    std::copy(this->cipher.begin(), this->cipher.end(), result + sizeof(kuic::kbr_key_version_t) + sizeof(kuic::kbr_encryption_type_t));

    return std::pair<kuic::byte_t *, size_t>(result, size);
}

kuic::handshake::kbr_encrypted_data
kuic::handshake::kbr_encrypted_data::deserialize(kuic::byte_t *buffer, size_t size, size_t &seek) {
    kuic::handshake::kbr_encrypted_data result;
    
    // version
    result.version = kuic::handshake::kbr_key_version_serializer::deserialize(buffer + seek, size, seek);

    // encryption type
    result.encryption_type = kuic::handshake::kbr_encryption_type_serializer::deserialize(buffer + seek, size, seek);

    // cipher
    result.cipher.assign(buffer + seek, buffer + size);
    seek = size;

    return result;
}


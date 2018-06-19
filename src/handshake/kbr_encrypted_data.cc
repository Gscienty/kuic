#include "handshake/kbr_encrypted_data.h"
#include "handshake/serializer.h"
#include "crypt/aead_sm4_gcm.h"
#include <utility>
#include <algorithm>
#include <memory>

kuic::handshake::kbr_encrypted_data::kbr_encrypted_data() { }

kuic::handshake::kbr_encrypted_data::kbr_encrypted_data(
        kuic::kbr_key_version_t key_version,
        kuic::crypt_mode_type_t crypt_mode_type)
    : version(key_version)
    , crypt_mode_type(crypt_mode_type) { }

void kuic::handshake::kbr_encrypted_data::set_plain_message(
        std::basic_string<kuic::byte_t> plain_text,
        std::basic_string<kuic::byte_t> a_data,
        kuic::crypt::aead &sealer) {

    // get sealer
    std::string cipher = sealer.seal(
            std::string(plain_text.begin(), plain_text.end()),
            kuic::packet_number_t(0),
            std::string(a_data.begin(), a_data.end()));
    this->cipher.assign(cipher.begin(), cipher.end());
}

std::basic_string<kuic::byte_t>
kuic::handshake::kbr_encrypted_data::get_plain_message(
        std::basic_string<kuic::byte_t> a_data,
        kuic::crypt::aead &sealer) {
    std::string plain_text = sealer.open(
            std::string(this->cipher.begin(), this->cipher.end()),
            kuic::packet_number_t(0),
            std::string(a_data.begin(), a_data.end()));

    return std::basic_string<kuic::byte_t>(plain_text.begin(), plain_text.end());
}

std::basic_string<kuic::byte_t>
kuic::handshake::kbr_encrypted_data::serialize() const {
    // declare result
    std::basic_string<kuic::byte_t> result;

    // serialize version
    result.append(kuic::handshake::kbr_key_version_serializer::serialize(this->version));
    // serialize encryption_type
    result.append(kuic::handshake::crypt_mode_type_serializer::serialize(this->crypt_mode_type));
    // copy cipher to result
    result.append(this->cipher);

    return result;
}

kuic::handshake::kbr_encrypted_data
kuic::handshake::kbr_encrypted_data::deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::kbr_encrypted_data result;
    
    // version
    result.version = kuic::handshake::kbr_key_version_serializer::deserialize(buffer, seek);
    // encryption type
    result.crypt_mode_type = kuic::handshake::crypt_mode_type_serializer::deserialize(buffer, seek);
    // cipher
    result.cipher.assign(buffer.begin() + seek, buffer.end());
    seek = buffer.size();

    return result;
}


#include "crypt/aead_sm4_gcm.h"
#include "eys.h"
#include <cryptopp/gcm.h>
#include <cryptopp/sm4.h>
#include <cryptopp/filters.h>

kuic::crypt::aead_sm4_gcm::aead_sm4_gcm(
        const std::string key,
        const std::string iv) {
    this->key.assign(key.begin(), key.end());
    this->iv.assign(iv.begin(), iv.end());
}

std::string kuic::crypt::aead_sm4_gcm::seal(
        const std::string plain, const kuic::packet_number_t packet_number, const std::string a_data) {
    std::string nonce = this->make_nonce(packet_number);
    
    CryptoPP::GCM<CryptoPP::SM4>::Encryption e;
    e.SetKeyWithIV(
        reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(this->key.data())),
        this->key.size(),
        reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(nonce.data())),
        nonce.size());

    std::string result;

    CryptoPP::AuthenticatedEncryptionFilter ef(e, new CryptoPP::StringSink(result), false, this->overhead());
    
    ef.ChannelPut(CryptoPP::AAD_CHANNEL, reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(a_data.data())), a_data.size());
    ef.ChannelMessageEnd(CryptoPP::AAD_CHANNEL);

    ef.ChannelPut(CryptoPP::DEFAULT_CHANNEL, reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(plain.data())), plain.size());
    ef.ChannelMessageEnd(CryptoPP::DEFAULT_CHANNEL);

    return result;
}

std::string kuic::crypt::aead_sm4_gcm::open(
        const std::string secret, const kuic::packet_number_t packet_number, const std::string a_data) {
    std::string nonce = this->make_nonce(packet_number);

    CryptoPP::GCM<CryptoPP::SM4>::Decryption d;
    d.SetKeyWithIV(
            reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(this->key.data())),
            this->key.size(),
            reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(nonce.data())),
            nonce.size());

    std::string result;

    try {
        CryptoPP::AuthenticatedDecryptionFilter df(d, NULL,
                CryptoPP::AuthenticatedDecryptionFilter::MAC_AT_BEGIN |
                CryptoPP::AuthenticatedDecryptionFilter::THROW_EXCEPTION,
                this->overhead());

        std::string enc = secret.substr(0, secret.size() - this->overhead());
        std::string mac = secret.substr(secret.size() - this->overhead());

        df.ChannelPut(CryptoPP::DEFAULT_CHANNEL, reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(mac.data())), mac.size());
        df.ChannelPut(CryptoPP::AAD_CHANNEL, reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(a_data.data())), a_data.size());
        df.ChannelPut(CryptoPP::DEFAULT_CHANNEL, reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(enc.data())), enc.size());

        df.ChannelMessageEnd(CryptoPP::AAD_CHANNEL);
        df.ChannelMessageEnd(CryptoPP::DEFAULT_CHANNEL);

        if (df.GetLastResult() == false) {
            return result;
        }

        df.SetRetrievalChannel(CryptoPP::DEFAULT_CHANNEL);
        size_t plain_len = df.MaxRetrievable();
        result.resize(plain_len);

        df.Get(reinterpret_cast<CryptoPP::byte *>(const_cast<char *>(result.data())), plain_len);

        return result;
    }
    catch (CryptoPP::HashVerificationFilter::HashVerificationFailed&) {
        return std::string();
    }
}

std::string kuic::crypt::aead_sm4_gcm::make_nonce(const kuic::packet_number_t packet_number) {
    std::pair<kuic::byte_t *, size_t> serialized_packet_number =
        eys::bigendian_serializer<kuic::byte_t, kuic::packet_number_t>::serialize(packet_number);
    size_t zero_count = this->iv.size() - 8;
    std::string result(zero_count, 0x00);
    result.insert(result.end(), serialized_packet_number.first, serialized_packet_number.first + serialized_packet_number.second);

    for (size_t i = 0; i < this->iv.size(); i++) {
        result[i] ^= this->iv[i];
    }
    
    return this->iv;
}

#include "crypt/mode.h"
#include "crypt/sm4.h"
#include <algorithm>

kuic::crypt::mode::mode(crypter *&&crypter_ptr)
    : crypter_ptr(std::unique_ptr<kuic::crypt::crypter>(crypter_ptr))
    , message_length(0)
    , key_length(0) { }


void kuic::crypt::mode::set_message(
        kuic::byte_t *message, size_t message_len) {
    this->message = message;
    this->message_length = message_len;
}

void kuic::crypt::mode::set_secret_key(
           kuic::byte_t *key, size_t key_len) {

    this->key = std::unique_ptr<kuic::byte_t>(
        new kuic::byte_t[this->crypter_ptr.get()->get_key_length()]);

    if (key_len < this->crypter_ptr->get_key_length()) {
        std::copy_n(key, key_len, this->key.get());
        std::fill_n(
                this->key.get() + key_len,
                this->crypter_ptr->get_key_length() - key_len,
                kuic::byte_t(0x00));
    }
    else { 
        std::copy_n(key, this->crypter_ptr->get_key_length(), this->key.get());
    }
}

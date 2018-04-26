#include "crypt/ecb_mode.h"
#include <algorithm>

std::pair<kuic::byte_t *, size_t>
kuic::crypt::ecb_mode::encrypt() {
    // calculate group count
    int group_count = this->message_length / this->crypter_ptr->get_message_length();
    
    // calculate remain bytes count
    size_t remain_bytes_count = this->message_length
        - group_count * this->crypter_ptr->get_message_length();
    
    // declare result cipher text buffer
    size_t cipher_length = (group_count + (remain_bytes_count == 0 ? 0 : 1)) * 
        this->crypter_ptr->get_cipher_length();
    kuic::byte_t *encrypted_buffer = new kuic::byte_t[cipher_length];
    
    for (int i = 0; i < group_count; i++) {
        // encrypt ith group message, the cipher stored in the "group_cipher"
        std::unique_ptr<kuic::byte_t> group_cipher(
                this->crypter_ptr->encrypt(
                    this->message + (i * this->crypter_ptr->get_message_length()), 
                    this->key.get()));
        // copy the cipher text from "group_cipher" to "entrypted_buffer"
        std::copy_n(
                group_cipher.get(),
                this->crypter_ptr->get_cipher_length(),
                encrypted_buffer + (i * this->crypter_ptr->get_cipher_length()));
    }

    // ECB mode spec
    if (remain_bytes_count != 0) {
        // declare a store unit, used to store plain text & extend remain message
        std::unique_ptr<kuic::byte_t> remain_plain_buffer(
                new kuic::byte_t[this->crypter_ptr->get_message_length()]);

        // copy remain plain text from "this->message" to "remain_plain_buffer"
        std::copy_n(
                this->message + group_count * this->crypter_ptr->get_message_length(),
                remain_bytes_count,
                remain_plain_buffer.get());
        // remain_plain_buffer fill '0'
        std::fill_n(
                remain_plain_buffer.get() + remain_bytes_count,
                this->crypter_ptr->get_message_length() - remain_bytes_count,
                kuic::byte_t(0x00));

        // declare a store unit, used to store cipher text
        std::unique_ptr<kuic::byte_t> remain_cipher_buffer(
                this->crypter_ptr->encrypt(remain_plain_buffer.get(), this->key.get()));

        // copy the cipher text from "remain_cipher_buffer" to "encrypted_buffer"
        std::copy_n(
                remain_cipher_buffer.get(),
                this->crypter_ptr->get_cipher_length(),
                encrypted_buffer + (group_count * this->crypter_ptr->get_cipher_length()));
    }

    return std::pair<kuic::byte_t *, size_t>(encrypted_buffer, cipher_length);
}

std::pair<kuic::byte_t *, size_t>
kuic::crypt::ecb_mode::decrypt() {
    // if message_length is not multiple of "ciper_length" then return imm
    if (this->message_length % this->crypter_ptr->get_cipher_length() != 0) {
        return std::pair<kuic::byte_t *, size_t>(nullptr, 0);
    }

    // calculate group count
    int group_count = this->message_length / this->crypter_ptr->get_cipher_length();

    // declare result
    kuic::byte_t *plain_buffer = new kuic::byte_t[
        group_count * this->crypter_ptr->get_message_length()];

    for (int i = 0; i < group_count; i++) {
        std::unique_ptr<kuic::byte_t> group_plain(
                this->crypter_ptr->decrypt(
                    this->message + (i * this->crypter_ptr->get_cipher_length()),
                    this->key.get()));
        std::copy_n(
                group_plain.get(),
                this->crypter_ptr->get_message_length(),
                plain_buffer + (i * this->crypter_ptr->get_message_length()));
    }

    return std::pair<kuic::byte_t *, size_t>(
            plain_buffer,
            group_count * this->crypter_ptr->get_message_length());
}

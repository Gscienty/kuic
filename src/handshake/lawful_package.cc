#include "handshake/lawful_package.h"

kuic::handshake::lawful_package::lawful_package()
    : package_error(kuic::no_error) { }

kuic::handshake::lawful_package::lawful_package(kuic::error_t err)
    : package_error(err) { }

bool kuic::handshake::lawful_package::is_lawful() const {
    return this->package_error == kuic::no_error;
}

#include "lawful_package.h"

kuic::lawful_package::lawful_package()
    : package_error(kuic::no_error) { }

kuic::lawful_package::lawful_package(kuic::error_t err)
    : package_error(err) { }

bool kuic::lawful_package::is_lawful() const {
    return this->package_error == kuic::no_error;
}

kuic::error_t
kuic::lawful_package::get_error() const {
    return this->package_error;
}

void kuic::lawful_package::set_error(kuic::error_t err) {
    this->package_error = err;
}

#include "handshake/kbr_principal_name.h"
#include "define.h"

kuic::handshake::kbr_principal_name::kbr_principal_name(std::string name)
    : name(name)
    , type(kuic::kbr_name_default_type) { }
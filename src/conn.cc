#include "conn.h"
#include "package_serializer.h"
#include <memory>

kuic::conn::conn(eys::address local_addr, std::shared_ptr<eys::connection> &conn, size_t buffer_size)
    : p_conn(local_addr, conn, buffer_size) { }

bool kuic::conn::write(std::basic_string<kuic::byte_t> &buffer) {
    this->p_conn.send()
        .put<std::basic_string<kuic::byte_t>, kuic::byte_t, kuic::package_serializer<std::basic_string<kuic::byte_t>>>(buffer);
    return true;
}

std::basic_string<kuic::byte_t> kuic::conn::read() {
    size_t size = this->p_conn.receive();
    return this->p_conn.get_range<kuic::byte_t>(size);
}

#ifndef _KUIC_CONN_
#define _KUIC_CONN_

#include "eys.h"
#include "rw_lock.h"
#include "type.h"
#include <string>

namespace kuic {
    class conn {
    private:
        kuic::rw_lock rw_mutex;
        eys::udp_visitor p_conn;
    public:
        conn(eys::address local_addr, std::shared_ptr<eys::connection> &conn, size_t buffer_size);
        bool write(std::basic_string<kuic::byte_t> &buffer);
        std::basic_string<kuic::byte_t> read();
    };
}

#endif


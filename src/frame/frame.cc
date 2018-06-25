#include "frame/frame.h"
#include "frame/stream_frame.h"
#include "frame/rst_stream_frame.h"
#include "frame/connection_close_frame.h"
#include "frame/max_data_frame.h"
#include "frame/max_stream_data_frame.h"
#include "frame/max_stream_id_frame.h"
#include "frame/ping_frame.h"
#include "frame/stream_blocked_frame.h"
#include "frame/stream_id_blocked_frame.h"
#include "frame/ack_frame.h"
#include "frame/stop_sending_frame.h"
#include "frame/blocked_frame.h"
#include "define.h"

std::shared_ptr<kuic::frame::frame>
kuic::frame::frame::parse_next_frame(std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    while (seek < buffer.size()) {
        kuic::byte_t type_byte = buffer[seek];

        if (type_byte == 0x00) {
            seek++;
            continue;
        }

        if ((type_byte & 0xF8) == 0x10) {
            return std::make_shared<kuic::frame::frame>(kuic::frame::stream_frame::deserialize(buffer, seek));
        }

        switch (type_byte) {
        case kuic::frame_type_rst_stream:
            return std::make_shared<kuic::frame::frame>(kuic::frame::rst_stream_frame::deserialize(buffer, seek));
        case kuic::frame_type_connection_close:
            return std::make_shared<kuic::frame::frame>(kuic::frame::connection_close_frame::deserialize(buffer, seek));
        case kuic::frame_type_max_data:
            return std::make_shared<kuic::frame::frame>(kuic::frame::max_data_frame::deserialize(buffer, seek));
        case kuic::frame_type_max_stream_data:
            return std::make_shared<kuic::frame::frame>(kuic::frame::max_stream_data_frame::deserialize(buffer, seek));
        case kuic::frame_type_stream_id:
            return std::make_shared<kuic::frame::frame>(kuic::frame::max_stream_id_frame::deserialize(buffer, seek));
        case kuic::frame_type_ping:
            return std::make_shared<kuic::frame::frame>(kuic::frame::ping_frame::deserialize(buffer, seek));
        case kuic::frame_type_blocked:
            return std::make_shared<kuic::frame::frame>(kuic::frame::blocked_frame::deserialize(buffer, seek));
        case kuic::frame_type_stream_blocked:
            return std::make_shared<kuic::frame::frame>(kuic::frame::stream_blocked_frame::deserialize(buffer, seek));
        case kuic::frame_type_stream_id_blocked:
            return std::make_shared<kuic::frame::frame>(kuic::frame::stream_id_blocked_frame::deserialize(buffer, seek));
        case kuic::frame_type_stop_sending:
            return std::make_shared<kuic::frame::frame>(kuic::frame::stop_sending_frame::deserialize(buffer, seek));
        case kuic::frame_type_ack:
            return std::make_shared<kuic::frame::frame>(kuic::frame::ack_frame::deserialize(buffer, seek));
        }
    }
    return std::shared_ptr<kuic::frame::frame>();
}

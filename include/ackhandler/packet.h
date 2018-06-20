#ifndef _KUIC_ACKHANDLER_PACKET_
#define _KUIC_ACKHANDLER_PACKET_

#include "frame/frame.h"
#include "clock.h"
#include "type.h"
#include <memory>
#include <vector>

namespace kuic {
    namespace ackhandler {
        struct packet {
            kuic::packet_number_t packet_number;
            kuic::packet_type_t packet_type;
            std::vector<std::shared_ptr<kuic::frame::frame>> frames;
            kuic::bytes_count_t length;
            kuic::special_clock send_time;
            
            kuic::packet_number_t largest_acked;

            bool can_be_retransmitted;
            bool included_in_bytes_in_flight;
            std::vector<kuic::packet_number_t> retransmitted_as;
            bool is_retransmission;
            kuic::packet_number_t retransmission_of;

            bool is_handshake;

            packet()
                : packet_number(0)
                , packet_type(0)
                , length(0)
                , send_time(kuic::special_clock({ 0, 0 }))
                , largest_acked(0)
                , can_be_retransmitted(false)
                , included_in_bytes_in_flight(false)
                , is_retransmission(false)
                , retransmission_of(0)
                , is_handshake(false) { }
        };

        std::vector<std::shared_ptr<kuic::frame::frame>> strip_non_retransmittable_frames(std::vector<std::shared_ptr<kuic::frame::frame>> &frames);
        bool is_frame_retransmittable(kuic::frame::frame *frame);

        template<typename InputIterator> bool has_retransmittable_frames(InputIterator begin, InputIterator end) {
            for (InputIterator iter = begin; iter != end; iter++) {
                if (is_frame_retransmittable(*iter)) {
                    return true;
                }
            }
            return false;
        }
    }
}

#endif


#ifndef _KUIC_FRAME_ACK_FRAME_
#define _KUIC_FRAME_ACK_FRAME_

#include "frame/frame.h"
#include "type.h"
#include <vector>
#include <utility>

namespace kuic {
    namespace frame {

        const int ack_delay_exponent = 3;
        
        class ack_frame
            : public frame {
        private:
            kuic::kuic_time_t delay_time;
            std::vector<std::pair<kuic::packet_number_t, kuic::packet_number_t>> ranges;

            ack_frame(kuic::error_t error) : frame(error) { }

            std::pair<unsigned long, unsigned long> encode_ack_range(int index) const;
            int encodable_ack_ranges_count() const;
            unsigned long encode_ack_delay(kuic::kuic_time_t delay) const;
        public:
            ack_frame() { }
            static ack_frame deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
            virtual std::basic_string<kuic::byte_t> serialize() const override;
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
            bool has_missing_ranges() const;
            kuic::packet_number_t largest_acked() const;
            kuic::packet_number_t lowest_acked() const;
            bool acks_packet(kuic::packet_number_t p) const;

            kuic::kuic_time_t& get_delay_time();

            std::vector<std::pair<kuic::packet_number_t, kuic::packet_number_t>> &get_ranges();
        };
    }
}

#endif


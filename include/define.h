#ifndef _KUIC_DEFINE_
#define _KUIC_DEFINE_

#include "type.h"
#include <unistd.h>

namespace kuic {
    const kuic_time_t clock_second      = 1000 * 1000 * 1000;
    const kuic_time_t clock_millisecond = 1000 * 1000;
    const kuic_time_t clock_microsecond = 1000;
    
    const bytes_count_t default_tcp_mss                     = 1460;
    const float         window_update_threshold             = 0.25;
    const float         connection_flow_control_multiplier  = 1.5;
    const unsigned int  max_parameters_count                = 128;
    const unsigned int  parameter_max_length                = 4000;

    const band_width_t bits_per_second  = 1;
    const band_width_t bytes_per_second = 8 * bits_per_second;

    inline band_width_t __inl_bandwidth_from_delta(bytes_count_t bytes, kuic_time_t delta) {
        return band_width_t(bytes) * band_width_t(clock_second) * band_width_t(delta) * bytes_per_second;
    }

    const kuic::bytes_count_t max_byte_count = (1UL << 62) - 1;
    const int default_max_congestion_window_packets = 1000;
    const bytes_count_t initial_congestion_window = 32 * default_tcp_mss;
    const bytes_count_t default_max_congestion_window = default_max_congestion_window_packets * default_tcp_mss;
    const int max_tracked_skipped_packets = 10;
    const int max_outstanding_sent_packets = 2 * default_max_congestion_window_packets;
    const int max_tracked_sent_packets = max_outstanding_sent_packets * 5 / 4;
    const kuic::kuic_time_t min_pacing_delay = 100 * clock_microsecond;
    const int max_tracked_received_ack_ranges = default_max_congestion_window_packets;
    const int max_stream_frame_sorter_gaps = 1000;

    inline size_t __inl_packet_number_length_for_header(kuic::packet_number_t packet_number, kuic::packet_number_t least_unacked) {
        size_t diff = packet_number - least_unacked;
        if (diff < (1 << (2 * 8 - 1))) {
            return 2;
        }
        return 4;
    }

    inline size_t __inl_packet_number_length(kuic::packet_number_t packet_number) {
        if (packet_number < (1UL << (1 * 8))) {
            return 1;
        }
        if (packet_number < (1UL << (2 * 8))) {
            return 2;
        }
        if (packet_number < (1UL << (4 * 8))) {
            return 4;
        }
        return 6;
    }
    
    const frame_type_t frame_type_padding           = 0x00;
    const frame_type_t frame_type_rst_stream        = 0x01;
    const frame_type_t frame_type_connection_close  = 0x02;
    const frame_type_t frame_type_application_close = 0x03;
    const frame_type_t frame_type_max_data          = 0x04;
    const frame_type_t frame_type_max_stream_data   = 0x05;
    const frame_type_t frame_type_stream_id         = 0x06;
    const frame_type_t frame_type_ping              = 0x07;
    const frame_type_t frame_type_blocked           = 0x08;
    const frame_type_t frame_type_stream_blocked    = 0x09;
    const frame_type_t frame_type_stream_id_blocked = 0x0A;
    const frame_type_t frame_type_new_connection_id = 0x0B;
    const frame_type_t frame_type_stop_sending      = 0x0C;
    const frame_type_t frame_type_ack               = 0x0D;
    const frame_type_t frame_type_path_challenge    = 0x0E;
    const frame_type_t frame_type_path_response     = 0x0F;
    const frame_type_t frame_type_stream            = 0x10;
    


    const send_mode_t send_mode_none            = 0x00;
    const send_mode_t send_mode_ack             = 0x01;
    const send_mode_t send_mode_retransmission  = 0x02;
    const send_mode_t send_mode_rto             = 0x03;
    const send_mode_t send_mode_tlp             = 0x04;
    const send_mode_t send_mode_any             = 0x05;

    const packet_type_t packet_type_initial     = 0x7F;
    const packet_type_t packet_type_retry       = 0x7E;
    const packet_type_t packet_type_handshake   = 0x7D;

    const kbr_flag_t kbr_flag_forwardable               = 0x00000001 << (1 - 1);
    const kbr_flag_t kbr_flag_forwarded                 = 0x00000001 << (2 - 1);
    const kbr_flag_t kbr_flag_proxiable                 = 0x00000001 << (3 - 1);
    const kbr_flag_t kbr_flag_proxy                     = 0x00000001 << (4 - 1);
    const kbr_flag_t kbr_flag_allow_postdate            = 0x00000001 << (5 - 1);
    const kbr_flag_t kbr_flag_renewable                 = 0x00000001 << (8 - 1);
    const kbr_flag_t kbr_flag_opt_hardware_auth         = 0x00000001 << (11 - 1);
    const kbr_flag_t kbr_flag_disable_transited_check   = 0x00000001 << (26 - 1);
    const kbr_flag_t kbr_flag_renewable_ok              = 0x00000001 << (27 - 1);
    const kbr_flag_t kbr_flag_etc_tkt_in_skey           = 0x00000001 << (28 - 1);
    const kbr_flag_t kbr_flag_renew                     = 0x00000001 << (30 - 1);
    const kbr_flag_t kbr_flag_validate                  = 0x00000001 << (31 - 1);

    const kbr_name_type_t kbr_name_default_type = 0x00000000;

    const kbr_protocol_version_t kbr_current_protocol_version = 0x00000001;

    const kbr_encryption_type_t kbr_encryption_type_sm4_ecb = 0x00010001;

    const kuic::bytes_count_t max_ack_frame_size = 1000;
}

#endif

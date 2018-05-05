#ifndef _KUIC_TYPE_
#define _KUIC_TYPE_

namespace kuic {
    typedef int             error_t;
    typedef long            kuic_time_t;
    typedef unsigned long   packet_number_t;
    typedef unsigned long   bytes_count_t;
    typedef unsigned long   band_width_t;
    typedef unsigned long   stream_id_t;
    typedef unsigned int    tag_t;
    typedef unsigned char   byte_t;
    typedef unsigned int    word_t;

    typedef unsigned int    kbr_protocol_version_t;
    typedef unsigned int    kbr_message_type_t;
    typedef unsigned int    kbr_flag_t;
    typedef unsigned int    kbr_encryption_type_t;
    typedef unsigned int    kbr_encryption_key_t;
    typedef unsigned int    kbr_key_version_t;
    typedef unsigned int    kbr_padata_type_t;
    typedef unsigned int    kbr_name_type_t;
    typedef unsigned int    kbr_ticket_version_t;
    typedef unsigned int    kbr_address_type_t;
    typedef unsigned int    kbr_authorization_data_item_type_t;
}

#endif

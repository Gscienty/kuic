#ifndef _KUIC_ACKHANDLER_SEND_PACKET_HISTORY_
#define _KUIC_ACKHANDLER_SEND_PACKET_HISTORY_

#include "ackhandler/packet.h"
#include <list>
#include <map>
#include <algorithm>

namespace kuic {
    namespace ackhandler {
        class send_packet_history {
        private:
            std::list<packet> packet_list;
            std::map<kuic::packet_number_t, std::list<packet>::iterator> packet_map;
            kuic::packet_number_t first_outstanding;

            std::list<packet>::iterator send_packet_implement(const packet &p);
        public:
            send_packet_history();
            void send_packet(const packet &p);
            const packet *get_packet(kuic::packet_number_t packet_number);

            template <typename InputIterator>
            void send_packets_as_retrainsmission(InputIterator begin, InputIterator end, size_t size, kuic::packet_number_t retransmission_of) {
                if (this->packet_map.find(retransmission_of) == this->packet_map.end()) {
                    std::for_each(begin, end,
                            [this] (const packet &p) -> void {
                                this->send_packet_implement(p);
                            });
                    return;
                }

                auto retransmission = this->packet_map[retransmission_of];
                int i = 0;
                retransmission->retransmitted_as.resize(size);
                std::for_each(begin, end,
                        [&] (const packet &p) -> void {
                            retransmission->retransmitted_as[i] = p.packet_number;
                            std::list<packet>::iterator element = this->send_packet_implement(p);
                            element->is_retransmission = true;
                            element->retransmission_of = retransmission_of;

                            i++;
                        });
            }

            template <typename IteratorFunctional>
            void iterate(IteratorFunctional callback) {
                for (auto iterator = this->packet_list.begin(); iterator != this->packet_list.end(); iterator++) {
                    if (callback(*iterator) == false) {
                        break;
                    }
                }
            }

            const packet *get_first_outstanding() const;
            void mark_cannot_be_retransmitted(kuic::packet_number_t packet_number);

            void readjust_first_outstanding();
            size_t size() const;
            void remove(kuic::packet_number_t packet_number);
        };
    }
}

#endif


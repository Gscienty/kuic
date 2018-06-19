#ifndef _KUIC_HANDSHAKE_HANDSHAKE_MESSAGE_
#define _KUIC_HANDSHAKE_HANDSHAKE_MESSAGE_

#include "type.h"
#include "lawful_package.h"
#include "package_serializer.h"
#include "eys.h"
#include <utility>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>

namespace kuic {
    namespace handshake {
        class handshake_message
            : public kuic::package_serializable
            , public kuic::lawful_package {
        private:
            kuic::tag_t tag;
            std::map<kuic::tag_t, std::basic_string<kuic::byte_t>> data;

            handshake_message(kuic::tag_t tag, std::map<kuic::tag_t, std::basic_string<kuic::byte_t>> &data);
        public:
            handshake_message();
            handshake_message(kuic::error_t err);
            handshake_message(kuic::tag_t tag);

            void insert(kuic::tag_t tag, std::basic_string<kuic::byte_t> data);

            template <typename FieldType, typename Serialize = kuic::package_serializer<FieldType>>
            void insert(kuic::tag_t tag, const FieldType &field) {
                this->insert(tag, Serialize::serialize(field));
            }

            template <typename ElementType, typename Serialize = kuic::package_serializer<ElementType>>
            void insert_elements(kuic::tag_t tag, std::vector<ElementType> field) {
                // declare temporay buffer will insert into handshake_message
                std::basic_string<kuic::byte_t> result;

                // get field (vector) size
                unsigned int field_size = field.size();
                // serialize field_size
                result.append(eys::littleendian_serializer<kuic::byte_t, unsigned int>::serialize(field_size));

                std::basic_string<byte_t> index;
                std::basic_string<byte_t> data;
                int offset = 0;

                std::for_each(field.begin(), field.end(),
                        [&] (const ElementType &element) -> void {
                            // serialize current element
                            std::basic_string<kuic::byte_t> serialized_element = Serialize::serialize(element);
                            data.append(serialized_element);

                            offset += serialized_element.size();
                            // serialize offset
                            index.append(eys::littleendian_serializer<kuic::byte_t, int>::serialize(offset));
                        });
                // copy index & data to temporary buffer
                result.append(index);
                result.append(data);

                this->insert(tag, result);
            }
            
            template <typename ResultType, typename Deserialize = kuic::package_serializer<ResultType>>
            void assign(ResultType &result, kuic::tag_t tag) {
                if (this->exist(tag) == false) {
                    return ;
                }
                result = this->get<ResultType, Deserialize>(tag);
            }
            
            template <typename ElementType, typename Deserialize = kuic::package_serializer<ElementType>>
            void assign_elements(std::vector<ElementType> &result, kuic::tag_t tag) {
                if (this->exist(tag) == false) {
                    return ;
                }

                result = this->get_elements<ElementType, Deserialize>(tag);
            }

            template <typename ResultType, typename Deserialize>
            ResultType get(kuic::tag_t tag) {
                std::basic_string<kuic::byte_t> &temporary_buffer = this->get_serialized_buffer(tag);
                size_t seek = 0;
                return Deserialize::deserialize(temporary_buffer, seek);
            }

            template <typename ElementType, typename Deserialize>
            std::vector<ElementType> get_elements(kuic::tag_t tag) {
                // declare result
                std::vector<ElementType> result;
                // get serialized buffer
                std::basic_string<kuic::byte_t> &temporary_buffer = this->get_serialized_buffer(tag);
                // declare seek
                size_t seek = 0;
                // get elements count
                unsigned int elements_count =
                    eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(temporary_buffer, seek);
                std::vector<int> offsets;
                for (unsigned int i = 0; i < elements_count; i++) {
                    // get ith segment end offset
                    offsets.push_back(
                            eys::littleendian_serializer<kuic::byte_t, int>::deserialize(temporary_buffer, seek));
                }
                // get data area start position
                size_t data_start_position = seek;
                for (unsigned int i = 0; i < elements_count; i++) {
                    // deserialize current segment
                    result.push_back(Deserialize::deserialize(temporary_buffer, seek));
                }
                
                return result;
            }

            void set_tag(kuic::tag_t tag);

            static handshake_message parse_handshake_message(std::basic_string<kuic::byte_t> &reader);
            static handshake_message deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);

            virtual std::basic_string<kuic::byte_t> serialize() const override;
            
            std::vector<kuic::tag_t> get_tags_sorted() const;
            std::basic_string<kuic::byte_t> &get_serialized_buffer(kuic::tag_t tag);
            bool exist(kuic::tag_t tag) const;
            kuic::tag_t get_tag() const;
        };
    }
}

#endif

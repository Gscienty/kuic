#ifndef _KUIC_FRAME_FRAME_
#define _KUIC_FRAME_FRAME_

#include "type.h"
#include "package_serializer.h"
#include "lawful_package.h"

namespace kuic {
    namespace frame {
        class frame
            : public kuic::package_serializable
            , public kuic::lawful_package {
        protected:
            kuic::version_t version;

            frame(kuic::error_t error) : kuic::lawful_package(error)  { }
            frame() { }

        public:
            virtual size_t length() const = 0;
            virtual kuic::frame_type_t type() const = 0;

            virtual ~frame() { }
        };

    }
}

#endif


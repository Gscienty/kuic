#ifndef _KUIC_NULLABLE_
#define _KUIC_NULLABLE_

#include <cstddef>
#include <iostream>

namespace kuic {
    template <typename ClassType> class nullable {
    private:
        bool is_reference;
        ClassType *ptr;
    public:
        nullable(std::nullptr_t) noexcept : is_reference(false), ptr(nullptr) { }
        template<typename ClassType_1> nullable(ClassType_1 *p = nullptr) noexcept : is_reference(false), ptr(p) { }
        template<typename ClassType_1> nullable(ClassType_1 &instance) noexcept : is_reference(true), ptr(&instance) { }
        template<typename ClassType_1> nullable(nullable<ClassType_1> &nullable_instance) 
            : is_reference(nullable_instance.is_reference), ptr(nullable_instance.ptr) {
            nullable_instance.is_reference = true;
        };

        template<typename ClassType_1> nullable(nullable<ClassType_1> &&nullable_instance)
            : is_reference(nullable_instance.is_reference), ptr(nullable_instance.ptr)  { }

        ~nullable() {
            if (this->is_reference == false && this->ptr != nullptr) {
                delete ptr;
            }
        }

        template<typename ClassType_1> nullable<ClassType> &operator= (nullable<ClassType_1> &a) noexcept {
            if (&a != this) {
                if (this->is_reference == false) {
                    delete this->ptr;
                }
                this->ptr = a.release();
                this->is_reference = a.is_reference;
            }
            return *this;
        }

        nullable &operator= (nullable<ClassType> &a) noexcept {
            if (&a != this) {
                if (this->is_reference == false) {
                    delete this->ptr;
                }
                this->ptr = a.release();
                this->is_reference = a.is_reference;
            }
            return *this;
        }

        nullable &operator= (ClassType *a) noexcept {
            if (a != this->ptr) {
                if (this->is_reference == false) {
                    delete this->ptr;
                }
                this->ptr = a;
                this->is_reference = a->is_reference;
            }
            return *this;
        }

        bool is_null() const noexcept {
            return this->ptr == nullptr;
        }

        ClassType &operator* () const noexcept {
            return *this->ptr;
        }

        ClassType *operator-> () const noexcept {
            return this->ptr;
        }

        ClassType *get() const noexcept {
            return this->ptr;
        }

        ClassType *release() const noexcept {
            ClassType *tmp = this->ptr;
            this->ptr = nullptr;
            return tmp;
        }

        void reset(ClassType *p = nullptr) noexcept {
            if (p != this->ptr) {
                if (this->is_reference == false) {
                    delete this->ptr;
                }
                this->ptr = p;
                this->is_reference = false;
            }
        }
    };
}

#endif


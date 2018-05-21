#ifndef _KUIC_NULLABLE_
#define _KUIC_NULLABLE_

#include <cstddef>

namespace kuic {
    template <typename ClassType> class nullable {
    private:
        ClassType *ptr;
    public:
        nullable(std::nullptr_t) noexcept : ptr(nullptr) { }
        template<typename ClassType_1> nullable(ClassType_1 *p = nullptr) noexcept : ptr(p) { }
        template<typename ClassType_1> nullable(ClassType_1 &instance) noexcept : ptr(&instance) { }
        template<typename ClassType_1> nullable(nullable<ClassType_1> &nullable_instance) : ptr(nullable_instance.ptr) { };
        
        ~nullable() { delete ptr; }

        template<typename ClassType_1> nullable<ClassType> &operator= (nullable<ClassType_1> &a) noexcept {
            if (&a != this) {
                delete this->ptr;
                this->ptr = a.release();
            }
            return *this;
        }

        nullable &operator= (nullable<ClassType> &a) noexcept {
            if (&a != this) {
                delete this->ptr;
                this->ptr = a.release();
            }
            return *this;
        }

        nullable &operator= (ClassType *a) noexcept {
            if (a != this->ptr) {
                delete this->ptr;
                this->ptr = a;
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
                delete this->ptr;
                this->ptr = p;
            }
        }
    };
}

#endif


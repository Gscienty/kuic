#ifndef _KUIC_NULLABLE_
#define _KUIC_NULLABLE_

namespace kuic {
    template <typename ClassType> class nullable {
    private:
        ClassType *ptr;
    public:
        explicit nullable(ClassType *p = nullptr) noexcept : ptr(p) { }
        nullable(nullable<ClassType> &nullable_instance) = delete;

        ~nullable() {
            delete this->ptr;
        }
        
        nullable &operator= (nullable<ClassType> &a) noexcept {
            if (&a != this) {
                delete this->ptr;
                this->ptr = a.release();
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


#ifndef _KUIC_RW_LOCK_
#define _KUIC_RW_LOCK_

#include <mutex>
#include <condition_variable>

namespace kuic {
    class rw_lock {
    private:
        const bool write_first;
        size_t waiting_write_count;
        size_t reading_count;
        bool writing;

        std::mutex inner_mutex;
        std::condition_variable cond_writer;
        std::condition_variable cond_reader;

    public:
        rw_lock(bool write_first = false);
        rw_lock(const rw_lock &) = delete;
        
        void read_lock();
        void write_lock();
        void read_release();
        void write_release();
    };

    class reader_lock_guard {
    private:
        rw_lock &locker;
    public:
        reader_lock_guard(rw_lock &);
        ~reader_lock_guard();
    };

    class writer_lock_guard {
    private:
        rw_lock &locker;
    public:
        writer_lock_guard(rw_lock &);
        ~writer_lock_guard();
    };
}

#endif
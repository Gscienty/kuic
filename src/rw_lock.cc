#include "rw_lock.h"

kuic::rw_lock::rw_lock(bool write_first)
    : write_first(write_first)
    , waiting_write_count(0)
    , reading_count(0)
    , writing(false) { }

void kuic::rw_lock::read_lock() {
    std::unique_lock<std::mutex> locker(this->inner_mutex);
    if (this->write_first) {
        this->cond_reader.wait(locker, [=] () -> bool {
            return this->waiting_write_count == 0 &&
                this->writing == false;
        });
    }
    else {
        this->cond_reader.wait(locker, [=] () -> bool {
            return this->writing == false;
        });
    }
    this->reading_count++;
}

void kuic::rw_lock::write_lock() {
    std::unique_lock<std::mutex> locker(this->inner_mutex);
    this->waiting_write_count++;
    this->cond_writer.wait(locker, [=] () -> bool {
        return this->reading_count == 0 &&
            this->writing == false;
    });
    this->waiting_write_count--;
    this->writing = true;
}

void kuic::rw_lock::read_release() {
    std::unique_lock<std::mutex> locker(this->inner_mutex);
    this->reading_count--;
    if (this->reading_count == 0 && this->waiting_write_count > 0) {
        this->cond_writer.notify_one();
    }
}

void kuic::rw_lock::write_release() {
    std::unique_lock<std::mutex> locker(this->inner_mutex);
    this->writing = false;
    if (this->write_first) {
        if (this->waiting_write_count > 0) {
            this->cond_writer.notify_one();
        }
        else {
            this->cond_reader.notify_all();
        }
    }
    else {
        this->cond_reader.notify_all();
        if (this->waiting_write_count > 0) {
            this->cond_writer.notify_one();
        }
    }
}

kuic::reader_lock_guard::reader_lock_guard(kuic::rw_lock &locker)
    : locker(locker) {
    
    locker.read_lock();
}

kuic::reader_lock_guard::~reader_lock_guard() {
    this->locker.read_release();
}

kuic::writer_lock_guard::writer_lock_guard(kuic::rw_lock &locker)
    : locker(locker) {
    
    locker.write_lock();
}

kuic::writer_lock_guard::~writer_lock_guard() {
    this->locker.write_release();
}
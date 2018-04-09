#include "readwrite_lock.h"

namespace kuic {
    KuicRWLock::KuicRWLock(bool writeFirst = false)
        : writeFirst { writeFirst }
        , waitingWriteCount { 0 }
        , readingCount { 0 }
        , writing { false } { }
    
    void KuicRWLock::readLock() {
        std::unique_lock<std::mutex> locker(this->innerMutex);
        if (this->writeFirst) {
            this->condReader.wait(locker, [=] () -> bool {
                return this->waitingWriteCount == 0 && this->writing == false;
            });
        }
        else {
            this->condReader.wait(locker, [=] () -> bool {
                return this->writing == false;
            });
        }
        this->readingCount++;
    }

    void KuicRWLock::writeLock() {
        std::unique_lock<std::mutex> locker(this->innerMutex);
        this->waitingWriteCount++;
        this->condWriter.wait(locker, [=] () -> bool {
            return this->readingCount == 0 && this->writing == false;
        });
        this->waitingWriteCount--;
        this->writing = true;
    }

    void KuicRWLock::readRelease() {
        std::unique_lock<std::mutex> locker(this->innerMutex);
        this->readingCount--;
        if (this->readingCount == 0 && this->waitingWriteCount > 0) {
            this->condWriter.notify_one();
        }
    }

    void KuicRWLock::writeRelease() {
        std::unique_lock<std::mutex> locker(this->innerMutex);
        this->writing = false;
        if (this->writeFirst) {
            if (this->waitingWriteCount > 0) {
                this->condWriter.notify_one();
            }
            else {
                this->condReader.notify_all();
            }
        }
        else {
            this->condReader.notify_all();
            if (this->waitingWriteCount > 0) {
                this->condWriter.notify_one();
            }
        }
    }

    KuicRWLockReaderLockGuard::KuicRWLockReaderLockGuard(KuicRWLock &locker) 
        : locker(locker) { this->locker.readLock(); }
    KuicRWLockReaderLockGuard::~KuicRWLockReaderLockGuard() {
        this->locker.readRelease();
    }

    KuicRWLockWriterLockGuard::KuicRWLockWriterLockGuard(KuicRWLock &locker) 
        : locker(locker) { this->locker.writeLock(); }
    KuicRWLockWriterLockGuard::~KuicRWLockWriterLockGuard() {
        this->locker.writeRelease();
    }
}
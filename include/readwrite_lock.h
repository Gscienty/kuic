#ifndef _KUIC_READ_WRITE_LOCK_
#define _KUIC_READ_WRITE_LOCK_

#include <mutex>
#include <condition_variable>

namespace kuic {
    class KuicRWLock {
    private:
        const bool writeFirst;

        int waitingWriteCount;
        int readingCount;
        bool writing;

        std::mutex innerMutex;
        std::condition_variable condWriter;
        std::condition_variable condReader;

    public:
        KuicRWLock(bool writeFirst);
        KuicRWLock(const KuicRWLock &) = delete;
        KuicRWLock& operator= (const KuicRWLock &) = delete;

        void readLock();
        void writeLock();
        void readRelease();
        void writeRelease();
    };

    class KuicRWLockReaderLockGuard {
    private:
        KuicRWLock &locker;
    public:
        KuicRWLockReaderLockGuard(KuicRWLock &locker);
        ~KuicRWLockReaderLockGuard();
    };

    class KuicRWLockWriterLockGuard {
    private:
        KuicRWLock &locker;
    public:
        KuicRWLockWriterLockGuard(KuicRWLock &locker);
        ~KuicRWLockWriterLockGuard();
    };
}

#endif
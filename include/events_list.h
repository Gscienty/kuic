#ifndef _KUIC_EVENTS_LIST_
#define _KUIC_EVENTS_LIST_

#include <map>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace kuic {
    class events_list {
    private:
        std::map<int, std::function<void ()>> events_dict;
        std::mutex notify_mutex;
        std::condition_variable notify_cond;
        std::queue<int> notify_queue;
    public:
        events_list();
        bool push(int event_id, std::function<void ()> &events_func);
        void notify(int event_id);
        void wait(bool blocked = true);
    };
}

#endif


#include "events_list.h"
#include <utility>

kuic::events_list::events_list() { }

bool kuic::events_list::push(int event_id, std::function<void ()> &&event_func) {
    if (this->events_dict.find(event_id) != this->events_dict.end()) {
        return false;
    }

    this->events_dict.insert(std::pair<int, std::function<void ()>>(event_id, event_func));
    return true;
}

void kuic::events_list::notify(int event_id) {
    if (this->events_dict.find(event_id) != this->events_dict.end()) {
        return;
    }
    std::lock_guard<std::mutex> lock(this->notify_mutex);
    this->notify_queue.push(event_id);
    this->notify_cond.notify_one();
}

void kuic::events_list::wait(bool blocked) {
    if (this->events_dict.empty()) {
        return;
    }

    std::unique_lock<std::mutex> lock(this->notify_mutex);
    this->notify_cond.wait(lock, [&, this] () -> bool {
                return blocked == false || this->notify_queue.empty() == false;
            });

    if (this->notify_queue.empty() && blocked == false) {
        return;
    }
    int event_id = this->notify_queue.front();
    this->notify_queue.pop();

    auto event = this->events_dict.find(event_id);
    if (event == this->events_dict.end()) {
        return;
    }

    event->second();
}

void kuic::events_list::clear() {
    std::lock_guard<std::mutex> lock(this->notify_mutex);
    this->events_dict.clear();
    this->notify_cond.notify_one();
}

void kuic::events_list::clear_event() {
    std::lock_guard<std::mutex> lock(this->notify_mutex);
    while (this->notify_queue.empty()) { this->notify_queue.pop(); }
}

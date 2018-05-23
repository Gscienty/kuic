#include "ackhandler/packet.h"
#include <algorithm>

std::vector<std::shared_ptr<kuic::frame::frame>>
kuic::ackhandler::strip_non_retransmittable_frames(std::vector<std::shared_ptr<kuic::frame::frame>> &frames) {
    std::vector<std::shared_ptr<kuic::frame::frame>> result;

    std::for_each(frames.begin(), frames.end(),
            [&] (std::shared_ptr<kuic::frame::frame> &frame) -> void {
                if (kuic::ackhandler::is_frame_retransmittable(frame.get())) {
                    result.push_back(frame);
                }
            });
    return result;
}

bool kuic::ackhandler::is_frame_retransmittable(kuic::frame::frame *frame) {
    switch (frame->type()) {
    case kuic::frame_type_ack:
        return false;
    default:
        return true;
    }
}

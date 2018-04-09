#ifndef _KUIC_CONNECTION_FLOW_CONTROLLER_
#define _KUIC_CONNECTION_FLOW_CONTROLLER_

#include "base_flow_controller.h"
#include "congestion/round_trip_statistics.h"
#include "error.h"

namespace kuic {

    class ConnectionFlowController : public BaseFlowController {
    private:
        unsigned long lastBlockedAt;
    public:
        ConnectionFlowController(RoundTripStatistics &rtt,
                        unsigned long receiveWindowSize,
                        unsigned long maxReceiveWindowSize);
        
        bool isNewlyBlocked() const;
        unsigned long getNewlyBlockedAt();
        ErrorCode incrementHighestReceived(unsigned long increment);
        unsigned long receiveWindowUpdate();
        void ensureMinimumWindowSize(unsigned long inc);
    };
}

#endif
#ifndef _KUIC_STREAM_FLOW_CONTROLLER_
#define _KUIC_STREAM_FLOW_CONTROLLER_

#include "flowcontrol/base_flow_controller.h"
#include "flowcontrol/connection_flow_controller.h"
#include "congestion/round_trip_statistics.h"
#include "error.h"

namespace kuic {
    class StreamFlowController : public BaseFlowController {
    private:
        unsigned long streamID;
        ConnectionFlowController &connection;
        bool contributesToConnection;
        bool receivedFinalOffset;
    public:
        StreamFlowController(unsigned long streamID,
                            bool contributesToConnection,
                            ConnectionFlowController &connection,
                            unsigned long receiveWindow,
                            unsigned long maxReceiveWindow,
                            unsigned long initialSendWindow,
                            RoundTripStatistics &rtt);
        
        unsigned long getStreamID() const;
        bool getContributesToConnection() const;

        ErrorCode updateHighestReceived(unsigned long byteOffset, bool isFinal);

        void addReadedBytesCount(unsigned long n);
        void addSentBytesCount(unsigned long n);
        unsigned long getSendWindowSize() const;

        bool isBlocked() const;
        unsigned long blockedAt() const;
        bool receiveWindowHasUpdate();
        unsigned long receiveWindowUpdate();
    };
}

#endif
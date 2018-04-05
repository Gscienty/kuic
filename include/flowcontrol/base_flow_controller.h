#ifndef _KUIC_BASE_FLOW_CONTROLLER_
#define _KUIC_BASE_FLOW_CONTROLLER_

namespace kuic {
    class BaseFlowController {
    private:
        unsigned long bytesSent;
        unsigned long sendWindow;
        
    };
}

#endif
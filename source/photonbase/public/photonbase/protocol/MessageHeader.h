//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Types.h"

namespace pht {

/*
| Message ID | DUI[3] | I think Uint16 may be not large enough. Although this field is not very likely to be a small number, for DUI[3] is large enough, and we can save 1 or 2 bytes in some cases |
| Timestamp | DUI[4] | Milliseconds |
| Reserved | 3 bit | |
| Message Type | 5 bit | |
| Message Length | DUI[4] | Bytes |

| 0   | Control Message | Control Message is used to configure or operate the whole connection |
| 1   | Remote Method Invoke | a.k.a. Remote Process Call |
| 2   | Video Message | This message is a video frame or related parameters |
| 3   | Audio Message | This message is an audio message |

 */
struct MessageHeader {
    enum class Type : uint8_t {
        kControl = 0,
        kRemoteMethodInvoke = 1,
        kVideo = 2,
        kAudio = 3
    };

    Uint32 messageId { 0 };
    Uint32 timestamp { 0 };
    Uint8 reserved { 0 };
    Type messageType { 0 };
    Uint32 messageLength { 0 };
};

}
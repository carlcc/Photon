//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Types.h"

namespace pht {

struct ChunkHeader {
    Uint16 channelId;
    Uint32 chunkId;
    Uint32 chunkSize;
};

}
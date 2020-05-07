//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "photonbase/core/Types.h"

namespace pht {

class IApplication;

class ApplicationManager {
public:
    static IApplication* GetApplication(const String& appName);

private:
};

}
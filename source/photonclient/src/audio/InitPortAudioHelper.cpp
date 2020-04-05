//
// Created by carl on 2020/4/5 0005.
//

#include "InitPortAudioHelper.h"
#include <iostream>
#include <portaudio.h>
#include <string>

InitPortAudioHelper::InitPortAudioHelper()
{
    auto error = Pa_Initialize();
    if (paNoError != error) {
        std::cerr << "Initilize port audio failed" << std::endl;
        sIsInitialized_ = false;
        return;
    }
    std::cerr << "Initilize port audio succeed" << std::endl;

    sIsInitialized_ = true;
}

InitPortAudioHelper::~InitPortAudioHelper()
{
    if (sIsInitialized_) {
        sIsInitialized_ = false;
        Pa_Terminate();
    }
}

bool InitPortAudioHelper::sIsInitialized_ { false };
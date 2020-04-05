//
// Created by carl on 2020/4/5 0005.
//
#pragma once

struct InitPortAudioHelper {
    InitPortAudioHelper();

    ~InitPortAudioHelper();

    static const InitPortAudioHelper&Instance() {
        static InitPortAudioHelper s;
        return s;
    }

    static bool sIsInitialized_;
};
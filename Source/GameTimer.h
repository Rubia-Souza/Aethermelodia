//
// Created by Aline on 20/06/2025.
//
#pragma once

#include "SDL.h"

class GameTimer {
public:
    void start() {
        mStarted = true;
        mPaused = false;
        mStartTicks = SDL_GetPerformanceCounter();
        mPausedTicks = 0;
    }

    void stop() {
        mStarted = false;
        mPaused = false;
        mStartTicks = 0;
        mPausedTicks = 0;
    }

    void pause() {
        if (mStarted && !mPaused) {
            mPaused = true;
            mPausedTicks = SDL_GetPerformanceCounter() - mStartTicks;
            mStartTicks = 0;
        }
    }

    void unpause() {
        if (mStarted && mPaused) {
            mPaused = false;
            mStartTicks = SDL_GetPerformanceCounter() - mPausedTicks;
            mPausedTicks = 0;
        }
    }

    double getSeconds() {
        Uint64 time = 0;
        if (mStarted) {
            if (mPaused) {
                time = mPausedTicks;
            } else {
                time = SDL_GetPerformanceCounter() - mStartTicks;
            }
        }
        return (double)time / (double)SDL_GetPerformanceFrequency();
    }

private:
    Uint64 mStartTicks = 0;
    Uint64 mPausedTicks = 0;
    bool mStarted = false;
    bool mPaused = false;
};
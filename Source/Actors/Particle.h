//
// Created by aline on 23/06/2025.
//
#pragma once
#include "Actor.h"
 #include <SDL.h>

class Particle
{
public:
    Particle();

    void Init(const Vector2& pos, const Vector2& vel, float lifetime, SDL_Color color, float size);

    void Update(float deltaTime);
    void Draw(SDL_Renderer* renderer);

    bool IsDead() const { return mIsDead; }

private:
    Vector2 mPosition;
    Vector2 mVelocity;
    float mLifetime;
    float mAge;
    float mSize;
    SDL_Color mColor;
    bool mIsDead;
};
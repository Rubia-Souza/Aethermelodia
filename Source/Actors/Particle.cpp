//
// Created by aline on 23/06/2025.
//
#include "Particle.h"
#include "../Game.h"

Particle::Particle()
    : mPosition(Vector2::Zero)
    , mVelocity(Vector2::Zero)
    , mLifetime(1.0f)
    , mAge(0.0f)
    , mSize(1.0f)
    , mColor({255, 255, 255, 255})
    , mIsDead(true)
{
}

void Particle::Init(const Vector2& pos, const Vector2& vel, float lifetime, SDL_Color color, float size)
{
    mPosition = pos;
    mVelocity = vel;
    mLifetime = lifetime;
    mAge = 0.0f;
    mColor = color;
    mSize = size;
    mIsDead = false;
}

void Particle::Update(float deltaTime)
{
    if (mIsDead) return;

    mAge += deltaTime;
    if (mAge >= mLifetime)
    {
        mIsDead = true;
        return;
    }

    mPosition += mVelocity * deltaTime;
}

void Particle::Draw(SDL_Renderer* renderer)
{
    if (mIsDead) return;

    SDL_SetRenderDrawColor(renderer, mColor.r, mColor.g, mColor.b, mColor.a);

    int x = static_cast<int>(mPosition.x);
    int y = static_cast<int>(mPosition.y);

    for (int i = -static_cast<int>(mSize); i <= static_cast<int>(mSize); ++i)
    {
        for (int j = -static_cast<int>(mSize); j <= static_cast<int>(mSize); ++j)
        {
            if (i*i + j*j <= mSize * mSize)
                SDL_RenderDrawPoint(renderer, x + i, y + j);
        }
    }
}

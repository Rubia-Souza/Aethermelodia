#include "Target.h"
#include <cmath>
#include "../Game.h"

Target::Target(Game* game, const Vector2& pos, SDL_Color color, int lane, int size)
    : Actor(game)
    , mOriginalColor(color)
    , mLane(lane)
    , mIsFlashing(false)
    , mFlashTimer(0.0f)
{
    SetPosition(pos);

    constexpr int segments = 32;
    const auto radius = static_cast<float>(size);
    const float diameter = radius * 2.0f;

    constexpr float angleStep = 2.0f * static_cast<float>(M_PI) / segments;
    std::vector<Vector2> circleVerts;

    for (int i = 0; i < segments; i++) {
        const float angle = i * angleStep;
        circleVerts.emplace_back(radius * cosf(angle), radius * sinf(angle));
    }

    mDrawComponent = new DrawPolygonComponent(this, circleVerts);
    mDrawComponent->SetColor(mOriginalColor.r, mOriginalColor.g, mOriginalColor.b);

    mColliderComponent = new AABBColliderComponent(this, -radius, -radius, diameter, diameter, ColliderLayer::Target);
}

void Target::OnUpdate(float deltaTime)
{
    Actor::OnUpdate(deltaTime);

    // Se o alvo está no meio de um "flash"
    if (mIsFlashing)
    {
        mFlashTimer -= deltaTime;
        if (mFlashTimer <= 0.0f)
        {
            mIsFlashing = false;
            mFlashTimer = 0.0f;
            // O flash acabou, volta para a cor original
            mDrawComponent->SetColor(mOriginalColor.r, mOriginalColor.g, mOriginalColor.b);
        }
    }
}

void Target::Flash()
{
    // Só inicia um novo flash se não estiver piscando no momento
    if (!mIsFlashing)
    {
        mIsFlashing = true;
        mFlashTimer = 0.15f; // Duração do flash em segundos
        // Define a cor do componente para branco
        mDrawComponent->SetColor(255, 255, 255);
    }
}

SDL_Color Target::GetLaneColor(int lane) {

    switch (lane) {
        case 0:
            return SDL_Color{0, 255, 0, 255};
        case 1:
            return SDL_Color{255, 0, 0, 255};
        case 2:
            return SDL_Color{0, 0, 255, 255};
        case 3:
            return SDL_Color{255, 255, 0, 255};
    }

    return SDL_Color{255, 255, 255, 255};
}
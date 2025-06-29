//
// Created by Aline on 20/06/2025.
//

#pragma once
#include "Actor.h"
#include <SDL.h>
#include "../Components/DrawComponents/DrawPolygonComponent.h"

class Target : public Actor
{
public:
    Target(class Game* game, const Vector2& pos, SDL_Color color, int lane, int size = 30);

    void OnUpdate(float deltaTime) override;

    void Flash();

    int GetLane() const { return mLane; }

private:
    class DrawPolygonComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;

    SDL_Color mOriginalColor;
    int mLane;
    bool mIsFlashing;
    float mFlashTimer;
};
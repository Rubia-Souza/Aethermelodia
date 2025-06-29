//
// Created by Aline on 28/06/2025.
//

#pragma once
#include "Actor.h"

class Ground : public Actor
{
public:
    Ground(class Game* game, const Vector2& position);

private:
    class AABBColliderComponent* mCollider;
};
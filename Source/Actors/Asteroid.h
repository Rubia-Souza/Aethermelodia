//
// Created by Lucas N. Ferreira on 10/09/23.
//

#pragma once
#include "Actor.h"

class Asteroid : public Actor
{
public:
    Asteroid(Game* game, Vector2 spawnPos, Vector2 targetPos, int lane);
    ~Asteroid();

    void Update(float deltaTime);

    int GetLane() const { return mLane; }

private:
    static std::vector<Vector2> GenerateVertices(int numVertices, float radius);

    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;

    Vector2 mTargetPos;
    int mLane;
};

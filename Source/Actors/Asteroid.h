//
// Created by Lucas N. Ferreira on 10/09/23.
//

#pragma once
#include "Actor.h"

class Asteroid : public Actor
{
public:
    Asteroid(Game* game, float radius, int numVertices = 10, float forwardForce = 1000.0f);
    Asteroid(Game* game, Vector2 spawnPos, Vector2 targetPos, int lane);
    ~Asteroid();

    void Update(float deltaTime);

    int GetLane() const { return mLane; } // Adicione um getter

private:
    static std::vector<Vector2> GenerateVertices(int numVertices, float radius);
    static Vector2 GenerateRandomStartingForce(float min, float max);

    static float CalculateAverageVerticesLength(std::vector<Vector2>& vertices);

    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;

    Vector2 mTargetPos; // Para guardar a posição do alvo
    int mLane;
};

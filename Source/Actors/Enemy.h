//
// Created by Lucas N. Ferreira on 30/09/23.
//

#pragma once

#include "Actor.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"

class Enemy : public Actor
{
public:
    std::vector<Vector2> GenerateVertices(int num_vertices, float radius);

    explicit Enemy(Game* game, Note note, Vector2 spawnPos, Vector2 targetPos,   float forwardSpeed = 100.0f, float deathTime = 0.2f);

    ~Enemy();

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    // TODO qdo for passar o update pro geral dos actors, retirar isso aq
    // void Update(float deltaTime);

    void Kill() override;
    void BumpKill(const float bumpForce = 300.0f);

    int GetLane() const { return mNote.lane; }

    void setHit(bool hit)  { this->mHit = hit; mDrawExplosionComponent->SetIsVisible(mHit); }
    bool getHit() const { return mHit; }

    int GetDurationInSeconds() const { return mNote.durationInSeconds; }

    const float NOTE_VELOCITY = 250.0f; // Ajuste para a velocidade desejada

private:
    bool mIsDying;
    float mForwardSpeed;
    float mDyingTimer;

    Vector2 mTargetPos;
    Note mNote;
    bool mIsMirrored;
    bool mHit = false;

    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
    class DrawRectangleComponent* mDrawTailComponent;
    class DrawAnimatedComponent* mDrawExplosionComponent;
};
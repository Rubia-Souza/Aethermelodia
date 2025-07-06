//
// Created by Aline on 27/06/2025.
//
#pragma once
#include "Actor.h"

class Lirael : public Actor
{
public:
    enum class LiraelState
    {
        Idle,
        MovingToTarget,
        WaitingAtTarget
    };

    explicit Lirael(Game* game);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    void OnHandleKeyPress(const int key, const bool isPressed) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;

private:
    void ManageAnimations();

    bool mIsDying;
    Vector2 mInitialPosition;
    bool mInitialPositionSet = false;

    class Target* mCurrentTarget = nullptr;
    LiraelState mState = LiraelState::Idle;

    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;

    Vector2 mJumpStartPos;
    Vector2 mJumpEndPos;
    float mJumpTime;
    float mJumpDuration;
    float mJumpHeight;

    void MoveToTarget(int lane);
    void ReturnToInitialPosition();
};
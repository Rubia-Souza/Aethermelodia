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

    explicit Lirael(Game* game, float forwardSpeed = 1500.0f, float jumpSpeed = -750.0f);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    void OnHandleKeyPress(const int key, const bool isPressed) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;

private:
    void ManageAnimations();

    float mForwardSpeed;
    float mJumpSpeed;
    bool mIsDying;

    Vector2 mInitialPosition;
    bool mInitialPositionSet = false;
    float mMovementSpeed = 700.0f;

    class Target* mCurrentTarget = nullptr;
    LiraelState mState = LiraelState::Idle;

    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;

    Vector2 mJumpStartPos;      // Posição inicial do pulo
    Vector2 mJumpEndPos;        // Posição final do pulo (o alvo)
    float mJumpTime;            // Tempo decorrido no pulo atual
    float mJumpDuration;        // Duração total que o pulo deve levar
    float mJumpHeight;          // Altura do arco do pulo

    void MoveToTarget(int lane);
    void ReturnToInitialPosition();
};
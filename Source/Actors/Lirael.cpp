//
// Created by Aline on 27/06/2025.
//

#include "Lirael.h"
#include "Block.h"
#include "Collectable.h"
#include "Target.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"

Lirael::Lirael(Game* game, const float forwardSpeed, const float jumpSpeed)
        : Actor(game)
        , mIsDying(false)
        , mForwardSpeed(forwardSpeed)
        , mJumpSpeed(jumpSpeed)
        , mMovementSpeed(900.0f)
{
    const int liraelWidth = 64.0f;
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, liraelWidth,Game::TILE_SIZE, ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this,
                                              "../Assets/Sprites/Lirael/Lirael.png",
                                              "../Assets/Sprites/Lirael/Lirael.json"); // Mudar

    mRigidBodyComponent->SetApplyFriction(false);

    mDrawComponent->AddAnimation("idle", {0});
    mDrawComponent->AddAnimation("Dead", {1});
    // mDrawComponent->AddAnimation("jump", {2});
    // mDrawComponent->AddAnimation("run", {3, 4, 5});
    // mDrawComponent->AddAnimation("win", {7});

    mDrawComponent->SetAnimation("idle");
    mDrawComponent->SetAnimFPS(10.0f);
}

void Lirael::OnProcessInput(const uint8_t* state)
{
    if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_F]) {
        mRotation = 0.0f;
    }

    if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_A]) {
        mRotation = Math::Pi;
    }
}

void Lirael::OnHandleKeyPress(const int key, const bool isPressed)
{
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    if (isPressed) {
        if (mState == LiraelState::Idle) {
            int lane = -1;
            switch (key)
            {
            case SDLK_a: lane = 0; break;
            case SDLK_d: lane = 1; break;
            case SDLK_s: lane = 2; break;
            case SDLK_f: lane = 3; break;
            }

            if (lane != -1) MoveToTarget(lane);
        }
    } else {
        if (mState == LiraelState::MovingToTarget || mState == LiraelState::WaitingAtTarget) {
            if(bool isGameKey = (key == SDLK_a || key == SDLK_s || key == SDLK_d || key == SDLK_f)) ReturnToInitialPosition();
        }
    }
}

void Lirael::MoveToTarget(int lane)
{
    const auto& targets = mGame->GetTargets();
    Target* target = nullptr;
    for (auto t : targets) {
        if (t->GetLane() == lane) {
            target = t;
            break;
        }
    }

    if (target) {
        mState = LiraelState::MovingToTarget;
        mCurrentTarget = target;

        float jumpStrength = 0.0f;

        if (lane == 0 || lane == 1) { // Alvos de CIMA
            jumpStrength = 880.0f; // Pulo alto
        } else { // Alvos de BAIXO
            jumpStrength = 380.0f; // Pulo mais curto e baixo
        }

        mRigidBodyComponent->SetApplyGravity(true);
        mRigidBodyComponent->SetVelocity(Vector2(0.0f, -jumpStrength));
    }
}

void Lirael::ReturnToInitialPosition()
{
    mState = LiraelState::Idle;
    mCurrentTarget = nullptr;
    mRigidBodyComponent->SetApplyGravity(true);
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    SetPosition(mInitialPosition);
}

void Lirael::OnUpdate(float deltaTime)
{
    if (!mInitialPositionSet && mIsOnGround) {
        mInitialPosition = GetPosition();
        mInitialPositionSet = true;
    }

    if (mState == LiraelState::MovingToTarget)
    {
        if (mCurrentTarget != nullptr)
        {
            const float horizontalForce = 1500.0f;
            Vector2 targetPos = mCurrentTarget->GetPosition();
            float dirX = targetPos.x - GetPosition().x > 0 ? 1.0f : -1.0f;

            if (Math::Abs(targetPos.x - GetPosition().x) > 5.0f)
            {
                mRigidBodyComponent->ApplyForce(Vector2::UnitX * dirX * horizontalForce);
            }

            auto targetCollider = mCurrentTarget->GetComponent<AABBColliderComponent>();
            if (mColliderComponent->Intersect(*targetCollider))
            {
                mRigidBodyComponent->SetVelocity(Vector2::Zero);
                mRigidBodyComponent->SetAcceleration(Vector2::Zero);
                mRigidBodyComponent->SetApplyGravity(false);

                Vector2 targetCenter = targetCollider->GetCenter();
                Vector2 liraelCenter = mColliderComponent->GetCenter();

                Vector2 dirFromTarget = liraelCenter - targetCenter;
                dirFromTarget.Normalize();

                float targetRadius = targetCollider->GetWidth() / 2.0f;
                float liraelHalfWidth = mColliderComponent->GetWidth() / 2.0f;
                float idealDist = targetRadius + liraelHalfWidth;

                Vector2 finalLiraelCenter = targetCenter + dirFromTarget * idealDist;
                Vector2 finalActorPos = finalLiraelCenter - Vector2(liraelHalfWidth, mColliderComponent->GetHeight() / 2.0f);
                SetPosition(finalActorPos);

                mState = LiraelState::WaitingAtTarget;
            }
        }
    }

    if (mRigidBodyComponent && mRigidBodyComponent->GetVelocity().y != 0 && mState == LiraelState::Idle) {
        mIsOnGround = false;
    }
}

void Lirael::ManageAnimations()
{
// TODO: add animations
}

void Lirael::Kill()
{
    mIsDying = true;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
    mDrawComponent->SetAnimation("Dead");

    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    mGame->GetAudio()->StopAllSounds();
    mGame->ResetGameScene(2.0f);
}

void Lirael::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{

}

void Lirael::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Ground) {
        if (mRigidBodyComponent->GetVelocity().y > 0.0f) {
            mIsOnGround = true;
            Vector2 pos = GetPosition();
            pos.y -= minOverlap;
            SetPosition(pos);
            mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, 0.0f));
        }
    }
}
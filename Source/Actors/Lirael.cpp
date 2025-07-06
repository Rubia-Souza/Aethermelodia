//
// Created by Aline on 27/06/2025.
//

#include "Lirael.h"
#include "Target.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

Lirael::Lirael(Game* game)
        : Actor(game)
        , mIsDying(false)
{
    const int liraelWidth = 64.0f;
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, liraelWidth,Game::TILE_SIZE, ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this,
                                              "../Assets/Sprites/Lirael/Lirael.png",
                                              "../Assets/Sprites/Lirael/Lirael.json");

    mRigidBodyComponent->SetApplyFriction(false);

    mDrawComponent->AddAnimation("Idle", {5});
    mDrawComponent->AddAnimation("Dead", {4});
    mDrawComponent->AddAnimation("Attack", {0, 1, 2, 3});

    mDrawComponent->SetAnimation("Idle");
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

// void Lirael::OnProcessInput(const uint8_t* state)
// {
//     if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;
//
//     const bool aDown = state[SDL_SCANCODE_A];
//     const bool sDown = state[SDL_SCANCODE_S];
//     const bool dDown = state[SDL_SCANCODE_D];
//     const bool fDown = state[SDL_SCANCODE_F];
//
//     if (const int keysPressed = aDown + sDown + dDown + fDown; keysPressed > 1) {
//         if (aDown && sDown) {
//             CenterOnScreen(Math::Pi);
//         } else if (dDown && fDown) {
//             CenterOnScreen(0.0f);
//         } else {
//             CenterOnScreen(GetRotation());
//         }
//         return;
//     }
//
//     if (dDown || fDown) {
//         mRotation = 0.0f;
//     }
//     else if (sDown || aDown) {
//         mRotation = Math::Pi;
//     }
// }

void Lirael::OnHandleKeyPress(const int key, const bool isPressed)
{
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    if (isPressed) {
        int lane = -1;
        switch (key)
        {
        case SDLK_a: lane = 0; break;
        case SDLK_d: lane = 1; break;
        case SDLK_s: lane = 2; break;
        case SDLK_f: lane = 3; break;
        }

        if (lane != -1) {
            if (mCurrentTarget && mCurrentTarget->GetLane() == lane) {
                return;
            }
            MoveToTarget(lane);
        }
    } else {
        if (key == SDLK_a || key == SDLK_s || key == SDLK_d || key == SDLK_f) {
            const Uint8* state = SDL_GetKeyboardState(NULL);
            if (!state[SDL_SCANCODE_A] && !state[SDL_SCANCODE_S] &&
                !state[SDL_SCANCODE_D] && !state[SDL_SCANCODE_F])
            {
                ReturnToInitialPosition();
            }
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
        mRigidBodyComponent->SetEnabled(false);
        mCurrentTarget = target;
        mJumpStartPos = GetPosition();

        const auto targetCollider = target->GetComponent<AABBColliderComponent>();
        const Vector2 targetWorldCenter = target->GetPosition();

        const float targetRadius = targetCollider->GetWidth() / 2.0f;
        float liraelWidth = mColliderComponent->GetWidth();
        const float visualOverlap = 45.0f;

        float finalX;
        if (lane == 1 || lane == 3) {
            finalX = (targetWorldCenter.x - targetRadius) - visualOverlap;
        } else {
            finalX = (targetWorldCenter.x + targetRadius - liraelWidth) + visualOverlap;
        }
        float finalY = targetWorldCenter.y - mColliderComponent->GetHeight();

        mJumpEndPos = Vector2(finalX, finalY);

        float distance = Vector2::Distance(mJumpStartPos, mJumpEndPos);
        const float heightMultiplier = 0.3f;
        const float durationMultiplier = 0.0012f;
        const float minDuration = 0.35f;

        mJumpTime = 0.0f;
        mJumpHeight = distance * heightMultiplier;
        mJumpDuration = distance * durationMultiplier;

        if (mJumpDuration < minDuration) {
            mJumpDuration = minDuration;
        }
    }
}

void Lirael::ReturnToInitialPosition()
{
    mRigidBodyComponent->SetEnabled(true);
    mState = LiraelState::Idle;
    mCurrentTarget = nullptr;
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    mRigidBodyComponent->SetAcceleration(Vector2::Zero);
    SetPosition(mInitialPosition);
}

void Lirael::OnUpdate(float deltaTime)
{
    if (!mInitialPositionSet && mIsOnGround) {
        mInitialPosition = GetPosition();
        mInitialPositionSet = true;
    }

    if (mState == LiraelState::MovingToTarget) {
        mJumpTime += deltaTime;
        float t = mJumpTime / mJumpDuration;

        if (t >= 1.0f) {
            SetPosition(mJumpEndPos);
            mState = LiraelState::WaitingAtTarget;
        } else {
            Vector2 newPos = Vector2::Lerp(mJumpStartPos, mJumpEndPos, t);
            float arc = mJumpHeight * sin(Math::Pi * t);
            newPos.y -= arc;
            SetPosition(newPos);
        }
    }

    ManageAnimations();
}

void Lirael::ManageAnimations()
{
    if(mIsDying) {
        mDrawComponent->SetAnimation("Dead");
    } else if (mState == LiraelState::MovingToTarget || mState == LiraelState::WaitingAtTarget) {
        mDrawComponent->SetAnimation("Attack");
    } else if (mIsOnGround) {
        mDrawComponent->SetAnimation("Idle");
    }
}

void Lirael::Kill()
{
    mIsDying = true;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
    mDrawComponent->SetAnimation("Dead");

    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    mGame->GetAudio()->StopAllSounds();
    mGame->GameOver(2.0f);
}

void Lirael::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
}

void Lirael::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (mRigidBodyComponent->IsEnabled() && other->GetLayer() == ColliderLayer::Ground) {
        if (mRigidBodyComponent->GetVelocity().y > 0.0f) {
            mIsOnGround = true;
            Vector2 pos = GetPosition();
            pos.y -= minOverlap;
            SetPosition(pos);
            mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, 0.0f));
        }
    }
}

void Lirael::CenterOnScreen(float rotation)
{
    if (mState == LiraelState::MovingToTarget || mState == LiraelState::WaitingAtTarget) {
        ReturnToInitialPosition();
    }

    float screenWidth = mGame->GetWindowWidth();
    float screenHeight = mGame->GetWindowHeight();

    float finalX = (screenWidth / 2.0f) - (mColliderComponent->GetWidth() / 2.0f);
    float finalY = screenHeight * 0.74f;

    SetPosition(Vector2(finalX, finalY));
    SetRotation(rotation);
}
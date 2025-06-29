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
        , mIsRunning(false)
        , mIsOnPole(false)
        , mIsDying(false)
        , mForwardSpeed(forwardSpeed)
        , mJumpSpeed(jumpSpeed)
        , mPoleSlideTimer(0.0f)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE - 4.0f,Game::TILE_SIZE,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawSpriteComponent(this, "../Assets/Sprites/Lirael/Lirael.png", 64, 64, 10);

//    mDrawComponent = new DrawAnimatedComponent(this,
//                                              "../Assets/Sprites/Lirael/Lirael.png",
//                                              "../Assets/Sprites/Mario/Mario.json"); // Mudar

    // mDrawComponent->AddAnimation("Dead", {0});
    // mDrawComponent->AddAnimation("idle", {1});
    // mDrawComponent->AddAnimation("jump", {2});
    // mDrawComponent->AddAnimation("run", {3, 4, 5});
    // mDrawComponent->AddAnimation("win", {7});
    //
    // mDrawComponent->SetAnimation("idle");
    // mDrawComponent->SetAnimFPS(10.0f);
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

        if (lane == 0 || lane == 1) // Alvos de CIMA (A e D)
        {
            mRigidBodyComponent->SetApplyGravity(true);

            const float JUMP_STRENGTH = 750.0f;
            const float JUMP_HORIZONTAL_SPEED = 280.0f;

            float horizontalSpeed = (lane == 0) ? -JUMP_HORIZONTAL_SPEED : JUMP_HORIZONTAL_SPEED;

            mRigidBodyComponent->SetVelocity(Vector2(horizontalSpeed, -JUMP_STRENGTH));
        }
        else // Alvos de BAIXO (S e F)
        {
            mRigidBodyComponent->SetApplyGravity(false);

            Vector2 dir = target->GetPosition() - GetPosition();
            dir.Normalize();
            mRigidBodyComponent->SetVelocity(dir * mMovementSpeed);
        }
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

    if (mState == LiraelState::MovingToTarget) {
        if (mCurrentTarget != nullptr) {
            auto targetCollider = mCurrentTarget->GetComponent<AABBColliderComponent>();
            if (mColliderComponent->Intersect(*targetCollider))
            {
                mRigidBodyComponent->SetApplyGravity(false);

                mRigidBodyComponent->SetVelocity(Vector2::Zero);
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
//    if(mIsDying)
//    {
//        mDrawComponent->SetAnimation("Dead");
//    }
//    else if (mIsOnGround && mIsRunning)
//    {
//        mDrawComponent->SetAnimation("run");
//    }
//    else if (mIsOnGround && !mIsRunning)
//    {
//        mDrawComponent->SetAnimation("idle");
//    }
//    else if (!mIsOnGround)
//    {
//        mDrawComponent->SetAnimation("jump");
//    }
}

void Lirael::Kill()
{
    mIsDying = true;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
//    mDrawComponent->SetAnimation("Dead");

    // Disable collider and rigid body
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    mGame->GetAudio()->StopAllSounds();
    mGame->ResetGameScene(3.5f); // Reset the game scene after 3 seconds
}

void Lirael::Win(AABBColliderComponent *poleCollider)
{
//    mDrawComponent->SetAnimation("win");
    mGame->SetGamePlayState(Game::GamePlayState::LevelComplete);
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
//
// Created by Aline on 27/06/2025.
//

#include "Lirael.h"
#include "Block.h"
#include "Collectable.h"
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
    // if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;
    //
    // if (state[SDL_SCANCODE_D])
    // {
    //     mRigidBodyComponent->ApplyForce(Vector2::UnitX * mForwardSpeed);
    //     mRotation = 0.0f;
    //     mIsRunning = true;
    // }
    //
    // if (state[SDL_SCANCODE_A])
    // {
    //     mRigidBodyComponent->ApplyForce(Vector2::UnitX * -mForwardSpeed);
    //     mRotation = Math::Pi;
    //     mIsRunning = true;
    // }
    //
    // if (!state[SDL_SCANCODE_D] && !state[SDL_SCANCODE_A])
    // {
    //     mIsRunning = false;
    // }
}

void Lirael::OnHandleKeyPress(const int key, const bool isPressed)
{
    if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    // Jump
    if (key == SDLK_SPACE && isPressed && mIsOnGround)
    {
        mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed));
        mIsOnGround = false;
    }
}

void Lirael::OnUpdate(float deltaTime)
{
    // Check if Lirael is off the ground
    if (mRigidBodyComponent && mRigidBodyComponent->GetVelocity().y != 0)
    {
        mIsOnGround = false;
    }

//    ManageAnimations();
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
    if (other->GetLayer() == ColliderLayer::Ground)
    {
        if (mRigidBodyComponent->GetVelocity().y > 0.0f)
        {
            mIsOnGround = true;
            Vector2 pos = GetPosition();
            pos.y -= minOverlap;
            SetPosition(pos);
            mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, 0.0f));
        }
    }
}
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
        , mMovementSpeed(900.0f)
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

// Em Lirael.cpp

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

        // Definimos a força do pulo baseada no alvo
        float jumpStrength = 0.0f;

        if (lane == 0 || lane == 1) { // Alvos de CIMA
            jumpStrength = 850.0f; // Pulo alto
        } else { // Alvos de BAIXO
            jumpStrength = 400.0f; // Pulo mais curto e baixo
        }

        // AGORA A LÓGICA É A MESMA PARA TODOS:
        // 1. Ativa a gravidade para criar o arco do pulo
        mRigidBodyComponent->SetApplyGravity(true);

        // 2. Aplica o impulso vertical inicial com a força que definimos
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

// Em Lirael.cpp

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
            // --- A FORÇA GUIA AGORA É APLICADA PARA TODOS OS PULOS ---
            const float horizontalForce = 1500.0f;
            Vector2 targetPos = mCurrentTarget->GetPosition();
            float dirX = targetPos.x - GetPosition().x > 0 ? 1.0f : -1.0f;

            // Aplica a força se não estiver horizontalmente alinhado
            if (Math::Abs(targetPos.x - GetPosition().x) > 5.0f)
            {
                mRigidBodyComponent->ApplyForce(Vector2::UnitX * dirX * horizontalForce);
            }
            // ---------------------------------------------------------

            // Detecção de colisão e alinhamento (esta parte não muda, já funciona para todos)
            auto targetCollider = mCurrentTarget->GetComponent<AABBColliderComponent>();
            if (mColliderComponent->Intersect(*targetCollider))
            {
                // Para o movimento imediatamente
                mRigidBodyComponent->SetVelocity(Vector2::Zero);
                mRigidBodyComponent->SetApplyGravity(false);

                // --- CÁLCULO DA POSIÇÃO FINAL ABSOLUTA ---

                // 1. Pega os centros dos dois colisores
                Vector2 targetCenter = targetCollider->GetCenter();
                Vector2 liraelCenter = mColliderComponent->GetCenter();

                // 2. Calcula o vetor de direção do centro do alvo para o Lirael
                Vector2 dirFromTarget = liraelCenter - targetCenter;
                dirFromTarget.Normalize(); // Agora temos apenas a direção

                // 3. Calcula a distância ideal entre os centros (soma dos raios)
                float targetRadius = targetCollider->GetWidth() / 2.0f;
                float liraelHalfWidth = mColliderComponent->GetWidth() / 2.0f;
                float idealDist = targetRadius + liraelHalfWidth;

                // 4. Calcula a posição final ideal para o *centro* do Lirael
                Vector2 finalLiraelCenter = targetCenter + dirFromTarget * idealDist;

                // 5. Converte a posição final do centro para a posição do ator (top-left)
                // A posição do ator é o seu canto superior esquerdo.
                // Posição(ator) = Posição(centro) - (largura/2, altura/2)
                Vector2 finalActorPos = finalLiraelCenter - Vector2(liraelHalfWidth, mColliderComponent->GetHeight() / 2.0f);

                // 6. Define a posição do Lirael para o local exato e corrigido
                SetPosition(finalActorPos);

                // ------------------------------------------

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
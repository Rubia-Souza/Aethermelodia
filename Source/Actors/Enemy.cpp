//
// Created by Lucas N. Ferreira on 30/09/23.
//

#include "Enemy.h"

#include "Target.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Random.h"
#include "../Components/DrawComponents/DrawRectangleComponent.h"

Enemy::Enemy(Game* game,Note note, Vector2 spawnPos, Vector2 targetPos, float forwardSpeed, float deathTime)
        : Actor(game)
        , mDyingTimer(deathTime)
        , mIsDying(false)
        , mForwardSpeed(forwardSpeed)
        , mNote(note)
        , mTargetPos(targetPos)
{

    mColliderComponent = new AABBColliderComponent(this, 0, 0,
                                                   Game::TILE_SIZE, Game::TILE_SIZE,
                                                   ColliderLayer::Enemy);
    mColliderComponent->SetEnabled(false);

    mDrawComponent = new DrawAnimatedComponent(this,
                                                  "../Assets/Sprites/Goomba/Goomba.png",
                                                  "../Assets/Sprites/Goomba/Goomba.json");

    mDrawComponent->AddAnimation("Dead", {0});
    mDrawComponent->AddAnimation("Idle", {1});
    mDrawComponent->AddAnimation("walk", {1, 2});
    mDrawComponent->SetAnimation("walk");
    mDrawComponent->SetAnimFPS(5.0f);


    mIsMirrored = targetPos.x > spawnPos.x;
    mDrawTailComponent = new class DrawRectangleComponent(
        this,
        NOTE_VELOCITY * mNote.durationInSeconds ,
        15,
        Vector2(mIsMirrored ? 0: 32, 8),
        mIsMirrored,
        99);

    // --- novo
    SetPosition(spawnPos);

    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);

    // 1. Desligar a gravidade para que as notas não caiam sozinhas.
    mRigidBodyComponent->SetApplyGravity(false);
    // 2. Desligar o atrito para que as notas não percam velocidade.
    mRigidBodyComponent->SetApplyFriction(false);

    // 3. Definir a velocidade constante em vez de aplicar uma força.
    Vector2 direction = targetPos - spawnPos;
    direction.Normalize();

    mRigidBodyComponent->SetVelocity(direction * NOTE_VELOCITY);

    game->AddEnemy(this);
}

Enemy::~Enemy()
{

    if (mHit) {
        // SDL_Log("acertou");
        mGame->addScore(100);
        mGame->SetCurrentLives(mGame->GetCurrentLives() + 1);
    }
    else {
        // A NOTA MORREU SEM SER ACERTADA
        // SDL_Log("errou");
        mGame->addScore(-100);
        mGame->SetCurrentLives(mGame->GetCurrentLives() - 1);
    }

    GetGame()->RemoveEnemy(this);
}


void Enemy::Kill()
{
    mIsDying = true;
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

}

void Enemy::BumpKill(const float bumpForce)
{
    mDrawComponent->SetAnimation("Idle");

    mRigidBodyComponent->SetVelocity(Vector2(bumpForce/2.0f, -bumpForce));
    mColliderComponent->SetEnabled(false);

    // Flip upside down (180 degrees)
    SetRotation(180);
}

void Enemy::OnUpdate(float deltaTime)
{
    if (mIsDying)
    {
        mDyingTimer -= deltaTime;
        if (mDyingTimer <= 0.0f) {
            mState = ActorState::Destroy;
        }
    }


    // Actor::Update(deltaTime);



    if (!mRigidBodyComponent)
    {
        return;
    }
    // logica do hit
    if (mHit) {
        mDrawTailComponent->SetColor(Target::GetLaneColor(mNote.lane));
        mDrawComponent->SetAnimation("Dead");
    } else {
        mDrawTailComponent->SetColor(SDL_Color{255, 255, 255, 255});
        mDrawComponent->SetAnimation("walk");
    }


    Vector2 currentPos = GetPosition();
    Vector2 velocity = mRigidBodyComponent->GetVelocity();

    // O raio do nosso alvo, que usaremos como margem.
    // O construtor do Target usa 30 como tamanho padrão.
    const float targetRadius = 30.0f;

    // Caso 1: O inimigo está se movendo para a DIREITA
    if (mIsMirrored)
    {

        // destrói se a posição X passar do centro do alvo + seu raio (a borda direita)
        if (currentPos.x >= (mTargetPos.x - targetRadius/2))
        {
            mRigidBodyComponent->SetVelocity(Vector2(0.0f, 0.0f));

            float deltaWidth = deltaTime * NOTE_VELOCITY;

            float newWidth = mDrawTailComponent->GetWidth() - deltaWidth;
            newWidth = newWidth < 0 ? 0 : newWidth;
            mDrawTailComponent->SetWidth( newWidth);

        }

    }
    // Caso 2: O asteroide está se movendo para a ESQUERDA
    else if (!mIsMirrored)
    {
        // destrói se passar do centro do alvo - seu raio (a borda esquerda)
        if (currentPos.x <= (mTargetPos.x - targetRadius/2))
        {
            mRigidBodyComponent->SetVelocity(Vector2(0.0f, 0.0f));

            float deltaWidth = deltaTime * NOTE_VELOCITY;

            float newWidth = mDrawTailComponent->GetWidth() - deltaWidth;
            newWidth = newWidth < 0 ? 0 : newWidth;
            mDrawTailComponent->SetWidth( newWidth);
        }
    }


    if (velocity.x == 0.0f && mDrawTailComponent->GetWidth() <= 0) {
        this->Kill();
    }

}

void Enemy::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // if ((other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Enemy))
    // {
    //     if (minOverlap > 0) {
    //         mRigidBodyComponent->SetVelocity(Vector2(-mForwardSpeed, 0.0f));
    //     }
    //     else {
    //         mRigidBodyComponent->SetVelocity(Vector2(mForwardSpeed, 0.0f));
    //     }
    // }
    //
    // if (other->GetLayer() == ColliderLayer::Player) {
    //     other->GetOwner()->Kill();
    // }
}

void Enemy::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // if (other->GetLayer() == ColliderLayer::Player) {
    //     other->GetOwner()->Kill();
    // }
}




// void Enemy::Update(float deltaTime)
// {
//     Actor::Update(deltaTime);
//
//     if (!mRigidBodyComponent)
//     {
//         return;
//     }
//
//     Vector2 currentPos = GetPosition();
//     Vector2 velocity = mRigidBodyComponent->GetVelocity();
//
//     // O raio do nosso alvo, que usaremos como margem.
//     // O construtor do Target usa 30 como tamanho padrão.
//     const float targetRadius = 30.0f;
//
//     // Caso 1: O asteroide está se movendo para a DIREITA
//     if (velocity.x > 0.0f)
//     {
//         // destrói se a posição X passar do centro do alvo + seu raio (a borda direita)
//         if (currentPos.x >= (mTargetPos.x + targetRadius))
//         {
//             SetState(ActorState::Destroy);
//         }
//     }
//     // Caso 2: O asteroide está se movendo para a ESQUERDA
//     else if (velocity.x < 0.0f)
//     {
//         // destrói se passar do centro do alvo - seu raio (a borda esquerda)
//         if (currentPos.x <= (mTargetPos.x - targetRadius))
//         {
//             SetState(ActorState::Destroy);
//         }
//     }
// }
//
//



std::vector<Vector2> Enemy::GenerateVertices(const int numVertices, const float radius)
{
    // std::vector<Vector2> vertices;
    //
    // float angle = 0.0f;
    //
    // for (int i = 0; i < numVertices; i++) {
    //     float randLength = (Random::GetFloat() * 0.5f + 0.5f) * radius;
    //
    //     float x = randLength * cosf(angle);
    //     float y = randLength * sinf(angle);
    //
    //     vertices.push_back(Vector2(x, y));
    //
    //     angle += static_cast<float>(2.0f * M_PI / numVertices);
    // }
    //
    // return vertices;



    std::vector<Vector2> vertices;

    float angle = 0.0f;

    for (int i = 0; i < numVertices; i++) {
        float randLength = (Random::GetFloat() * 0.5f + 0.5f) * radius;

        float x = randLength * cosf(angle);
        float y = randLength * sinf(angle);

        vertices.push_back(Vector2(x, y));

        angle += static_cast<float>(2.0f * M_PI / numVertices);
    }

    return vertices;
}
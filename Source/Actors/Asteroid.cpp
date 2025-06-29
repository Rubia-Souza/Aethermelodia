//
// Created by Lucas N. Ferreira on 10/09/23.
//

#define DB_PERLIN_IMPL

#include "Asteroid.h"
#include "../Game.h"
#include "../Random.h"
#include "../Components/ColliderComponents/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawComponent.h"
#include "../Components//DrawComponents//DrawPolygonComponent.h"

Asteroid::Asteroid(Game* game, Vector2 spawnPos, Vector2 targetPos, int lane)
    :Actor(game)
    ,mRigidBodyComponent(nullptr)
    ,mDrawComponent(nullptr)
    ,mTargetPos(targetPos)
    ,mLane(lane)
{
    SetPosition(spawnPos);

    const float radius = 10.0f;
    const int numVertices = 12;

    std::vector<Vector2> vertices = GenerateVertices(numVertices, radius);
    mDrawComponent = new DrawPolygonComponent(this, vertices);

    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mCircleColliderComponent = new CircleColliderComponent(this, radius);

    // 1. Desligar a gravidade para que as notas não caiam sozinhas.
    mRigidBodyComponent->SetApplyGravity(false);
    // 2. Desligar o atrito para que as notas não percam velocidade.
    mRigidBodyComponent->SetApplyFriction(false);

    // 3. Definir a velocidade constante em vez de aplicar uma força.
    Vector2 direction = targetPos - spawnPos;
    direction.Normalize();
    const float NOTE_VELOCITY = 250.0f; // Ajuste para a velocidade desejada
    mRigidBodyComponent->SetVelocity(direction * NOTE_VELOCITY);

    game->AddAsteroid(this);
}

Asteroid::~Asteroid()
{
    GetGame()->RemoveAsteroid(this);
}

void Asteroid::Update(float deltaTime)
{
    Actor::Update(deltaTime);

    if (!mRigidBodyComponent)
    {
        return;
    }

    Vector2 currentPos = GetPosition();
    Vector2 velocity = mRigidBodyComponent->GetVelocity();

    // O raio do nosso alvo, que usaremos como margem.
    // O construtor do Target usa 30 como tamanho padrão.
    const float targetRadius = 30.0f;

    // Caso 1: O asteroide está se movendo para a DIREITA
    if (velocity.x > 0.0f)
    {
        // destrói se a posição X passar do centro do alvo + seu raio (a borda direita)
        if (currentPos.x >= (mTargetPos.x + targetRadius))
        {
            SetState(ActorState::Destroy);
        }
    }
    // Caso 2: O asteroide está se movendo para a ESQUERDA
    else if (velocity.x < 0.0f)
    {
        // destrói se passar do centro do alvo - seu raio (a borda esquerda)
        if (currentPos.x <= (mTargetPos.x - targetRadius))
        {
            SetState(ActorState::Destroy);
        }
    }
}

std::vector<Vector2> Asteroid::GenerateVertices(const int numVertices, const float radius)
{
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

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

Asteroid::Asteroid(Game* game, const float radius, const int numVertices, const float forwardForce)
        :Actor(game)
        ,mRigidBodyComponent(nullptr)
        ,mDrawComponent(nullptr)
        ,mTargetPos(Vector2::Zero)
{
    // Create a circle with numVertices
    std::vector<Vector2> vertices = GenerateVertices(numVertices, radius);

    // Calculate average vertices length to be the collider radius
    float averageLength = CalculateAverageVerticesLength(vertices);

    float naveRadius = 30.0f; // Hardcoded here with the same value from Game, ideally I would use a GetShip() method
    float safeDist = naveRadius * 2 + averageLength; // Random calculation to create an space between ship and asteroids

    Vector2 randPos = Random::GetIntRange(0, 1) == 0
        ? Random::GetVector(Vector2(0, 0), Vector2(mGame->GetWindowWidth() / 2 - safeDist, mGame->GetWindowHeight()))
        : Random::GetVector(Vector2(mGame->GetWindowWidth() / 2 + safeDist, 0), Vector2(mGame->GetWindowWidth(), mGame->GetWindowHeight()));

    SetPosition(randPos);

    // Generate random starting force
    Vector2 randStartingForce = GenerateRandomStartingForce(700.0f, 1000.0f);

    mDrawComponent = new DrawComponent(this);
    mRigidBodyComponent = new RigidBodyComponent(this, 2.0f);
    mCircleColliderComponent = new CircleColliderComponent(this, averageLength);

    mRigidBodyComponent->ApplyForce(randStartingForce);

    game->AddAsteroid(this);

    // CUSTOMIZATION: Random colors for asteroids
    std::vector<SDL_Color> asteroidColors;
    asteroidColors = {
        {255, 165, 0, 255}, // orange
        {150, 75, 0, 255}, // brown
        {220, 180, 120, 255}, // sand
        {255, 182, 193, 255}, // light pink
    };
    // Choose a random color from the palette
    int index = Random::GetIntRange(0, static_cast<int>(asteroidColors.size()) - 1);
    SDL_Color color = asteroidColors[index];

}

Asteroid::Asteroid(Game* game, Vector2 spawnPos, Vector2 targetPos, int lane)
    :Actor(game)
    ,mRigidBodyComponent(nullptr)
    ,mDrawComponent(nullptr)
    ,mTargetPos(targetPos)
    ,mLane(lane) // <-- SALVE A LANE AQUI
{
    SetPosition(spawnPos);

    const float radius = 10.0f;
    const int numVertices = 12;

    std::vector<Vector2> vertices = GenerateVertices(numVertices, radius);
    mDrawComponent = new DrawPolygonComponent(this, vertices);

    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mCircleColliderComponent = new CircleColliderComponent(this, radius);

    // --- A SOLUÇÃO ESTÁ AQUI ---

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

// Em Asteroid.cpp, substitua o método Update por esta versão final.

void Asteroid::Update(float deltaTime)
{
    Actor::Update(deltaTime);

    if (!mRigidBodyComponent)
    {
        return;
    }

    // Não precisamos mais dos logs, você pode removê-los se quiser.
    // SDL_Log("[Asteroid %p] ...", ...);

    Vector2 currentPos = GetPosition();
    Vector2 velocity = mRigidBodyComponent->GetVelocity();

    // --- LÓGICA DE DETECÇÃO FINAL E ROBUSTA ---

    // Caso 1: O asteroide está se movendo para a DIREITA
    if (velocity.x > 0.0f)
    {
        // Usa >= para cobrir o caso de aterrissar exatamente no alvo.
        if (currentPos.x >= mTargetPos.x)
        {
            SetState(ActorState::Destroy);
        }
    }
    // Caso 2: O asteroide está se movendo para a ESQUERDA
    else if (velocity.x < 0.0f)
    {
        // Usa <= para cobrir o caso de aterrissar exatamente no alvo.
        if (currentPos.x <= mTargetPos.x)
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

float Asteroid::CalculateAverageVerticesLength(std::vector<Vector2>& vertices)
{
    float totalLength = 0.0f;
    for (const auto& vertex : vertices) {
        totalLength += vertex.Length();
    }

    return totalLength / vertices.size();
}

Vector2 Asteroid::GenerateRandomStartingForce(const float min, const float max)
{
    float randDirX = (Random::GetFloat() < 0.5f) ? 1.0f : -1.0f;
    float randDirY = (Random::GetFloat() < 0.5f) ? 1.0f : -1.0f;

    Vector2 randForce = Random::GetVector(Vector2(min, min), Vector2(max, max));
    randForce.x *= randDirX;
    randForce.y *= randDirY;

    return randForce;
}
#include "Target.h"
#include <cmath>
#include "../Game.h"

Target::Target(Game* game, const Vector2& pos, SDL_Color color, int mSize)
    : Actor(game),
      mColor(color),
      mSize(mSize),
      mIsActive(false),
      mRadius(static_cast<float>(mSize))
{
    SetPosition(pos);
    SetScale(1.0f);

    // Gera vértices de um círculo
    const int segments = 32;
    const float angleStep = 2.0f * static_cast<float>(M_PI) / segments;
    std::vector<Vector2> circleVerts;

    for (int i = 0; i < segments; ++i)
    {
        float angle = i * angleStep;
        float x = cosf(angle) * mRadius;
        float y = sinf(angle) * mRadius;
        circleVerts.emplace_back(x, y);
    }

    // Cria DrawPolygonComponent com os vértices gerados
    mDrawComponent = new DrawPolygonComponent(this, circleVerts, 100);
}

void Target::OnUpdate(float deltaTime)
{
    // Nada necessário por enquanto
}

void Target::Draw(SDL_Renderer* renderer) const
{
    // Deixe o componente desenhar — cor pode ser passada como modColor
    Vector3 modColor = mIsActive
        ? Vector3(1.0f, 1.0f, 1.0f) // branco quando pressionado
        : Vector3(mColor.r / 255.0f, mColor.g / 255.0f, mColor.b / 255.0f);

    mDrawComponent->Draw(renderer, modColor);
}

void Target::SetActive(bool active)
{
    mIsActive = active;
}

bool Target::IsActive() const
{
    return mIsActive;
}

//
// Created by Aline on 20/06/2025.
//

#pragma once
#include "Actor.h"
#include <SDL.h>
#include "../Components/DrawComponents/DrawPolygonComponent.h"

class Target : public Actor
{
public:
    Target(class Game* game, const Vector2& pos, SDL_Color color, int mSize = 35);

    // Desenha o círculo fixo na tela
    void Draw(SDL_Renderer* renderer) const;

    // Define se está ativo (pressionado pelo jogador)
    void SetActive(bool active);
    bool IsActive() const;

    // Propriedades de aparência
    void SetColor(SDL_Color color) { mColor = color; }
    SDL_Color GetColor() const { return mColor; }

    void SetRadius(float r) { mRadius = r; }
    float GetRadius() const { return mRadius; }

protected:
    virtual void OnUpdate(float deltaTime) override;

private:
    SDL_Color mColor;
    float mRadius;
    bool mIsActive;

    int mSize;
    DrawComponent* mDrawComponent;
};

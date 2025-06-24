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
    // Construtor principal
    Target(class Game* game, const Vector2& pos, SDL_Color color, int lane, int size = 50);

    // Atualiza o estado do flash a cada frame
    void OnUpdate(float deltaTime) override;

    // Aciona o efeito de flash
    void Flash();

    // Retorna a qual pista (lane) este alvo pertence
    int GetLane() const { return mLane; }

private:
    // Ponteiro para o componente que nos desenha (para podermos mudar a cor)
    class DrawPolygonComponent* mDrawComponent;

    // Propriedades do alvo
    SDL_Color mOriginalColor;
    int mLane;
    bool mIsFlashing;
    float mFlashTimer;
};
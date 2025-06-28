//
// Created by Aline on 28/06/2025.
//

#include "Ground.h"
#include "../Game.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

// Não precisamos mais das texturas aqui, pois será sem imagem.

Ground::Ground(Game* game, const Vector2& position)
    : Actor(game)
{
    SetPosition(position);

    // Cria um colisor do tamanho de um tile padrão do jogo.
    // O último parâmetro 'true' o marca como estático.
    mCollider = new AABBColliderComponent(this, 0, 0,
                                          Game::TILE_SIZE,
                                          Game::TILE_SIZE,
                                          ColliderLayer::Ground,
                                          true);
}
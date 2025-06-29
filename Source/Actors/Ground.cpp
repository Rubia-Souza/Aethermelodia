//
// Created by Aline on 28/06/2025.
//

#include "Ground.h"
#include "../Game.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

Ground::Ground(Game* game, const Vector2& position)
    : Actor(game)
{
    SetPosition(position);

    mCollider = new AABBColliderComponent(this, 0, 0,
                                          Game::TILE_SIZE,
                                          Game::TILE_SIZE,
                                          ColliderLayer::Ground,
                                          true);
}
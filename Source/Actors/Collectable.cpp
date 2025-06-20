//
// Created by rubia on 14/06/2025.
//

#include "Collectable.h"

#include "../Game.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

Collectable::Collectable(Game *game, const std::string &texturePath)
    : Actor(game) {
    new DrawSpriteComponent(this, texturePath, Game::TILE_SIZE, Game::TILE_SIZE);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE - 20, Game::TILE_SIZE - 20, ColliderLayer::Collectable, true);
}

Collectable::~Collectable() {}

void Collectable::Collect() {
    mColliderComponent->SetEnabled(false);
    mGame->GetAudio()->PlaySound("Coin.wav");
    mGame->addScore(10);
    mGame->IncrementAmountOfCoins();
    mState = ActorState::Destroy;
}

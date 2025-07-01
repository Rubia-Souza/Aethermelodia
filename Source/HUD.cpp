//
// Created by Lucas N. Ferreira on 08/12/23.
//

#include "HUD.h"
#include "Game.h"
#include "UIElements/UIText.h"

HUD::HUD(class Game* game, const std::string& fontName)
    :UIScreen(game, fontName)
{
    // 1) Pré-carrega as texturas UMA ÚNICA VEZ
    mHeartFullTex  = mGame->LoadTexture("../Assets/Sprites/UI/heart_full.png");   // <<<
    mHeartEmptyTex = mGame->LoadTexture("../Assets/Sprites/UI/heart_empty.png");  // <<<

    this->AddText("Score", Vector2(mGame->GetWindowWidth() - 200, 15), Vector2(100, 20), POINT_SIZE);
    mScoreText = this->AddText("000000", Vector2(mGame->GetWindowWidth() - 200, 40), Vector2(120, 20), POINT_SIZE);

    // --- HUD de corações (5 vidas) ---
    this->AddText("Vidas", Vector2(30, 15), Vector2(100, 20), POINT_SIZE);
    for (int i = 0; i < game->GetMaxLives(); ++i) {
        UIImage* h = this->AddImage(
            "../Assets/Sprites/UI/heart_full.png",
            Vector2(startX + i * spacing, startY),
            Vector2(heartSize, heartSize)
        );
        mHeartIcons.push_back(h);
    }
}

HUD::~HUD()
{

    // 3) Destrói as texturas carregadas no construtor
    if (mHeartFullTex)  SDL_DestroyTexture(mHeartFullTex);    // <<<
    if (mHeartEmptyTex) SDL_DestroyTexture(mHeartEmptyTex);   // <<<

}

void HUD::SetLives(int lives)
{
    // apenas troca a textura de cada UIImage já existente
    for (int i = 0; i < Game::GetMaxLives(); ++i) {

        SDL_Texture* tex =  i < lives ? mHeartFullTex : mHeartEmptyTex;

        mHeartIcons[i]->SetTexture(tex);
        // if (i>=lives) {
        //     SDL_Texture* tex = mHeartEmptyTex;
        //     mHeartIcons[i]->SetTexture(tex);  // <<< chama novo método
        // } else {
        //     SDL_Texture* tex = mHeartFullTex;
        //     mHeartIcons[i]->SetTexture(tex);  // <<< chama novo método
        // }

    }
}

void HUD::SetScore(int playerScore) {
    std::string score = std::to_string(playerScore);
    mScoreText->SetText(score);
}

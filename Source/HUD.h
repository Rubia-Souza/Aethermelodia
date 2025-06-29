//
// Created by Lucas N. Ferreira on 08/12/23.
//

#pragma once

#include <string>
#include <vector>
#include "UIElements/UIImage.h"
#include "UIElements/UIScreen.h"

class HUD : public UIScreen
{
public:
    const int POINT_SIZE = 48;
    const int WORD_HEIGHT = 20.0f;
    const int WORD_OFFSET = 25.0f;
    const int CHAR_WIDTH = 20.0f;
    const int HUD_POS_Y = 10.0f;

    HUD(class Game* game, const std::string& fontName);
    ~HUD();

    // Reinsert the HUD elements
    void SetTime(int time);

    void SetLevelName(const std::string& levelName);

    void SetScore(int playerScore);

    void SetAmountOfCoins(int amountOfCoins);

    void SetLives(int lives);

private:
    // HUD elements
    UIText* mScoreCounter;
    UIText* mLevelName;
    UIText* mLivesCount;
    UIText* mTimeText;
    UIText* mScoreText;
    UIText* mCoinsCounter;
    // ícones de coração
    int mLives;
    std::vector<UIImage*> mHeartIcons;
    SDL_Texture* mHeartFullTex;
    SDL_Texture* mHeartEmptyTex;
    const float startX     = 30.0f;          // x
    const float startY     = 40.0f;          // y
    const float heartSize  = 32.0f;          // tamanho do sprite
    const float spacing    = heartSize + 8;  // espaço entre corações
};

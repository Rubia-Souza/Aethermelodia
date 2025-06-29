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
    // --------------
    // TODO - PARTE 3
    // --------------

    // TODO 1.: Adicione um texto com a string "Time" no canto superior direito da tela, como no jogo orginal. Note que
    //  a classe HUD tem constantes WORD_HEIGHT, WORD_OFFSET, CHAR_WIDTH, POINT_SIZE e HUD_POS_Y que podem ser usadas
    //  para posicionar e definir o tamanho do texto.
    //this->AddText("Time", Vector2(mGame->GetWindowWidth() - 120, 15), Vector2(80, 20), POINT_SIZE);

    // TODO 2.: Adicione um texto com a string "400" (400 segundos) logo abaixo do texto "Time".
    //  Guarde o ponteiro do texto em um membro chamado mTimeText.
    mTimeText = this->AddText("400", Vector2(mGame->GetWindowWidth() - 120, 40), Vector2(80, 20), POINT_SIZE);


    // TODO 3.: Adicione um texto com a string "World" à esquerda do texto "Time", como no jogo original.
    this->AddText("World", Vector2((mGame->GetWindowWidth() / 2) + 50, 15), Vector2(80, 20), POINT_SIZE);


    // TODO 4.: Adicione um texto com a string "1-1" logo abaixo do texto "World".
    mLevelName = this->AddText("1-1", Vector2((mGame->GetWindowWidth() / 2) + 60, 40), Vector2(60, 20), POINT_SIZE);


    // TODO 5.: Adicione um texto com a string "Mario" no canto superior esquerdo da tela, como no jogo original.
    //this->AddText("Mario", Vector2(30, 15), Vector2(100, 20), POINT_SIZE);

    // TODO 6.: Adicione um texto com a string "000000" logo abaixo do texto "Mario".
    //mScoreText = this->AddText("000000", Vector2(30, 40), Vector2(120, 20), POINT_SIZE);

    //this->AddText("Coins", Vector2(250, 15), Vector2(80, 20), POINT_SIZE);
    //mCoinsCounter = this->AddText("00",  Vector2(250, 40), Vector2(60, 20), POINT_SIZE);

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
        if (i>=lives) {
            SDL_Texture* tex = mHeartEmptyTex;
            mHeartIcons[i]->SetTexture(tex);  // <<< chama novo método
        }

    }
}


void HUD::SetTime(int time)
{
    // --------------
    // TODO - PARTE 3
    // --------------

    // TODO 1.: Utilize o método SetText() do mTimeText para atualizar o texto com o tempo restante. Lembre-se que
    //  o tempo é um inteiro que representa os segundos restantes, e deve ser convertido para string.
    mTimeText->SetText(std::to_string(time));
    // TODO 2.: A posição e o tamanho do texto irão mudar dependendo do número de dígitos na variável time.
    //  Ajuste a posição e o tamanho do mTimeText de acordo com o número de dígitos, de tal forma que
    //  o texto fique alinhado à direita com o texto "Time" e o tamanho do texto seja proporcional ao número de dígitos.
    if (time < 10) {
        mTimeText->SetPosition(Vector2(mGame->GetWindowWidth() - 65, 40));
        mTimeText->SetSize(Vector2(20, 20));
    } else if (time < 100) {
        mTimeText->SetPosition(Vector2(mGame->GetWindowWidth() - 85, 40));
        mTimeText->SetSize(Vector2(40, 20));
    }
}

void HUD::SetLevelName(const std::string &levelName)
{
    // --------------
    // TODO - PARTE 3
    // --------------

    // TODO 1.: Utilize o método SetText() do mLevelName para atualizar o texto com o nome do nível.
    mLevelName->SetText(levelName);
}

void HUD::SetScore(int playerScore) {
    std::string score = std::to_string(playerScore);
    for (int i = score.length(); i < 6; i++) {
        score.insert(0, "0");
    }
    mScoreText->SetText(score);
}

void HUD::SetAmountOfCoins(int amountOfCoins) {
    std::string score = std::to_string(amountOfCoins);
    for (int i = score.length(); i < 2; i++) {
        score.insert(0, "0");
    }
    mCoinsCounter->SetText(score);
}


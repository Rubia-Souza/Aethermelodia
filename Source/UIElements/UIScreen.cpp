// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "UIScreen.h"
#include "../Game.h"
#include "UIFont.h"

UIScreen::UIScreen(Game* game, const std::string& fontName)
	:mGame(game)
	,mPos(0.f, 0.f)
	,mSize(0.f, 0.f)
	,mState(UIState::Active)
    ,mSelectedButtonIndex(-1)
{
    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Adicione a UIScreen à lista de telas de UI do jogo (mGame) com PushUI(this).
    mGame->PushUI(this);

    // TODO 2.: Utilize o método LoadFont do jogo (mGame) para carregar a fonte padrão,
    //  passada como parâmetro (fontName).
    // TODO [RUBIA] Olhar se é para armazenar no mFont mesmo
    mFont = mGame->LoadFont(fontName);
}

UIScreen::~UIScreen()
{
    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Percorra a listas de textos (mTexts) e delete cada UIText. Limpe a lista ao final do laço.
    for (UIText* text : mTexts) {
        delete text;
    }
    mTexts.clear();

    // --------------
    // TODO - PARTE 1-2
    // --------------

    // TODO 1.: Percorra a lista de botões (mButtons) e delete cada UIButton. Limpe a lista ao final do laço.
    for (UIButton* button : mButtons) {
        delete button;
    }
    mButtons.clear();

    // --------------
    // TODO - PARTE 1-3
    // --------------

    // TODO 1.: Percorra a lista de imagens (mImages) e delete cada UIImage. Limpe a lista ao final do laço.
    for (UIImage* image : mImages) {
        delete image;
    }
    mImages.clear();
}

void UIScreen::Update(float deltaTime)
{
	
}

void UIScreen::Draw(SDL_Renderer *renderer)
{
    // --------------
    // TODO - PARTE 1-3
    // --------------

    // TODO 1.: Percorra a lista de imagens (mImages) e chame o método Draw de cada UIImage, passando o renderer
    for (UIImage* image : mImages) {
        image->Draw(renderer, mPos);
    }

    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Percorra a listas de textos (mTexts) e chame o método Draw de cada UIText, passando o renderer
    //  e a posição da tela (mPos).
    for (UIText* text : mTexts) {
        text->Draw(renderer, mPos);
    }

    // --------------
    // TODO - PARTE 1-2
    // --------------

    // TODO 1.: Percorra a lista de botões (mButtons) e chame o método Draw de cada UIButton, passando o renderer
    for (UIButton* button : mButtons) {
        button->Draw(renderer, mPos);
    }
}

void UIScreen::ProcessInput(const uint8_t* keys)
{

}

void UIScreen::HandleKeyPress(int key)
{
    if (mButtons.empty()) return; // Safe guarde para evitar bug de memoria (w e s encerrando o jogo fora do menu

    if (key == SDLK_w) {
        UIButton* oldButton = mButtons[mSelectedButtonIndex];
        oldButton->SetHighlighted(false);

        mSelectedButtonIndex--;
        if (mSelectedButtonIndex < 0) {
            mSelectedButtonIndex = mButtons.size() - 1;
        }

        UIButton* button = mButtons[mSelectedButtonIndex];
        button->SetHighlighted(true);
    } else if (key == SDLK_s) {
        UIButton* oldButton = mButtons[mSelectedButtonIndex];
        oldButton->SetHighlighted(false);

        mSelectedButtonIndex++;
        if (mSelectedButtonIndex >= mButtons.size()) {
            mSelectedButtonIndex = 0;
        }

        UIButton* button = mButtons[mSelectedButtonIndex];
        button->SetHighlighted(true);
    } else if (key == SDLK_RETURN) {
        if (mSelectedButtonIndex >= 0 || mSelectedButtonIndex < mButtons.size()) {
            UIButton* button = mButtons[mSelectedButtonIndex];
            button->OnClick();
        }
    }
}

void UIScreen::Close()
{
	mState = UIState::Closing;
}

UIText* UIScreen::AddText(const std::string &name, const Vector2 &pos, const Vector2 &dims, const int pointSize, const int unsigned wrapLength)
{
    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Crie um novo UIText com o nome, fonte (mFont), tamanho do ponto (pointSize), comprimento de quebra (wrapLength),
    //  posição (pos), dimensões (dims) e cor branca. Armazene o ponteiro em uma variável t. A seguir, adicione o UIText
    //  à lista de textos (mTexts) e retorne o ponteiro t.
    UIText* text = new UIText(name, mFont, pointSize, wrapLength, pos, dims, Color::White);
    mTexts.push_back(text);
    return text;
}

UIButton* UIScreen::AddButton(const std::string& name, const Vector2 &pos, const Vector2& dims, std::function<void()> onClick, Vector2 textSize = Vector2(140, 20))
{
    // --------------
    // TODO - PARTE 1-2
    // --------------

    // TODO 1.: Crie um novo UIButton com o nome, fonte (mFont), função de clique (onClick),
    //  posição (pos), dimensões (dims) e cor laranja. Adicione o botão à lista de botões (mButtons).
    // TODO [Rubia] Testar cor diferente
    UIButton* button = new UIButton(name, mFont, onClick, pos, dims, Vector3(200, 100, 0), 40, 1024, Vector2::Zero, textSize);
    mButtons.push_back(button);

    // TODO 2.: Se a lista de botões (mButtons) tiver apenas um botão, defina o índice do botão
    //  selecionado (mSelectedButtonIndex) como 0 e destaque o botão (b->SetHighlighted(true)).
    if (mButtons.size() == 1) {
        mSelectedButtonIndex = 0;
        button->SetHighlighted(true);
    }

    // TODO 3.: Retorne o ponteiro do botão criado (b).
    return button;
}

UIImage* UIScreen::AddImage(const std::string &imagePath, const Vector2 &pos, const Vector2 &dims, const Vector3 &color)
{
    // --------------
    // TODO - PARTE 1-3
    // --------------

    // TODO 1.: Crie um novo UIImage com o caminho da imagem (imagePath), posição (pos), dimensões (dims) e cor (color).
    //  Armazene o ponteiro em uma variável img. A seguir, adicione o UIImage à lista de imagens (mImages) e retorne
    //  o ponteiro img.
    UIImage* img = new UIImage(mGame->GetRenderer(), imagePath, pos, dims, color);
    mImages.push_back(img);
    return img;
}
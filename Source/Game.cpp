// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "CSV.h"
#include "Random.h"
#include "Game.h"
#include "HUD.h"
#include "SpatialHashing.h"
#include "Actors/Actor.h"
#include "Actors/Mario.h"
#include "Actors/Block.h"
#include "Actors/Collectable.h"
#include "Actors/Spawner.h"
#include "Actors/Target.h"
#include "Actors/Asteroid.h"
#include "UIElements/UIScreen.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Components/ColliderComponents/AABBColliderComponent.h"
#include "GameTimer.h"
#include "FileReaderUtil.h"
#include "Actors/Lirael.h"
#include "Actors/Ground.h"

Game::Game(int windowWidth, int windowHeight)
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mWindowWidth(windowWidth)
        ,mWindowHeight(windowHeight)
        ,mMario(nullptr)
        ,mLirael(nullptr)
        ,mHUD(nullptr)
        ,mBackgroundColor(0, 0, 0)
        ,mModColor(255, 255, 255)
        ,mCameraPos(Vector2::Zero)
        ,mAudio(nullptr)
        ,mGameTimer(0.0f)
        ,mGameTimeLimit(0)
        ,mSceneManagerTimer(0.0f)
        ,mSceneManagerState(SceneManagerState::None)
        ,mGameScene(GameScene::MainMenu)
        ,mNextScene(GameScene::MainMenu)
        ,mBackgroundTexture(nullptr)
        ,mBackgroundSize(Vector2::Zero)
        ,mBackgroundPosition(Vector2::Zero)
        ,playerScore(0)
        ,mFadeState(FadeState::None)
        ,amountCoinsCollected(0)
        ,mMusicStartOffset(2.5f)
        ,mYPosTop(windowHeight * 0.70f)
        ,mYPosBottom(windowHeight * 0.85f)
        ,mXPosLeft(windowWidth * 0.42f)
        ,mXPosRight(windowWidth * 0.58f)
{

}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("TP4: Super Mario Bros", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWindowWidth, mWindowHeight, 0);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        SDL_Log("Unable to initialize SDL_image: %s", SDL_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0)
    {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        SDL_Log("Failed to initialize SDL_mixer");
        return false;
    }

    // Start random number generator
    Random::Init();

    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1. Instancie um AudioSystem.
    mAudio = new AudioSystem();


    mSpatialHashing = new SpatialHashing(TILE_SIZE * 4.0f,
                                         LEVEL_WIDTH * TILE_SIZE,
                                         LEVEL_HEIGHT * TILE_SIZE);
    mTicksCount = SDL_GetTicks();

    // Init all game actors
    SetGameScene(GameScene::MainMenu);

    return true;
}

void Game::SetGameScene(Game::GameScene scene, float transitionTime)
{
    // --------------
    // TODO - PARTE 2
    // --------------

    // TODO 1.: Verifique se o estado do SceneManager mSceneManagerState é SceneManagerState::None.
    //  Se sim, verifique se a cena passada scene passada como parâmetro é uma das cenas válidas (MainMenu, Level1, Level2).
    //  Se a cena for válida, defina mNextScene como essa nova cena, mSceneManagerState como SceneManagerState::Entering e
    //  mSceneManagerTimer como o tempo de transição passado como parâmetro.
    //  Se a cena for inválida, registre um erro no log e retorne.
    //  Se o estado do SceneManager não for SceneManagerState::None, registre um erro no log e retorne.
    if (mSceneManagerState == SceneManagerState::None) {
        if (scene == GameScene::MainMenu || scene == GameScene::Level1 || scene == GameScene::Level2) {
            mNextScene = scene;
            mSceneManagerState = SceneManagerState::Entering;
            mSceneManagerTimer = transitionTime;
        } else {
            SDL_Log("[ERROR] Invalid scene state");
        }
    } else {
        SDL_Log("[ERROR] Invalid Scene Manager State");
    }
}

void Game::ResetGameScene(float transitionTime)
{
    // --------------
    // TODO - PARTE 2
    // --------------

    // TODO 1.: Chame SetGameScene passando o mGameScene atual e o tempo de transição.
    SetGameScene(mGameScene, transitionTime);
}

void Game::ChangeScene()
{
    // Unload current Scene
    UnloadScene();

    // Reset camera position
    mCameraPos.Set(0.0f, 0.0f);

    // Reset game timer
    mGameTimer = 0.0f;

    // Reset gameplau state
    mGamePlayState = GamePlayState::Playing;

    // Reset scene manager state
    mSpatialHashing = new SpatialHashing(CELL_SIZE, mWindowWidth, PLAYABLE_AREA_HEIGHT);

    playerScore = 0;
    amountCoinsCollected = 0;

    // Scene Manager FSM: using if/else instead of switch
    if (mNextScene == GameScene::MainMenu)
    {
        // Set background color
        mBackgroundColor.Set(107.0f, 140.0f, 255.0f);

        // Initialize main menu actors
        LoadMainMenu();
    }
    else if (mNextScene == GameScene::Level1)
    {
        mAudio->StopAllSounds();
        mHUD = new HUD(this, "../Assets/Fonts/SMB.ttf");

        mGameTimeLimit = 400;
        mHUD->SetTime(mGameTimeLimit);
        mHUD->SetLevelName("1-1");

        // mMusicHandle = mAudio->PlaySound("medium-song.ogg", true);
        gameTimer.start();
        chart = FileReaderUtil::loadChartManually("../Assets/SoundsChart/easy-notes.chart");

        SetBackgroundImage("../Assets/Sprites/Background.png", Vector2(0,0), Vector2(mWindowWidth,mWindowHeight));
        for (int i = 0; i < (mWindowWidth / Game::TILE_SIZE + 1); i++) new Ground(this, Vector2(i * Game::TILE_SIZE,  PLAYABLE_AREA_HEIGHT - 10)); // Chao em blocos por conta do mSpatialHashing

        auto target0 = new Target(this, Vector2(mXPosLeft, mYPosBottom), SDL_Color{0, 255, 0, 255}, 0, 30);
        auto target1 = new Target(this, Vector2(mXPosRight, mYPosBottom), SDL_Color{255, 0, 0, 255}, 1, 30);
        auto target2 = new Target(this, Vector2(mXPosLeft, mYPosTop), SDL_Color{0, 0, 255, 255}, 2, 30);
        auto target3 = new Target(this, Vector2(mXPosRight, mYPosTop), SDL_Color{255, 255, 0, 255}, 3, 30);

        mTargets.emplace_back(target0);
        mTargets.emplace_back(target1);
        mTargets.emplace_back(target2);
        mTargets.emplace_back(target3);

        mLirael = new Lirael(this);
        mLirael->SetPosition(Vector2(mWindowWidth * 0.47, 0)); // 0.47 para começar entre os targets
    }
    else if (mNextScene == GameScene::Level2)
    {
        // --------------
        // TODO - PARTE 3
        // --------------

        // TODO 1.: Crie um novo objeto HUD, passando o ponteiro do Game e o caminho para a fonte SMB.ttf. Como
        //  feito no nível 1-1.
        mHUD = new HUD(this, "../Assets/Fonts/SMB.ttf");

        // TODO 2.: Altere o atributo mGameTimeLimit para 400 (400 segundos) e ajuste o HUD com esse tempo inicial. Como
        //  feito no nível 1-1.
        mGameTimeLimit = 400;
        mHUD->SetTime(mGameTimeLimit);
        mHUD->SetLevelName("1-2");

        // --------------
        // TODO - PARTE 4
        // --------------

        // TODO 1. Toque a música de fundo "MusicUnderground.ogg" em loop e armaze o SoundHandle retornado em mMusicHandle.
        mMusicHandle = mAudio->PlaySound("MusicUnderground.ogg", true);


        // Set background color
        mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

        // Set mod color
        mModColor.Set(0.0f, 255.0f, 200.0f);

        // Initialize actors
        LoadLevel("../Assets/Levels/level1-2.csv", LEVEL_WIDTH, LEVEL_HEIGHT);
    }

    // Set new scene
    mGameScene = mNextScene;
}


void Game::LoadMainMenu()
{
    auto mainMenu = new UIScreen(this, "../Assets/Fonts/SMB.ttf");
    // mAudio->PlaySound("Escape-of-Tower-Ending-Theme-1.ogg", true);

    auto menuBackground = mainMenu->AddImage("../Assets/Sprites/Menu_Background.jpg", Vector2::Zero, Vector2(mWindowWidth, mWindowHeight));
    auto title = mainMenu->AddText("Aethermelodia", Vector2((GetWindowWidth() - 352) / 2, (GetWindowHeight() - 176) / 2 - 100), Vector2(352, 176));

    auto button1 = mainMenu->AddButton("Play Game", Vector2(mWindowWidth/2.0f - 150.0f, 400.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::Level1); }, Vector2(180, 15));
    auto button2 = mainMenu->AddButton("How to Play", Vector2(mWindowWidth/2.0f - 150.0f, 465.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::Level2); }, Vector2(160, 15));
    auto button3 = mainMenu->AddButton("Credits", Vector2(mWindowWidth/2.0f - 150.0f, 535.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::Level2); }, Vector2(120, 15));
}

void Game::LoadLevel(const std::string& levelName, const int levelWidth, const int levelHeight)
{
    // Load level data
    int **mLevelData = ReadLevelData(levelName, levelWidth, levelHeight);

    if (!mLevelData) {
        SDL_Log("Failed to load level data");
        return;
    }

    // Instantiate level actors
    BuildLevel(mLevelData, levelWidth, levelHeight);
}

void Game::BuildLevel(int** levelData, int width, int height)
{

    // Const map to convert tile ID to block type
    const std::map<int, const std::string> tileMap = {
            {0, "../Assets/Sprites/Blocks/BlockA.png"},
            {1, "../Assets/Sprites/Blocks/BlockC.png"},
            {2, "../Assets/Sprites/Blocks/BlockF.png"},
            {4, "../Assets/Sprites/Blocks/BlockB.png"},
            {6, "../Assets/Sprites/Blocks/BlockI.png"},
            {8, "../Assets/Sprites/Blocks/BlockD.png"},
            {9, "../Assets/Sprites/Blocks/BlockH.png"},
            {12, "../Assets/Sprites/Blocks/BlockG.png"}
    };

    for (int y = 0; y < LEVEL_HEIGHT; ++y)
    {
        for (int x = 0; x < LEVEL_WIDTH; ++x)
        {
            int tile = levelData[y][x];

            if(tile == 16) // Mario
            {
                mMario = new Mario(this);
                mMario->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }
            else if(tile == 10) // Spawner
            {
                Spawner* spawner = new Spawner(this, SPAWN_DISTANCE);
                spawner->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }
            else if(tile == 3) {
                Collectable* collectable = new Collectable(this, "../Assets/Sprites/Collectables/Coin.png");
                collectable->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }
            else // Blocks
            {
                auto it = tileMap.find(tile);
                if (it != tileMap.end())
                {
                    // Create a block actor
                    Block* block = new Block(this, it->second);
                    block->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
                }
            }
        }
    }
}

int **Game::ReadLevelData(const std::string& fileName, int width, int height)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        SDL_Log("Failed to load paths: %s", fileName.c_str());
        return nullptr;
    }

    // Create a 2D array of size width and height to store the level data
    int** levelData = new int*[height];
    for (int i = 0; i < height; ++i)
    {
        levelData[i] = new int[width];
    }

    // Read the file line by line
    int row = 0;

    std::string line;
    while (!file.eof())
    {
        std::getline(file, line);
        if(!line.empty())
        {
            auto tiles = CSVHelper::Split(line);

            if (tiles.size() != width) {
                SDL_Log("Invalid level data");
                return nullptr;
            }

            for (int i = 0; i < width; ++i) {
                levelData[row][i] = tiles[i];
            }
        }

        ++row;
    }

    // Close the file
    file.close();

    return levelData;
}

void Game::addScore(int points) {
    playerScore += points;
    mHUD->SetScore(playerScore);
}

void Game::IncrementAmountOfCoins() {
    amountCoinsCollected++;
    mHUD->SetAmountOfCoins(amountCoinsCollected);
}

void Game::RunLoop()
{
    while (mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
            case SDL_KEYDOWN:
                // Handle key press for UI screens
                if (!mUIStack.empty()) {
                    mUIStack.back()->HandleKeyPress(event.key.keysym.sym);
                }

                HandleKeyPressActors(event.key.keysym.sym, event.key.repeat == 0);

                // Check if the Return key has been pressed to pause/unpause the game
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    TogglePause();
                }
                break;
        }
    }

    ProcessInputActors();
}

void Game::ProcessInputActors()
{
    if(mGamePlayState == GamePlayState::Playing)
    {
        // Get actors on camera
        std::vector<Actor*> actorsOnCamera =
                mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

        const Uint8* state = SDL_GetKeyboardState(nullptr);

        bool isLiraelOnCamera = false;
        for (auto actor: actorsOnCamera)
        {
            actor->ProcessInput(state);

            if (actor == mLirael) {
                isLiraelOnCamera = true;
            }
        }

        // If Lirael is not on camera, process input for him
        if (!isLiraelOnCamera && mLirael) {
            mLirael->ProcessInput(state);
        }
    }
}

void Game::HandleKeyPressActors(const int key, const bool isPressed)
{

    if(mGamePlayState == GamePlayState::Playing && isPressed)
    {
        switch(key)
        {
            // Pista Superior Esquerda
        case SDLK_a:
        case SDLK_LEFT: // Seta para esquerda
            HitLane(0);
            break;

            // Pista Superior Direita
        case SDLK_f:
        case SDLK_UP: // Seta para cima
            HitLane(1);
            break;

            // Pista Inferior Esquerda
        case SDLK_s:
        case SDLK_DOWN: // Seta para baixo
            HitLane(2);
            break;

            // Pista Inferior Direita
        case SDLK_d:
        case SDLK_RIGHT: // Seta para direita
            HitLane(3);
            break;
        }
    }

    // if(mGamePlayState == GamePlayState::Playing)
    // {
    //     // Get actors on camera
    //     std::vector<Actor*> actorsOnCamera =
    //             mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);
    //
    //     // Handle key press for actors
    //     bool isLiraelOnCamera = false;
    //     for (auto actor: actorsOnCamera) {
    //         actor->HandleKeyPress(key, isPressed);
    //
    //         if (actor == mLirael) {
    //             isLiraelOnCamera = true;
    //         }
    //     }
    //
    //     // If Mario is not on camera, handle key press for him
    //     if (!isLiraelOnCamera && mLirael)
    //     {
    //         mLirael->HandleKeyPress(key, isPressed);
    //     }
    // }

}

void Game::TogglePause()
{

    if (mGameScene != GameScene::MainMenu)
    {
        if (mGamePlayState == GamePlayState::Playing)
        {
            mGamePlayState = GamePlayState::Paused;

            // --------------
            // TODO - PARTE 4
            // --------------

            // TODO 1.: Pare a música de fundo atual usando PauseSound() e toque o som "Coin.wav" para indicar a pausa.
            mAudio->PauseSound(mMusicHandle);
            mAudio->PlaySound("Coin.wav", false);
        }
        else if (mGamePlayState == GamePlayState::Paused)
        {
            mGamePlayState = GamePlayState::Playing;

            // --------------
            // TODO - PARTE 4
            // --------------

            // TODO 1.: Retome a música de fundo atual usando ResumeSound() e toque o som "Coin.wav" para
            //  indicar a retomada do jogo.
            mAudio->ResumeSound(mMusicHandle);
            mAudio->PlaySound("Coin.wav", false);
        }
    }
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    if(mGamePlayState != GamePlayState::Paused && mGamePlayState != GamePlayState::GameOver)
    {
        // Reinsert all actors and pending actors
        UpdateActors(deltaTime);
    }

    // Reinsert audio system
    mAudio->Update(deltaTime);

    // Reinsert UI screens
    for (auto ui : mUIStack) {
        if (ui->GetState() == UIScreen::UIState::Active) {
            ui->Update(deltaTime);
        }
    }

    // Delete any UIElements that are closed
    auto iter = mUIStack.begin();
    while (iter != mUIStack.end()) {
        if ((*iter)->GetState() == UIScreen::UIState::Closing) {
            delete *iter;
            iter = mUIStack.erase(iter);
        } else {
            ++iter;
        }
    }

    // ---------------------
    // Game Specific Updates
    // ---------------------
    // UpdateCamera();

    // --------------
    // TODO - PARTE 2
    // --------------

    // TODO 1.: Chame UpdateSceneManager passando o deltaTime.
    UpdateSceneManager(deltaTime);

    // --------------
    // TODO - PARTE 3
    // --------------

    // TODO 1.: Verifique se a cena atual é diferente de GameScene::MainMenu e se o estado do jogo é
    //  GamePlayState::Playing. Se sim, chame UpdateLevelTime passando o deltaTime.
    if (GameScene::MainMenu != mGameScene && mGamePlayState == GamePlayState::Playing) {
        UpdateLevelTime(deltaTime);
    }

    if (mGameScene == GameScene::Level1 && !chart.empty())
    {
        double currentTime = gameTimer.getSeconds() - mMusicStartOffset;

        // SDL_Log("Chart timeInSeconds: %s", std::to_string(chart[currentNoteIndex].timeInSeconds).c_str());
        // SDL_Log("Chart lane: %s", std::to_string(chart[currentNoteIndex].lane).c_str());
        // SDL_Log("currentTime: %s ", std::to_string(currentTime).c_str());

        // Define as posições dos 4 alvos (baseado no seu código em ChangeScene)
        std::vector<Vector2> targets;
        targets.emplace_back(mXPosLeft, mYPosTop);     // Alvo 0 (Superior Esquerdo)
        targets.emplace_back(mXPosRight, mYPosTop);     // Alvo 1 (Superior Direito)
        targets.emplace_back(mXPosLeft, mYPosBottom);     // Alvo 2 (Inferior Esquerdo)
        targets.emplace_back(mXPosRight, mYPosBottom);     // Alvo 3 (Inferior Direito)

        // Este loop verifica quais notas do chart devem se tornar visíveis
        while (currentNoteIndex < chart.size() && chart[currentNoteIndex].timeInSeconds <= currentTime + NOTE_VISIBILITY_WINDOW)
        {
            const auto& note = chart[currentNoteIndex];
            int lane = note.lane;

            // Se a pista for maior que 3 (0, 1, 2, 3), ignora esta nota
            if (lane >= 4) {
                currentNoteIndex++;
                continue;
            }

            Vector2 spawnPos;
            Vector2 targetPos = targets[lane];

            // Define a posição de spawn baseada na pista (lane)
            // Pistas 0 e 2 (esquerda) vêm da borda esquerda
            // Pistas 1 e 3 (direita) vêm da borda direita
            if (lane == 0 || lane == 2) {
                spawnPos.x = -50.0f; // Fora da tela, à esquerda
                spawnPos.y = targetPos.y;
            } else { // lane == 1 || lane == 3
                spawnPos.x = mWindowWidth + 50.0f; // Fora da tela, à direita
                spawnPos.y = targetPos.y;
            }

            new Asteroid(this, spawnPos, targetPos, lane);

            // Avança para a próxima nota no chart
            currentNoteIndex++;
        }
    }

}

void Game::HitLane(int lane)
{
    const float HIT_WINDOW_RADIUS = 75.0f;
    Target* hitTarget = nullptr;

    for (auto target : mTargets) {
        if (target->GetLane() == lane) {
            hitTarget = target;
            break;
        }
    }

    if (!hitTarget) return;

    hitTarget->Flash();

    Asteroid* hittableNote = nullptr;
    float minDistance = 10000.0f;
    Vector2 targetPos = hitTarget->GetPosition();

    for (auto ast : mAsteroids) {
        if (ast->GetLane() == lane) {
            float dist = (ast->GetPosition() - targetPos).Length();
            if (dist < minDistance) {
                minDistance = dist;
                hittableNote = ast;
            }
        }
    }

    if (hittableNote && minDistance <= HIT_WINDOW_RADIUS) {
        SDL_Log("HIT! Na pista %d", lane);
        hittableNote->SetState(ActorState::Destroy);
        addScore(100);
        // mAudio->PlaySound("hit.wav");
    } else {
        SDL_Log("MISS! Na pista %d", lane);
    }
}

void Game::UpdateSceneManager(float deltaTime)
{
    if (mFadeState == FadeState::FadeOut)
    {
        mFadeTime += deltaTime;
        if (mFadeTime >= TRANSITION_TIME)
        {
            mFadeTime = 0.0f;
            mFadeState = FadeState::FadeIn;
        }
    }
    else if(mFadeState == FadeState::FadeIn)
    {
        mFadeTime += deltaTime;
        if (mFadeTime >= TRANSITION_TIME) {
            mFadeTime = 0.0f;
            mFadeState = FadeState::None;
        }
    }

    if (mSceneManagerState == SceneManagerState::Entering) {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0) {
            mSceneManagerTimer = TRANSITION_TIME;
            mSceneManagerState = SceneManagerState::Active;
            mFadeState = FadeState::FadeOut;
        }
    }

    if (mSceneManagerState == SceneManagerState::Active) {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0) {
            ChangeScene();
            mSceneManagerState = SceneManagerState::None;
        }
    }
}

void Game::UpdateLevelTime(float deltaTime)
{
    mGameTimer += deltaTime;
    if (mGameTimer >= 1.0) {
        mGameTimer = 0.0f;
        mGameTimeLimit -= 1;
        mHUD->SetTime(mGameTimeLimit);

        if (mGameTimeLimit <= 0) {
            mLirael->Kill();
        }
    }
}

void Game::UpdateCamera()
{
    if (!mLirael) return;

    float horizontalCameraPos = mLirael->GetPosition().x - (mWindowWidth / 2.0f);

    if (horizontalCameraPos > mCameraPos.x)
    {
        // Limit camera to the right side of the level
        float maxCameraPos = (LEVEL_WIDTH * TILE_SIZE) - mWindowWidth;
        horizontalCameraPos = Math::Clamp(horizontalCameraPos, 0.0f, maxCameraPos);

        mCameraPos.x = horizontalCameraPos;
    }
}

void Game::UpdateActors(float deltaTime)
{

    for (size_t i = 0; i < mAsteroids.size(); ++i)
    {
        mAsteroids[i]->Update(deltaTime);
    }

    // Get actors on camera
    std::vector<Actor*> actorsOnCamera =
            mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

    bool isMarioOnCamera = false;
    for (auto actor : actorsOnCamera)
    {
        actor->Update(deltaTime);
        if (actor == mLirael)
        {
            isMarioOnCamera = true;
        }
    }

    // If Mario is not on camera, reset camera position
    if (!isMarioOnCamera && mLirael)
    {
        mLirael->Update(deltaTime);
    }

    for (auto actor : actorsOnCamera)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            delete actor;
            if (actor == mLirael) {
                mLirael = nullptr;
            }
        }
    }
}

void Game::AddActor(Actor* actor)
{
    mSpatialHashing->Insert(actor);
}

void Game::RemoveActor(Actor* actor)
{
    mSpatialHashing->Remove(actor);
}
void Game::Reinsert(Actor* actor)
{
    mSpatialHashing->Reinsert(actor);
}

void Game::AddAsteroid(Asteroid* ast)
{
    mAsteroids.emplace_back(ast);
}

void Game::RemoveAsteroid(Asteroid* ast)
{
    if (const auto iter = std::find(mAsteroids.begin(), mAsteroids.end(), ast); iter != mAsteroids.end()) {
        mAsteroids.erase(iter);
    }
}

std::vector<Actor *> Game::GetNearbyActors(const Vector2& position, const int range)
{
    return mSpatialHashing->Query(position, range);
}

std::vector<AABBColliderComponent *> Game::GetNearbyColliders(const Vector2& position, const int range)
{
    return mSpatialHashing->QueryColliders(position, range);
}

void Game::GenerateOutput()
{
    // Clear frame with background color
    SDL_SetRenderDrawColor(mRenderer, mBackgroundColor.x, mBackgroundColor.y, mBackgroundColor.z, 255);

    // Clear back buffer
    SDL_RenderClear(mRenderer);

    // Draw background texture considering camera position
    if (mBackgroundTexture)
    {
        SDL_Rect dstRect = { static_cast<int>(mBackgroundPosition.x - mCameraPos.x),
                             static_cast<int>(mBackgroundPosition.y - mCameraPos.y),
                             static_cast<int>(mBackgroundSize.x),
                             static_cast<int>(mBackgroundSize.y) };

        SDL_RenderCopy(mRenderer, mBackgroundTexture, nullptr, &dstRect);
    }

    // Get actors on camera
    std::vector<Actor*> actorsOnCamera =
            mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

    // Get list of drawables in draw order
    std::vector<DrawComponent*> drawables;

    for (auto actor : actorsOnCamera)
    {
        auto drawable = actor->GetComponent<DrawComponent>();
        if (drawable && drawable->IsVisible())
        {
            drawables.emplace_back(drawable);
        }
    }

    // Sort drawables by draw order
    std::sort(drawables.begin(), drawables.end(),
              [](const DrawComponent* a, const DrawComponent* b) {
                  return a->GetDrawOrder() < b->GetDrawOrder();
              });

    // Draw all drawables
    for (auto drawable : drawables)
    {
        drawable->Draw(mRenderer, mModColor);
    }

    // Draw all UI screens
    for (auto ui :mUIStack)
    {
        ui->Draw(mRenderer);
    }

    // --------------
    // TODO - PARTE 2
    // --------------

    if (mFadeState == FadeState::FadeOut)
    {
        float alphaOut = mFadeTime/TRANSITION_TIME;
        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255 * alphaOut);
        SDL_RenderFillRect(mRenderer, nullptr);
    }
    else if (mFadeState == FadeState::FadeIn)
    {
        float alphaIn = mFadeTime/TRANSITION_TIME;
        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255 * (1.0f - alphaIn));
        SDL_RenderFillRect(mRenderer, nullptr);
    }
    // TODO 1.: Verifique se o SceneManager está no estado ativo. Se estiver, desenhe um retângulo preto cobrindo
    //  toda a tela.
    // if (mSceneManagerState == SceneManagerState::Active) {
    //     SDL_Rect background = {
    //         0,
    //         0,
    //         mWindowWidth,
    //         mWindowHeight
    //     };
    //     SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
    //     SDL_RenderFillRect(mRenderer, &background);
    // }


    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);
}

void Game::SetBackgroundImage(const std::string& texturePath, const Vector2 &position, const Vector2 &size)
{
    if (mBackgroundTexture) {
        SDL_DestroyTexture(mBackgroundTexture);
        mBackgroundTexture = nullptr;
    }

    // Load background texture
    mBackgroundTexture = LoadTexture(texturePath);
    if (!mBackgroundTexture) {
        SDL_Log("Failed to load background texture: %s", texturePath.c_str());
    }

    // Set background position
    mBackgroundPosition.Set(position.x, position.y);

    // Set background size
    mBackgroundSize.Set(size.x, size.y);
}

SDL_Texture* Game::LoadTexture(const std::string& texturePath)
{
    SDL_Surface* surface = IMG_Load(texturePath.c_str());

    if (!surface) {
        SDL_Log("Failed to load image: %s", IMG_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return nullptr;
    }

    return texture;
}


UIFont* Game::LoadFont(const std::string& fileName)
{
    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Verifique se o arquivo de fonte já está carregado no mapa mFonts.
    //  Se sim, retorne o ponteiro para a fonte carregada.
    auto it = mFonts.find(fileName);
    if (it != mFonts.end()) {
        return it->second;
    }
    //  Se não, crie um novo objeto UIFont, carregue a fonte do arquivo usando o método Load,
    //  e se o carregamento for bem-sucedido, adicione a fonte ao mapa mFonts.
    //  Se o carregamento falhar, descarregue a fonte com Unload e delete o objeto UIFont, retornando nullptr.
    UIFont* newFont = new UIFont(mRenderer);
    bool wasSuccessful = newFont->Load(fileName);
    if (!wasSuccessful) {
        SDL_Log("[ERROR] Failed to load font: %s", fileName.c_str());
        newFont->Unload();
        delete newFont;
        return nullptr;
    }

    mFonts.insert(std::make_pair(fileName, newFont));
    return newFont;
}

void Game::UnloadScene()
{
    // Delete actors
    delete mSpatialHashing;

    // Delete UI screens
    for (auto ui : mUIStack) {
        delete ui;
    }
    mUIStack.clear();
    mTargets.clear();
    // Delete background texture
    if (mBackgroundTexture) {
        SDL_DestroyTexture(mBackgroundTexture);
        mBackgroundTexture = nullptr;
    }
}

void Game::Shutdown()
{
    UnloadScene();

    for (auto font : mFonts) {
        font.second->Unload();
        delete font.second;
    }
    mFonts.clear();

    delete mAudio;
    mAudio = nullptr;

    Mix_CloseAudio();

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

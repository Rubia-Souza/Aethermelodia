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
#include "Random.h"
#include "Game.h"
#include "HUD.h"
#include "SpatialHashing.h"
#include "Actors/Actor.h"
#include "Actors/Target.h"
#include "UIElements/UIScreen.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Components/ColliderComponents/AABBColliderComponent.h"
#include "GameTimer.h"
#include "FileReaderUtil.h"
#include "Actors/Enemy.h"
#include "Actors/Lirael.h"
#include "Actors/Ground.h"

Game::Game(int windowWidth, int windowHeight)
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mWindowWidth(windowWidth)
        ,mWindowHeight(windowHeight)
        ,mLirael(nullptr)
        ,mHUD(nullptr)
        ,mBackgroundColor(0, 0, 0)
        ,mModColor(255, 255, 255)
        ,mCameraPos(Vector2::Zero)
        ,mAudio(nullptr)
        ,mSceneManagerTimer(0.0f)
        ,mSceneManagerState(SceneManagerState::None)
        ,mGameScene(GameScene::MainMenu)
        ,mNextScene(GameScene::MainMenu)
        ,mBackgroundTexture(nullptr)
        ,mBackgroundSize(Vector2::Zero)
        ,mBackgroundPosition(Vector2::Zero)
        ,playerScore(0)
        ,mFadeState(FadeState::None)
        ,mCurrentLives(kMaxLives)
        ,mMusicStartOffset(2.0f)
        ,mYPosTop(windowHeight * 0.70f)
        ,mYPosBottom(windowHeight * 0.85f)
        ,mXPosLeft(windowWidth * 0.42f)
        ,mXPosRight(windowWidth * 0.58f)
        ,mGameDifficulty(Difficulty::EASY_SINGLE)
{

}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("Aethermelodia", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWindowWidth, mWindowHeight, 0);
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
    if (mSceneManagerState == SceneManagerState::None) {
        if (scene == GameScene::MainMenu || scene == GameScene::Level1 || scene == GameScene::Level2 || scene == GameScene::Level3 || scene == GameScene::Credits || scene == GameScene::HowToPlay || scene == GameScene::ToBeContinue || scene == GameScene::GameOver || scene == GameScene::DifficultySelection) {
            mPreviousScene = mGameScene;
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
    SetGameScene(mGameScene, transitionTime);
}

void Game::GameOver(float transitionTime)
{
    SetGameScene(GameScene::GameOver, transitionTime);
}

void Game::ChangeScene()
{
    // Unload current Scene
    UnloadScene();

    // Reset camera position
    mCameraPos.Set(0.0f, 0.0f);

    // Reset scene manager state
    mSpatialHashing = new SpatialHashing(CELL_SIZE, mWindowWidth, PLAYABLE_AREA_HEIGHT);

    playerScore = 0;

    // Scene Manager FSM: using if/else instead of switch
    if (mNextScene == GameScene::MainMenu)
    {
        mAudio->StopAllSounds();
        // Set background color
        mBackgroundColor.Set(107.0f, 140.0f, 255.0f);

        // Initialize main menu actors
        LoadMainMenu();
    } else if (mNextScene == GameScene::Level1) {
        LoadGameLevel("level-1.ogg", "../Assets/Levels/Level-1/level-1.chart");
    } else if (mNextScene == GameScene::Level2) {
        LoadGameLevel("level-2.ogg", "../Assets/Levels/Level-2/level-2.chart");
    } else if (mNextScene == GameScene::Level3) {
        LoadGameLevel("level-3.ogg", "../Assets/Levels/Level-3/level-3.chart");
    } else if (mNextScene == GameScene::ToBeContinue) {
        LoadToBeContinueScreen();
    } else if (mNextScene == GameScene::HowToPlay) {
        LoadHowToPlay();
    } else if (mNextScene == GameScene::Credits) {
        LoadCredits();
    } else if (mNextScene == GameScene::GameOver) {
        LoadGameOverScreen();
    } else if (mNextScene == GameScene::DifficultySelection) {
        LoadDifficultySelectionScreen();
    }

    // Set new scene
    mGameScene = mNextScene;
}

void Game::LoadGameLevel(std::string levelSong, std::string levelChart) {
    mCurrentLives = kMaxLives;
    currentNoteIndex = 0;
    mGamePlayState = GamePlayState::Playing;

    mAudio->StopAllSounds();
    mHUD = new HUD(this, "../Assets/Fonts/SMB.ttf");

    mMusicHandle = mAudio->PlaySound(levelSong, true);
    gameTimer.start();
    chart = FileReaderUtil::loadChartManually(levelChart, mGameDifficulty);

    SetBackgroundImage("../Assets/Sprites/Background.png", Vector2(0,0), Vector2(mWindowWidth,mWindowHeight));
    for (int i = 0; i < (mWindowWidth / Game::TILE_SIZE + 1); i++) new Ground(this, Vector2(i * Game::TILE_SIZE,  PLAYABLE_AREA_HEIGHT - 10)); // Chao em blocos por conta do mSpatialHashing

    auto target0 = new Target(this, Vector2(mXPosLeft, mYPosTop), Target::GetLaneColor(0), 0, 30);
    auto target1 = new Target(this, Vector2(mXPosRight, mYPosTop), Target::GetLaneColor(1), 1, 30);
    auto target2 = new Target(this, Vector2(mXPosLeft, mYPosBottom), Target::GetLaneColor(2), 2, 30);
    auto target3 = new Target(this, Vector2(mXPosRight, mYPosBottom), Target::GetLaneColor(3), 3, 30);

    mTargets.emplace_back(target0);
    mTargets.emplace_back(target1);
    mTargets.emplace_back(target2);
    mTargets.emplace_back(target3);

    mLirael = new Lirael(this);
    const float liraelWidth = 64.0f;
    const float centeredLiraelX = (mWindowWidth * 0.5f) - (liraelWidth / 2.0f);
    mLirael->SetPosition(Vector2(centeredLiraelX, 0));
}

void Game::LoadMainMenu()
{
    auto mainMenu = new UIScreen(this, "../Assets/Fonts/SMB.ttf");
    SoundState state = mAudio->GetSoundState(mMainMenuSoundHandle);
    if (state == SoundState::Stopped || state == SoundState::Paused) {
        mMainMenuSoundHandle = mAudio->PlaySound("Escape-of-Tower-Ending-Theme-1.ogg", true);
    }

    auto menuBackground = mainMenu->AddImage("../Assets/Sprites/Menu_Background.jpg", Vector2::Zero, Vector2(mWindowWidth, mWindowHeight));
    auto title = mainMenu->AddText("Aethermelodia", Vector2((GetWindowWidth() - 700) / 2, (GetWindowHeight() - 176) / 2 - 100), Vector2(700, 176));

    mainMenu->AddButton("Play Game", Vector2(mWindowWidth/2.0f - 150.0f, 400.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::Level1); }, Vector2(140, 30));
    mainMenu->AddButton("How to Play", Vector2(mWindowWidth/2.0f - 150.0f, 465.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::HowToPlay); }, Vector2(140, 30));
    mainMenu->AddButton("Difficulty", Vector2(mWindowWidth / 2.0f - 150.0f, 535.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::DifficultySelection); }, Vector2(140, 30));
    mainMenu->AddButton("Credits", Vector2(mWindowWidth/2.0f - 150.0f, 605.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::Credits); }, Vector2(130, 30));
}

void Game::LoadCredits() {
    auto credits = new UIScreen(this, "../Assets/Fonts/SMB.ttf");

    credits->AddImage("../Assets/Sprites/Menu_Background.jpg", Vector2::Zero, Vector2(mWindowWidth, mWindowHeight));
    credits->AddText("Aethermelodia", Vector2((GetWindowWidth() - 700) / 2, (GetWindowHeight() - 176) / 2 - 250), Vector2(700, 176));

    credits->AddText("Arts", Vector2(100.0f, 200.0f), Vector2(200.0f, 35.0f));
    credits->AddText("- Background Menu -", Vector2(25.0f, 270.0f), Vector2(350.0f, 20.0f));
    credits->AddText("Steven Greenblatt", Vector2(95.0f, 320.0f), Vector2(200.0f, 20.0f));

    credits->AddText("- Background Level 1 -", Vector2(25.0f, 370.0f), Vector2(350.0f, 20.0f));
    credits->AddText("edermunizz", Vector2(95.0f, 420.0f), Vector2(200.0f, 20.0f));

    credits->AddText("- Assets -", Vector2(95.0f, 480.0f), Vector2(200.0f, 20.0f));
    credits->AddText("Itch.io", Vector2(95.0f, 520.0f), Vector2(200.0f, 20.0f));

    credits->AddText("Music", Vector2(GetWindowWidth() / 2 - 100, 200.0f), Vector2(200.0f, 35.0f));
    credits->AddText("- Menu -", Vector2(GetWindowWidth() / 2 - 60, 270.0f), Vector2(120.0f, 20.0f));
    credits->AddText("The King of Fighters EX Neo Blood - Escape of Tower Ending Theme 1", Vector2(GetWindowWidth() / 2 - 225, 300.0f), Vector2(450.0f, 70.0f));
    credits->AddText("- Levels -", Vector2(GetWindowWidth() / 2 - 75, 400.0f), Vector2(150.0f, 20.0f));
    credits->AddText("Ichika Nito", Vector2(GetWindowWidth() / 2 - 100, 430.0f), Vector2(200.0f, 20.0f));
    credits->AddText("- Ending -", Vector2(GetWindowWidth() / 2 - 75, 500.0f), Vector2(150.0f, 20.0f));
    credits->AddText("Night in the Woods - Ending", Vector2(GetWindowWidth() / 2 - 200, 530.0f), Vector2(400.0f, 40.0f));

    credits->AddText("Code", Vector2(GetWindowWidth() - 300, 200.0f), Vector2(200.0f, 35.0f));
    credits->AddText("Aline Cristina", Vector2(mWindowWidth - 300.0f, 270.0f), Vector2(200.0f, 20.0f));
    credits->AddText("Gabriel Henrique", Vector2(mWindowWidth - 300.0f, 320.0f), Vector2(200.0f, 20.0f));
    credits->AddText("Rubia Alice", Vector2(mWindowWidth - 300.0f, 370.0f), Vector2(150.0f, 20.0f));
    credits->AddText("Vinicius Gabriel", Vector2(mWindowWidth - 300.0f, 420.0f), Vector2(200.0f, 20.0f));
    credits->AddButton("Back", Vector2(mWindowWidth/2.0f - 150.0f, 600.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::MainMenu); }, Vector2(100, 30));
}

void Game::LoadToBeContinueScreen() {
    mAudio->StopAllSounds();
    mMusicHandle = mAudio->PlaySound("Night in the Woods - Ending.mp3", false);
    mBackgroundColor.Set(121.0f, 142.0f, 173.0f);

    auto toBeContinue = new UIScreen(this, "../Assets/Fonts/SMB.ttf");
    toBeContinue->AddText("To be continue...", Vector2((GetWindowWidth() - 1200) / 2, (GetWindowHeight() - 150) / 2), Vector2(1200, 150));
    toBeContinue->AddText("Thanks for playing", Vector2((GetWindowWidth() - 200) / 2, (GetWindowHeight() - 20) / 2 + 100), Vector2(200, 20));
    toBeContinue->AddButton("Main Menu", Vector2(mWindowWidth/2.0f - 150.0f, 600.0f), Vector2(300.0f, 50.0f), [this]() {
        mAudio->StopAllSounds();
        SetGameScene(GameScene::MainMenu);
    }, Vector2(100, 30));
}

void Game::LoadHowToPlay()
{
    auto howToPlay = new UIScreen(this, "../Assets/Fonts/SMB.ttf");

    auto menuBackground = howToPlay->AddImage("../Assets/Sprites/Menu_Background.jpg", Vector2::Zero, Vector2(mWindowWidth, mWindowHeight));
    auto title = howToPlay->AddText("Aethermelodia", Vector2((GetWindowWidth() - 700) / 2, (GetWindowHeight() - 176) / 2 - 250), Vector2(700, 176));

    howToPlay->AddText("- Lore -", Vector2(175.0f, 200.0f), Vector2(200.0f, 35.0f));
    howToPlay->AddText("In this game you play as Lirael, a bard that is fighting for survival after losing his friends on a journey. Now he needs to use his songs to escape the dangers of the wilderness and find his friends again.", Vector2(40.0f, 250.0f), Vector2(600.0f, 300.0f));

    howToPlay->AddText("- Controls -", Vector2(mWindowWidth - 400.0f, 200.0f), Vector2(200.0f, 35.0f));
    auto text1 = howToPlay->AddText("W or E: Hit green note", Vector2(mWindowWidth - 500.0f, 280.0f), Vector2(400.0f, 30.0f));
    auto text2 = howToPlay->AddText("S or D: Hit blue note", Vector2(mWindowWidth - 500.0f, 335.0f), Vector2(400.0f, 30.0f));
    auto text3 = howToPlay->AddText("I or O: Hit red note", Vector2(mWindowWidth - 500.0f, 390.0f), Vector2(400.0f, 30.0f));
    auto text4 = howToPlay->AddText("J or K: Hit yellow note", Vector2(mWindowWidth - 500.0f, 445.0f), Vector2(400.0f, 30.0f));
    auto returnButton = howToPlay->AddButton("Back", Vector2(mWindowWidth/2.0f - 150.0f, 600.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::MainMenu); }, Vector2(100, 30));
}

void Game::LoadGameOverScreen() {
    mAudio->StopAllSounds();
    mMusicHandle = mAudio->PlaySound("Night in the Woods - Ending.mp3", false);
    auto gameOver = new UIScreen(this, "../Assets/Fonts/SMB.ttf");
    mBackgroundColor.Set(121.0f, 142.0f, 173.0f);

    const Vector2 titleSize = Vector2(200.0f, 200.0f) * 2.0f;
    const Vector2 titlePos = Vector2(mWindowWidth * 0.36, 60.0f);
    gameOver->AddImage("../Assets/Sprites/Game_Over.png", titlePos, titleSize);

    gameOver->AddText("Play again?", Vector2(mWindowWidth * 0.44, 450.0f), Vector2(200.0f, 35.0f));

    gameOver->AddButton("Yes", Vector2(mWindowWidth * 0.42, 510.0f), Vector2(250.0f, 50.0f), [this]() { SetGameScene(mPreviousScene); }, Vector2(60, 18));
    gameOver->AddButton("No", Vector2(mWindowWidth * 0.42, 580.0f), Vector2(250.0f, 50.0f), [this]() { SetGameScene(GameScene::MainMenu); }, Vector2(50, 18));
}

void Game::LoadDifficultySelectionScreen()
{
    auto difficultySelection = new UIScreen(this, "../Assets/Fonts/SMB.ttf");

    difficultySelection->AddImage("../Assets/Sprites/Menu_Background.jpg", Vector2::Zero, Vector2(mWindowWidth, mWindowHeight));
    difficultySelection->AddText("Aethermelodia", Vector2((GetWindowWidth() - 700) / 2, (GetWindowHeight() - 176) / 2 - 250), Vector2(700, 176));

    difficultySelection->AddText("Actual difficulty: ", Vector2(GetWindowWidth() / 2 - 100 - 100, 300.0f), Vector2(250.0f, 30.0f));
    auto actualDifficultyText = difficultySelection->AddText(FileReaderUtil::getDifficultyName(mGameDifficulty), Vector2(GetWindowWidth() / 2 - 100 + 150, 300.0f), Vector2(100.0f, 30.0f));

    difficultySelection->AddButton("Easy", Vector2(mWindowWidth/2.0f - 150.0f, 350.0f), Vector2(300.0f, 50.0f), [this, actualDifficultyText]() {
        mGameDifficulty = Difficulty::EASY_SINGLE;
        actualDifficultyText->SetText(FileReaderUtil::getDifficultyName(mGameDifficulty));
    }, Vector2(100, 30));

    difficultySelection->AddButton("Medium", Vector2(mWindowWidth/2.0f - 150.0f, 415.0f), Vector2(300.0f, 50.0f), [this, actualDifficultyText]() {
        mGameDifficulty = Difficulty::MEDIUM_SINGLE;
        actualDifficultyText->SetText(FileReaderUtil::getDifficultyName(mGameDifficulty));
    }, Vector2(100, 30));

    difficultySelection->AddButton("Hard", Vector2(mWindowWidth/2.0f - 150.0f, 485.0f), Vector2(300.0f, 50.0f), [this, actualDifficultyText]() {
        mGameDifficulty = Difficulty::HARD_SINGLE;
        actualDifficultyText->SetText(FileReaderUtil::getDifficultyName(mGameDifficulty));
    }, Vector2(100, 30));

    difficultySelection->AddButton("Back", Vector2(mWindowWidth/2.0f - 150.0f, 600.0f), Vector2(300.0f, 50.0f), [this]() { SetGameScene(GameScene::MainMenu); }, Vector2(100, 30));
}

void Game::addScore(int points) {
    playerScore += points;
    mHUD->SetScore(playerScore);
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
                HandleKeyDownActors(event.key.keysym.sym,  true); // event.key.repeat == 0

                // Check if the Return key has been pressed to pause/unpause the game
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    TogglePause();
                }
                break;
            case SDL_KEYUP:
                HandleKeyUpActors(event.key.keysym.sym, false);

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

void Game::HandleKeyDownActors(const int key, const bool isPressed)
{

    if(mGamePlayState == GamePlayState::Playing && isPressed)
    {
        switch(key)
        {
            // Pista Superior Esquerda - verde
            case SDLK_w:
            case SDLK_e:
                HitLane(0);
                break;

            // Pista Superior Direita - vermelho
            case SDLK_i:
            case SDLK_o:
                HitLane(1);
                break;

            // Pista Inferior Esquerda - azul
            case SDLK_s:
            case SDLK_d:
                HitLane(2);
                break;

            // Pista Inferior Direita - amarelo
            case SDLK_j:
            case SDLK_k:
                HitLane(3);
                break;
        }
    }

    if(mGamePlayState == GamePlayState::Playing)
    {
        // Get actors on camera
        std::vector<Actor*> actorsOnCamera =
                mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

        // Handle key press for actors
        bool isLiraelOnCamera = false;
        for (auto actor: actorsOnCamera) {
            actor->HandleKeyPress(key, isPressed);

            if (actor == mLirael) {
                isLiraelOnCamera = true;
            }
        }

        // If Mario is not on camera, handle key press for him
        if (!isLiraelOnCamera && mLirael)
        {
            mLirael->HandleKeyPress(key, isPressed);
        }
    }
}

void Game::HandleKeyUpActors(const int key, const bool isPressed)
{

    if(mGamePlayState == GamePlayState::Playing )
    {
        switch(key)
        {
            // Pista Superior Esquerda - verde
            case SDLK_w:
            case SDLK_e:
                UnhitLane(0);
                break;

            // Pista Superior Direita - vermelho
            case SDLK_i:
            case SDLK_o:
                UnhitLane(1);
                break;

            // Pista Inferior Esquerda - azul
            case SDLK_s:
            case SDLK_d:
                UnhitLane(2);
                break;

            // Pista Inferior Direita - amarelo
            case SDLK_j:
            case SDLK_k:
                UnhitLane(3);
                break;
        }
    }

    if(mGamePlayState == GamePlayState::Playing)
    {
        std::vector<Actor*> actorsOnCamera =
                mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

        bool isLiraelOnCamera = false;
        for (auto actor: actorsOnCamera) {
            actor->HandleKeyPress(key, isPressed);

            if (actor == mLirael) {
                isLiraelOnCamera = true;
            }
        }

        if (!isLiraelOnCamera && mLirael)
        {
            mLirael->HandleKeyPress(key, isPressed);
        }
    }
}

void Game::TogglePause()
{
    if (mGameScene != GameScene::MainMenu && mGameScene != GameScene::Credits && mGameScene != GameScene::HowToPlay && mGameScene != GameScene::DifficultySelection)
    {
        if (mGamePlayState == GamePlayState::Playing)
        {
            mGamePlayState = GamePlayState::Paused;

            mAudio->PauseSound(mMusicHandle);
            mAudio->PlaySound("Coin.wav", false);
        }
        else if (mGamePlayState == GamePlayState::Paused)
        {
            mGamePlayState = GamePlayState::Playing;

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

    if(mGamePlayState == GamePlayState::Playing)
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

    UpdateSceneManager(deltaTime);

    if ((mGameScene == GameScene::Level1 || mGameScene == GameScene::Level2 || mGameScene == GameScene::Level3) && mGamePlayState == GamePlayState::Playing) {
        const bool allNotesSpawned = (currentNoteIndex >= chart.size());
        const bool allEnemiesCleared = mEnemies.empty();

        if (allNotesSpawned && allEnemiesCleared) {
            mGamePlayState = GamePlayState::LevelComplete;

            mAudio->StopAllSounds();
            gameTimer.stop();

            if (mGameScene == GameScene::Level1) {
                SetGameScene(GameScene::Level2);
            } else if (mGameScene == GameScene::Level2) {
                SetGameScene(GameScene::Level3);
            } else {
                SetGameScene(GameScene::ToBeContinue);
            }
        }
    }

    if ((mGameScene == GameScene::Level1 || mGameScene == GameScene::Level2 || mGameScene == GameScene::Level3) && !chart.empty() && mGamePlayState == GamePlayState::Playing)
    {
        double currentTime = gameTimer.getSeconds() - mMusicStartOffset;

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
            targetPos.y -= 48;

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

            new Enemy(this, chart[currentNoteIndex], spawnPos, targetPos);

            currentNoteIndex++;
        }

        if (mCurrentLives <= 0) {
            mLirael->Kill();
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

    Enemy* hittableNote = nullptr;
    float minDistance = 10000.0f;
    Vector2 targetPos = hitTarget->GetPosition();

    for (auto enemy : mEnemies) {
        if (enemy->GetLane() == lane) {
            float dist = (enemy->GetPosition() - targetPos).Length();
            if (dist < minDistance) {
                minDistance = dist;
                hittableNote = enemy;
            }
        }
    }

    if (hittableNote && minDistance <= HIT_WINDOW_RADIUS) {
        // SDL_Log("HIT! Na pista %d", lane);
        hittableNote->setHit(true);
    } else {

        if (mGameDifficulty == Difficulty::HARD_SINGLE)
            SetCurrentLives(mCurrentLives - 1); // TODO opcional: se for deixar menos punitivo, comentar essa linha, mas deixar o feedback sonoro
        
        mMusicHandle = mAudio->PlaySound("Wood walk 2.ogg", false);
        // SDL_Log("MISS! Vidas restantes: %d", mCurrentLives);
    }
}

void Game::UnhitLane(int lane)
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

    Enemy* hittableNote = nullptr;
    float minDistance = 10000.0f;
    Vector2 targetPos = hitTarget->GetPosition();

    for (auto enemy : mEnemies) {
        if (enemy->GetLane() == lane) {
            float dist = (enemy->GetPosition() - targetPos).Length();
            if (dist < minDistance) {
                minDistance = dist;
                hittableNote = enemy;
            }
        }
    }

    if (hittableNote && minDistance <= HIT_WINDOW_RADIUS && hittableNote->GetDurationInSeconds() > 0) {
        hittableNote->setHit(false);
        // if (hittableNote->GetDurationInSeconds() <= 0) {
            mMusicHandle = mAudio->PlaySound("Wood walk 2.ogg", false);
        // }
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
    // Get actors on camera
    std::vector<Actor*> actorsOnCamera =
            mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

    for (size_t i = 0; i < mEnemies.size(); ++i)
    {
        mEnemies[i]->Update(deltaTime);
    }

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

void Game::AddEnemy(Enemy* enemy)
{
    mEnemies.emplace_back(enemy);
}

void Game::RemoveEnemy(Enemy* enemy)
{
    if (const auto iter = std::find(mEnemies.begin(), mEnemies.end(), enemy); iter != mEnemies.end()) {
        mEnemies.erase(iter);
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

    // For each actor on camera...
    for (auto actor : actorsOnCamera)
    {
        // ...get ALL of its drawable components (DrawComponent and its children)
        auto actorDrawables = actor->GetComponents<DrawComponent>();

        // Add each visible drawable component to our main list
        for (auto drawable : actorDrawables)
        {
            if (drawable && drawable->IsVisible())
            {
                drawables.emplace_back(drawable);
            }
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
    auto it = mFonts.find(fileName);
    if (it != mFonts.end()) {
        return it->second;
    }

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
    delete mSpatialHashing;
    mSpatialHashing = nullptr;

    delete mHUD;
    mHUD = nullptr;

    mEnemies.clear();
    mTargets.clear();
    if(mLirael) mLirael = nullptr;

    mUIStack.clear();

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

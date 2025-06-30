// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <vector>
#include <unordered_map>
#include "AudioSystem.h"
#include "FileReaderUtil.h"
#include "Math.h"
#include "GameTimer.h"

class Game
{
public:
    static const int LEVEL_WIDTH = 215;
    static const int LEVEL_HEIGHT = 15;
    static const int TILE_SIZE = 32;
    static const int SPAWN_DISTANCE = 700;
    static const int TRANSITION_TIME = 1;
    static const int FADE_TIME = 3;

    static const int CELL_SIZE = 128; // grades de 128x128, os objetos serao de 64x64 - esse valor pode mudar
    static const int PLAYABLE_AREA_HEIGHT = 650; // Altura em pixels onde o chão termina (grama escura).

    // Constantes para a jogabilidade do ritmo
    const float NOTE_VISIBILITY_WINDOW = 3.0f; // Notas aparecerão 3 segundos antes do tempo de acerto
    const float NOTE_TARGET_Y_POS = 650.0f;    // Posição Y onde as notas devem ser acertadas

    enum class GameScene
    {
        MainMenu,
        Level1,
        Level2,
        TO_BE_CONTINUE,
        HOW_TO_PLAY,
        CREDITS,
    };

    enum class SceneManagerState
    {
        None,
        Entering,
        Active,
        Exiting
    };

    enum class GamePlayState
    {
        Playing,
        Paused,
        GameOver,
        LevelComplete,
        Leaving
    };

    enum class FadeState {
        FadeOut,
        FadeIn,
        None
    };

    Game(int windowWidth, int windowHeight);

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);
    void ProcessInputActors();




    void HandleKeyPressActors(const int key, const bool isPressed);
    void HandleKeyDownActors(const int key, const bool isPressed);
    void HandleKeyUpActors(const int key, const bool isPressed);


    // Level functions
    void LoadMainMenu();
    void LoadLevel(const std::string& levelName, const int levelWidth, const int levelHeight);

    std::vector<Actor *> GetNearbyActors(const Vector2& position, const int range = 1);
    std::vector<class AABBColliderComponent *> GetNearbyColliders(const Vector2& position, const int range = 2);

    void Reinsert(Actor* actor);

    // Camera functions
    Vector2& GetCameraPos() { return mCameraPos; };
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };

    // Audio functions
    class AudioSystem* GetAudio() { return mAudio; }

    // UI functions
    void PushUI(class UIScreen* screen) { mUIStack.emplace_back(screen); }
    const std::vector<class UIScreen*>& GetUIStack() { return mUIStack; }

    // Window functions
    int GetWindowWidth() const { return mWindowWidth; }
    int GetWindowHeight() const { return mWindowHeight; }

    // Loading functions
    class UIFont* LoadFont(const std::string& fileName);
    SDL_Texture* LoadTexture(const std::string& texturePath);

    void SetGameScene(GameScene scene, float transitionTime = .0f);
    void ResetGameScene(float transitionTime = .0f);
    void UnloadScene();

    void SetBackgroundImage(const std::string& imagePath, const Vector2 &position = Vector2::Zero, const Vector2& size = Vector2::Zero);
    void TogglePause();

    // Game-specific
    const class Mario* GetMario() { return mMario; }

    void SetGamePlayState(GamePlayState state) { mGamePlayState = state; }
    GamePlayState GetGamePlayState() const { return mGamePlayState; }

    SDL_Renderer* GetRenderer() { return mRenderer; }

    void addScore(int points);

    void IncrementAmountOfCoins();
\
    // void AddAsteroid(class Asteroid* ast);
    // void RemoveAsteroid(class Asteroid* ast);
    // std::vector<class Asteroid*>& GetAsteroids() { return mAsteroids; }

    void AddEnemy(class Enemy* ast);
    void RemoveEnemy(class Enemy* ast);
    std::vector<class Enemy*>& GetEnemies() { return mEnemies; }

    void HitLane(int lane);
    void UnhitLane(int lane);
    const std::vector<class Target*>& GetTargets() const { return mTargets; }

private:
    void ProcessInput();
    void UpdateGame();
    void UpdateCamera();
    void GenerateOutput();

    // Scene Manager
    void UpdateSceneManager(float deltaTime);
    void ChangeScene();
    SceneManagerState mSceneManagerState;
    float mSceneManagerTimer;

    // HUD functions
    void UpdateLevelTime(float deltaTime);

    // Load the level from a CSV file as a 2D array
    int **ReadLevelData(const std::string& fileName, int width, int height);
    void BuildLevel(int** levelData, int width, int height);

    // Spatial Hashing for collision detection
    class SpatialHashing* mSpatialHashing;

    // All the UI elements
    std::vector<class UIScreen*> mUIStack;
    std::unordered_map<std::string, class UIFont*> mFonts;

    // SDL stuff
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    AudioSystem* mAudio;

    // Window properties
    int mWindowWidth;
    int mWindowHeight;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track actors state
    bool mIsRunning;
    GamePlayState mGamePlayState;

    // Track level state
    GameScene mGameScene;
    GameScene mNextScene;

    // Background and camera
    Vector3 mBackgroundColor;
    Vector3 mModColor;
    Vector2 mCameraPos;

    // Game-specific
    class Mario *mMario;
    class HUD *mHUD;
    SoundHandle mMusicHandle;

    float mGameTimer;
    int mGameTimeLimit;

    SDL_Texture *mBackgroundTexture;
    Vector2 mBackgroundSize;
    Vector2 mBackgroundPosition;

    FadeState mFadeState;
    float mFadeTime;

    int playerScore;
    int amountCoinsCollected;

    // Definicao de targets e direcao de inimigos
    float mYPosTop;
    float mYPosBottom;
    float mXPosLeft;
    float mXPosRight;

    class Lirael *mLirael;

    GameTimer gameTimer;
    std::vector<Note> chart;
    int currentNoteIndex = 0;

    // std::vector<class Asteroid*> mAsteroids;
    std::vector<class Enemy*> mEnemies;
    std::vector<class Target*> mTargets;
    float mMusicStartOffset;

    SoundHandle mMainMenuSoundHandle;
};

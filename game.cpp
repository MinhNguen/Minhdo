#include "game.h"
#include <iostream>
#include <fstream>

// ===================== LevelManager Implementation =====================

LevelManager::LevelManager() {
    currentLevel = 0;
    initializeLevels();
    loadProgress();
}

void LevelManager::initializeLevels() {
    levels = {
        {1, "Easy Valley", 6, 90, 250, 50, true, 0, GRASSLAND},
        {2, "Rocky Hills", 7, 80, 220, 100, false, 0, DESERT},
        {3, "Desert Storm", 8, 70, 200, 150, false, 0, FOREST},
        {4, "Thunder Plains", 9, 60, 180, 200, false, 0, MOUNTAIN},
        {5, "Volcano Peak", 10, 50, 150, 300, false, 0, VOLCANO}
    };
}

void LevelManager::saveProgress() {
    std::ofstream file("game_progress.dat");
    if (file.is_open()) {
        for (const auto& level : levels) {
            file << level.unlocked << " " << level.bestScore << "\n";
        }
        file.close();
    }
}

void LevelManager::loadProgress() {
    std::ifstream file("game_progress.dat");
    if (file.is_open()) {
        for (auto& level : levels) {
            file >> level.unlocked >> level.bestScore;
        }
        file.close();
    }
}

void LevelManager::unlockNextLevel() {
    if (static_cast<size_t>(currentLevel) < levels.size() - 1) {
        levels[currentLevel + 1].unlocked = true;
        saveProgress();
    }
}

void LevelManager::updateBestScore(int score) {
    if (score > levels[currentLevel].bestScore) {
        levels[currentLevel].bestScore = score;
        saveProgress();
    }
}

bool LevelManager::isLevelComplete(int score) {
    return score >= levels[currentLevel].targetScore;
}

LevelInfo& LevelManager::getCurrentLevelInfo() {
    return levels[currentLevel];
}

void LevelManager::setCurrentLevel(int index) {
    if (index >= 0 && static_cast<size_t>(index) < levels.size()) {
        currentLevel = index;
    }
}

// ===================== Game Class Implementation =====================

Game::Game(int width, int height)
    : SCREEN_WIDTH(width), SCREEN_HEIGHT(height), GROUND_Y(380),
      window(nullptr), renderer(nullptr),
      fontBig(nullptr), fontMedium(nullptr), fontSmall(nullptr), fontTiny(nullptr),
      backgroundMusic(nullptr), uiRenderer(nullptr),
      obstacleManager(GROUND_Y, 6, SCREEN_WIDTH),
      scoreManager(GROUND_Y, 6, SCREEN_WIDTH),
      powerUpManager(GROUND_Y, 6, SCREEN_WIDTH),
      mapTheme(GRASSLAND),
      dayNightCycle(0.0008f),
      state(GameState::MENU),
      running(true),
      gameOver(false), musicPlaying(false) {

    player.groundY = GROUND_Y;
    player.y = GROUND_Y;
    player.totalLevelsCompleted = 0;
}

Game::~Game() {
    cleanup();
}

bool Game::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    if (Mix_Init(MIX_INIT_MP3) != MIX_INIT_MP3) {
        std::cerr << "Mix_Init Error: " << Mix_GetError() << std::endl;
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
        Mix_Quit();
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    window = SDL_CreateWindow("Dino Game - Complete System",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    fontBig = TTF_OpenFont("NotoSans-Regular.ttf", 48);
    fontMedium = TTF_OpenFont("NotoSans-Regular.ttf", 32);
    fontSmall = TTF_OpenFont("NotoSans-Regular.ttf", 24);
    fontTiny = TTF_OpenFont("NotoSans-Regular.ttf", 18);

    if (!fontBig || !fontSmall || !fontMedium || !fontTiny) {
        std::cerr << "Failed to load font! Error: " << TTF_GetError() << std::endl;
        cleanup();
        return false;
    }

    backgroundMusic = Mix_LoadMUS("image/music.mp3");
    if (!backgroundMusic) {
        std::cerr << "Failed to load music (music.mp3)! Error: " << Mix_GetError() << std::endl;
    }

    uiRenderer = UIRenderer(renderer);
    shop.initialize(renderer);
    loadProgress();

    if (dailyResetSystem.shouldResetDaily()) {
        std::cout << "Daily reset detected. Resetting daily quests..." << std::endl;
        questSystem.resetDailyQuests();
        dailyResetSystem.markReset();
        saveProgress();
    }

    return true;
}

void Game::run() {
    while (running) {
        uiRenderer.update();
        handleEvents();
        if (state == GameState::PLAYING && !gameOver && !musicPlaying) {
            if (backgroundMusic) {
                Mix_PlayMusic(backgroundMusic, -1); // -1 để lặp vô tận
            }
            musicPlaying = true;
        }
        else if ((state != GameState::PLAYING || gameOver) && musicPlaying) {
            Mix_HaltMusic(); // Dừng nhạc
            musicPlaying = false;
        }

        update();
        render();
        SDL_Delay(16);
    }
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
            return;
        }

        switch (state) {
            case GameState::MENU:
                handleMenuInput(e);
                break;
            case GameState::LEVEL_SELECT:
                handleLevelSelectInput(e);
                break;
            case GameState::PLAYING:
                handlePlayingInput(e);
                break;
            case GameState::GAME_OVER:
                handleGameOverInput(e);
                break;
            case GameState::LEVEL_COMPLETE:
                handleLevelCompleteInput(e);
                break;
            case GameState::SHOP:
                handleShopInput(e);
                break;
            case GameState::QUEST:
                handleQuestInput(e);
                break;
            case GameState::ACHIEVEMENT:
                handleAchievementInput(e);
                break;
            case GameState::LEADERBOARD:
                handleLeaderboardInput(e);
                break;
        }
    }
}

void Game::handleMenuInput(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mx = e.button.x, my = e.button.y;

        SDL_Rect playBtn = { SCREEN_WIDTH / 2 - 150, 140, 300, 60 };
        SDL_Rect shopBtn = { SCREEN_WIDTH / 2 - 150, 220, 300, 60 };
        SDL_Rect questBtn = { SCREEN_WIDTH / 2 - 150, 300, 300, 60 };
        SDL_Rect achievementBtn = { SCREEN_WIDTH / 2 - 150, 380, 300, 60 };
        SDL_Rect leaderboardBtn = { SCREEN_WIDTH / 2 - 150, 460, 300, 60 };
        SDL_Rect quitBtn = { SCREEN_WIDTH / 2 - 150, 540, 300, 60 };

        if (mx >= playBtn.x && mx <= playBtn.x + playBtn.w && my >= playBtn.y && my <= playBtn.y + playBtn.h)
            state = GameState::LEVEL_SELECT;
        else if (mx >= shopBtn.x && mx <= shopBtn.x + shopBtn.w && my >= shopBtn.y && my <= shopBtn.y + shopBtn.h)
            state = GameState::SHOP;
        else if (mx >= questBtn.x && mx <= questBtn.x + questBtn.w && my >= questBtn.y && my <= questBtn.y + questBtn.h)
            state = GameState::QUEST;
        else if (mx >= achievementBtn.x && mx <= achievementBtn.x + achievementBtn.w && my >= achievementBtn.y && my <= achievementBtn.y + achievementBtn.h)
            state = GameState::ACHIEVEMENT;
        else if (mx >= leaderboardBtn.x && mx <= leaderboardBtn.x + leaderboardBtn.w && my >= leaderboardBtn.y && my <= leaderboardBtn.y + leaderboardBtn.h)
            state = GameState::LEADERBOARD;
        else if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w && my >= quitBtn.y && my <= quitBtn.y + quitBtn.h)
            running = false;
    }
}

void Game::handleLevelSelectInput(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mx = e.button.x, my = e.button.y;

        for (size_t i = 0; i < levelManager.levels.size(); i++) {
            int btnX = 50 + (i % 3) * 250;
            int btnY = 120 + (i / 3) * 100;
            SDL_Rect levelBtn = { btnX, btnY, 200, 80 };

            if (mx >= levelBtn.x && mx <= levelBtn.x + levelBtn.w &&
                my >= levelBtn.y && my <= levelBtn.y + levelBtn.h &&
                levelManager.levels[i].unlocked) {
                startLevel(i);
                break;
            }
        }

        SDL_Rect backBtnLS = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 70, 200, 50 };
        if (mx >= backBtnLS.x && mx <= backBtnLS.x + backBtnLS.w &&
            my >= backBtnLS.y && my <= backBtnLS.y + backBtnLS.h)
            state = GameState::MENU;
    }
}

void Game::handlePlayingInput(SDL_Event& e) {
    if (gameOver) return;

    if (e.type == SDL_KEYDOWN) {
        if ((e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_UP) && player.isOnGround) {
            player.vy = -12.0f;
            player.isOnGround = false;
            questSystem.onJump();
        } else if (e.key.keysym.sym == SDLK_d && powerUpManager.canDash()) {
            player.x += 100;
            powerUpManager.useDash(player);
        } else if (e.key.keysym.sym == SDLK_ESCAPE) {
            levelManager.updateBestScore(scoreManager.getCurrentScore());
            saveProgress();
            state = GameState::LEVEL_SELECT;
        }
    }
}

void Game::handleGameOverInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_r) {
            resetGame();
        } else if (e.key.keysym.sym == SDLK_m) {
            saveProgress();
            state = GameState::LEVEL_SELECT;
        }
    }
}

void Game::handleLevelCompleteInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_n) {
            if (static_cast<size_t>(levelManager.currentLevel) < levelManager.levels.size() - 1) {
                startLevel(levelManager.currentLevel + 1);
            }
        } else if (e.key.keysym.sym == SDLK_m) {
            saveProgress();
            state = GameState::LEVEL_SELECT;
        }
    }
}

void Game::handleShopInput(SDL_Event& e) {
    auto saveCallback = [this]() { saveProgress(); };

    if (shop.handleInput(e, player, player.equippedSkinIndex, saveCallback)) {
        state = GameState::MENU;
    }

    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        state = GameState::MENU;
    }
}

void Game::handleQuestInput(SDL_Event& e) {
    if (questScreen.handleInput(e, questSystem, player)) {
        saveProgress();
        state = GameState::MENU;
    }

    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        saveProgress();
        state = GameState::MENU;
    }
}

void Game::handleAchievementInput(SDL_Event& e) {
    if (achievementScreen.handleInput(e, SCREEN_WIDTH, SCREEN_HEIGHT, achievementSystem, questSystem, player)) {
        state = GameState::MENU;
    }
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        state = GameState::MENU;
    }
}

void Game::handleLeaderboardInput(SDL_Event& e) {
    if (leaderboard.handleInput(e)) {
        state = GameState::MENU;
    }

    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        state = GameState::MENU;
    }
}

void Game::update() {
    if (state == GameState::PLAYING && !gameOver) {
        // Update day/night cycle
        dayNightCycle.update();
        mapTheme.update(SCREEN_WIDTH, SCREEN_HEIGHT, dayNightCycle);

        if (!player.isOnGround) player.vy += player.gravity;
        player.y += 2*player.vy;
        if (player.y >= GROUND_Y) {
            player.y = GROUND_Y;
            player.vy = 0;
            player.isOnGround = true;
        }

        difficultyManager.update();
        obstacleManager.setSpeed(difficultyManager.getSpeed());
        obstacleManager.update();
        scoreManager.setSpeed(difficultyManager.getSpeed());
        scoreManager.update(player);

        questSystem.onScoreUpdate(scoreManager.getCurrentScore());
        for (auto& coin : scoreManager.coins) {
            if (!coin.collected && coin.checkCollision(player.x, player.y, player.width, player.height)) {
                comboSystem.addCombo();
                questSystem.onCoinCollected();
            }
        }
        LevelInfo& level = levelManager.getCurrentLevelInfo();
        powerUpManager.setSpeed(level.obstacleSpeed);
        powerUpManager.update(player, &scoreManager);
        for (auto& pu : powerUpManager.powerUps) {
            if (!pu.collected && pu.checkCollision(player.x, player.y, player.width, player.height)) {
                questSystem.onPowerupCollected();
                player.totalPowerupsCollected++;
            }
        }

        comboSystem.update();
        questSystem.onComboUpdate(comboSystem.currentCombo);
        if (comboSystem.getMaxCombo() > player.bestComboAchieved) {
            player.bestComboAchieved = comboSystem.getMaxCombo();
        }
        questSystem.onSurvivalTimeUpdate();
        questSystem.updateQuests(player);
        achievementSystem.update();

        achievementSystem.checkAchievements(scoreManager.getCurrentScore(),
                                           player.totalCoins,
                                           comboSystem.getMaxCombo(),
                                           levelManager.currentLevel + 1);

        if (levelManager.isLevelComplete(scoreManager.getCurrentScore())) {
            levelManager.updateBestScore(scoreManager.getCurrentScore());
            levelManager.unlockNextLevel();
            player.addXp(50);
            player.totalCoins += achievementSystem.getTotalRewardsEarned();
            achievementSystem.clearSessionRewards();
            questSystem.onLevelComplete();
            player.totalLevelsCompleted++;
            saveProgress();
            state = GameState::LEVEL_COMPLETE;
        }

        if (obstacleManager.checkCollisionWithPlayer(player.x, player.y, player.width, player.height) &&
            !powerUpManager.shieldActive) {
            gameOver = true;
            questSystem.onDamageTaken();
            levelManager.updateBestScore(scoreManager.getCurrentScore());
            player.totalCoins += achievementSystem.getTotalRewardsEarned();
            achievementSystem.clearSessionRewards();
            saveProgress();
            state = GameState::GAME_OVER;
        }
    }
}

void Game::render() {
    SDL_RenderClear(renderer);

    switch (state) {
        case GameState::MENU:
            renderMenu();
            break;
        case GameState::LEVEL_SELECT:
            renderLevelSelect();
            break;
        case GameState::PLAYING:
        case GameState::GAME_OVER:
        case GameState::LEVEL_COMPLETE:
            renderPlaying();
            break;
        case GameState::SHOP:
            renderShop();
            break;
        case GameState::QUEST:
            renderQuest();
            break;
        case GameState::ACHIEVEMENT:
            renderAchievement();
            break;
        case GameState::LEADERBOARD:
            renderLeaderboard();
            break;
    }

    SDL_RenderPresent(renderer);
}

void Game::renderMenu() {
    SDL_Color white = {255, 255, 255, 255};

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        float t = (float)y / SCREEN_HEIGHT;
        Uint8 r = 30 + (120 - 30) * t;
        Uint8 g = 144 + (200 - 144) * t;
        Uint8 b = 255;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
    }

    SDL_FRect titlePanel = {150, 30, 500, 90};
    uiRenderer.drawEnhancedGlassPanel(titlePanel, {80, 120, 200, 200});
    uiRenderer.renderTextCentered("DINO ADVENTURE", 400, 75, fontBig, white);

    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_FRect playBtn = {SCREEN_WIDTH/2.0f - 150, 150, 300, 60};
    bool hovPlay = (mx >= playBtn.x && mx <= playBtn.x + playBtn.w &&
                    my >= playBtn.y && my <= playBtn.y + playBtn.h);
    uiRenderer.renderEnhancedButton(playBtn, hovPlay, "PLAY", fontSmall, {50, 150, 50, 255});

    SDL_FRect shopBtn = {SCREEN_WIDTH/2.0f - 150, 230, 300, 60};
    bool hovShop = (mx >= shopBtn.x && mx <= shopBtn.x + shopBtn.w &&
                    my >= shopBtn.y && my <= shopBtn.y + shopBtn.h);
    uiRenderer.renderEnhancedButton(shopBtn, hovShop, "SHOP", fontSmall, {200, 150, 50, 255});

    SDL_FRect questBtn = {SCREEN_WIDTH/2.0f - 150, 310, 300, 60};
    bool hovQuest = (mx >= questBtn.x && mx <= questBtn.x + questBtn.w &&
                     my >= questBtn.y && my <= questBtn.y + questBtn.h);
    uiRenderer.renderEnhancedButton(questBtn, hovQuest, "QUESTS", fontSmall, {70, 180, 100, 255});

    SDL_FRect achievementBtn = {SCREEN_WIDTH/2.0f - 150, 390, 300, 60};
    bool hovAchievement = (mx >= achievementBtn.x && mx <= achievementBtn.x + achievementBtn.w &&
                          my >= achievementBtn.y && my <= achievementBtn.y + achievementBtn.h);
    uiRenderer.renderEnhancedButton(achievementBtn, hovAchievement, "ACHIEVEMENTS",
                                   fontSmall, {200, 150, 50, 255});

    SDL_FRect leaderboardBtn = {SCREEN_WIDTH/2.0f - 150, 470, 300, 60};
    bool hovLeaderboard = (mx >= leaderboardBtn.x && mx <= leaderboardBtn.x + leaderboardBtn.w &&
                          my >= leaderboardBtn.y && my <= leaderboardBtn.y + leaderboardBtn.h);
    uiRenderer.renderEnhancedButton(leaderboardBtn, hovLeaderboard, "LEADERBOARD",
                                   fontSmall, {100, 100, 200, 255});

    SDL_FRect quitBtn = {SCREEN_WIDTH/2.0f - 150, 550, 300, 60};
    bool hovQuit = (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                    my >= quitBtn.y && my <= quitBtn.y + quitBtn.h);
    uiRenderer.renderEnhancedButton(quitBtn, hovQuit, "QUIT", fontSmall, {200, 50, 50, 255});
}

void Game::renderLevelSelect() {
    SDL_Color white = {255,255,255,255};
    SDL_Color green = {0,200,0,255};

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        float t = (float)y / SCREEN_HEIGHT;
        Uint8 r = 50 + (130 - 50) * t;
        Uint8 g = 150 + (200 - 150) * t;
        Uint8 b = 255;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
    }

    SDL_FRect titlePanel = {200, 15, 400, 70};
    uiRenderer.drawEnhancedGlassPanel(titlePanel, {100, 150, 250, 200});
    uiRenderer.renderTextCentered("SELECT LEVEL", 400, 50, fontBig, white);

    int mx, my;
    SDL_GetMouseState(&mx, &my);

    for (size_t i = 0; i < levelManager.levels.size(); i++) {
        LevelInfo& level = levelManager.levels[i];
        int btnX = 50 + (i % 3) * 250;
        int btnY = 110 + (i / 3) * 110;
        SDL_FRect levelBtn = {(float)btnX, (float)btnY, 200, 90};

        bool hovered = (mx >= levelBtn.x && mx <= levelBtn.x + levelBtn.w &&
                       my >= levelBtn.y && my <= levelBtn.y + levelBtn.h);

        SDL_Color bgColor;
        if (level.unlocked) {
            float hue = 120.0f - (i * 30.0f);
            bgColor = uiRenderer.hsvToRgb(hue, 0.6f, 0.7f);
            bgColor.a = 200;
        } else {
            bgColor = {80, 80, 80, 180};
        }

        uiRenderer.drawEnhancedGlassPanel(levelBtn, bgColor);

        std::string levelNum = "Level " + std::to_string(level.levelNumber);
        uiRenderer.renderTextLeft(levelNum, levelBtn.x + 15, levelBtn.y + 12, fontSmall, white);

        if (level.unlocked) {
            uiRenderer.renderTextLeft(level.name, levelBtn.x + 15, levelBtn.y + 40,
                                     fontTiny, {240, 240, 240, 255});

            std::string goalText = "Goal: " + std::to_string(level.targetScore);
            uiRenderer.renderTextLeft(goalText, levelBtn.x + 15, levelBtn.y + 62, fontTiny, green);

            if (level.bestScore > 0) {
                SDL_FRect badge = {levelBtn.x + levelBtn.w - 85, levelBtn.y + 8, 75, 25};
                std::string bestText = "Best: " + std::to_string(level.bestScore);
                uiRenderer.renderEnhancedButton(badge, false, bestText, fontTiny,
                                               {255, 215, 0, 255});
            }
        } else {
            SDL_FRect lockIcon = {levelBtn.x + levelBtn.w/2 - 30, levelBtn.y + 35, 60, 40};
            uiRenderer.drawRoundedRect(lockIcon, 10, {50, 50, 50, 255});
            uiRenderer.renderTextCentered("LOCKED", levelBtn.x + levelBtn.w/2,
                                        levelBtn.y + 55, fontTiny, {128,128,128,255});
        }

        if (hovered && level.unlocked) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 30);
            SDL_FRect hoverRect = {levelBtn.x, levelBtn.y, levelBtn.w, levelBtn.h};
            SDL_RenderFillRectF(renderer, &hoverRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        }
    }

    SDL_FRect backBtnLS = {SCREEN_WIDTH/2.0f - 100, SCREEN_HEIGHT - 70, 200, 50};
    bool hovBackLS = (mx >= backBtnLS.x && mx <= backBtnLS.x + backBtnLS.w &&
                      my >= backBtnLS.y && my <= backBtnLS.y + backBtnLS.h);
    uiRenderer.renderEnhancedButton(backBtnLS, hovBackLS, "BACK", fontSmall, {180, 50, 50, 255});
}

void Game::renderPlaying() {
    // Render dynamic background with day/night cycle
    mapTheme.renderBackground(renderer, SCREEN_WIDTH, SCREEN_HEIGHT, dayNightCycle);

    // Render ground with theme
    mapTheme.renderGround(renderer, GROUND_Y, SCREEN_WIDTH, SCREEN_HEIGHT, dayNightCycle);

    // Render environment particles
    mapTheme.renderParticles(renderer);

    // Render game objects
    obstacleManager.render(renderer);
    scoreManager.render(renderer);
    powerUpManager.render(renderer);

    // Render player
    SDL_Texture* skin = shop.items[player.equippedSkinIndex].texture;
    if (skin) {
        SDL_Rect rect = { player.x, player.y - (int)player.height, (int)player.width, (int)player.height };
        SDL_RenderCopy(renderer, skin, nullptr, &rect);
    }

    // UI overlay with adjusted colors for visibility
    SDL_Color white = {255,255,255,255};
    SDL_Color yellow = {255,255,0,255};
    SDL_Color black = {0,0,0,255};

    // Semi-transparent background for text readability
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect uiPanel = {5, 5, 300, 90};
    SDL_RenderFillRect(renderer, &uiPanel);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    LevelInfo& level = levelManager.getCurrentLevelInfo();
    renderLeftText(fontSmall, "Level " + std::to_string(level.levelNumber) + ": " + level.name, white, 10, 10);
    renderLeftText(fontSmall, "Score: " + std::to_string(scoreManager.getCurrentScore()) + " / " + std::to_string(level.targetScore), white, 10, 40);

    // Display time of day
    std::string timeOfDayStr;
    switch(dayNightCycle.getCurrentTimeOfDay()) {
        case MORNING: timeOfDayStr = "Morning"; break;
        case DAY: timeOfDayStr = "Day"; break;
        case EVENING: timeOfDayStr = "Evening"; break;
        case NIGHT: timeOfDayStr = "Night"; break;
    }
    renderLeftText(fontTiny, "Time: " + timeOfDayStr, {200,200,255,255}, 10, 70);

    // Player stats panel
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect statsPanel = {SCREEN_WIDTH - 160, 5, 155, 60};
    SDL_RenderFillRect(renderer, &statsPanel);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    renderLeftText(fontTiny, "Coins: " + std::to_string(player.totalCoins), yellow, SCREEN_WIDTH - 150, 10);
    renderLeftText(fontTiny, "Lvl " + std::to_string(player.level) + " (XP: " + std::to_string(player.xp) + "/" + std::to_string(player.xpToNextLevel) + ")", white, SCREEN_WIDTH - 150, 30);

    if (state == GameState::GAME_OVER) {
        // Dark overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        renderCenteredText(fontBig, "GAME OVER", white, 120, SCREEN_WIDTH);
        renderCenteredText(fontMedium, "Final Score: " + std::to_string(scoreManager.getCurrentScore()), yellow, 190, SCREEN_WIDTH);
        renderCenteredText(fontSmall, "Max Combo: x" + std::to_string(comboSystem.getMaxCombo()), white, 230, SCREEN_WIDTH);
        renderCenteredText(fontSmall, "PRESS R TO RETRY OR M FOR MENU", white, 280, SCREEN_WIDTH);
    }
    else if (state == GameState::LEVEL_COMPLETE) {
        // Light overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Color green = { 0, 255, 0, 255 };
        SDL_Color gold = {255, 215, 0, 255};
        renderCenteredText(fontBig, "LEVEL COMPLETE!", green, 80, SCREEN_WIDTH);
        renderCenteredText(fontMedium, "Final Score: " + std::to_string(scoreManager.getCurrentScore()), gold, 190, SCREEN_WIDTH);

        if (static_cast<size_t>(levelManager.currentLevel) < levelManager.levels.size() - 1) {
            renderCenteredText(fontSmall, "Press N for NEXT LEVEL or M for MENU", white, 310, SCREEN_WIDTH);
        } else {
            renderCenteredText(fontSmall, "Congratulations! All levels complete!", gold, 270, SCREEN_WIDTH);
            renderCenteredText(fontSmall, "Press M for MENU", white, 310, SCREEN_WIDTH);
        }
    }
}

void Game::renderShop() {
    SDL_SetRenderDrawColor(renderer, 230, 230, 250, 255);
    SDL_RenderClear(renderer);
    shop.render(renderer, fontBig, fontSmall, fontTiny, player, uiRenderer);
}

void Game::renderQuest() {
    SDL_SetRenderDrawColor(renderer, 240, 240, 255, 255);
    SDL_RenderClear(renderer);
    questScreen.render(renderer, fontBig, fontSmall, fontTiny, questSystem, player, uiRenderer);
}

void Game::renderAchievement() {
    SDL_SetRenderDrawColor(renderer, 255, 250, 240, 255);
    SDL_RenderClear(renderer);
    achievementScreen.render(renderer, fontBig, fontMedium, fontSmall,
                     SCREEN_WIDTH, SCREEN_HEIGHT, achievementSystem, player);
    achievementScreen.updateParticles();
}

void Game::renderLeaderboard() {
    SDL_SetRenderDrawColor(renderer, 240, 240, 255, 255);
    SDL_RenderClear(renderer);
    leaderboard.render(renderer, fontBig, fontSmall, fontTiny, uiRenderer);
}

void Game::saveProgress() {
    std::ofstream file("game_progress.dat", std::ios::trunc);
    if (file.is_open()) {
        for (const auto& level : levelManager.levels) {
            file << level.unlocked << " " << level.bestScore << "\n";
        }
        file << player.totalCoins << "\n";
        file << player.equippedSkinIndex << "\n";
        file << player.level << "\n";
        file << player.xp << "\n";
        file << player.xpToNextLevel << "\n";
        file << player.totalLevelsCompleted << "\n";
        file << player.totalPowerupsCollected << "\n";
        file << player.bestComboAchieved << "\n";

        for (const auto& item : shop.items) {
            file << item.isOwned << " ";
        }
        file << "\n";
        file.close();
    }
    achievementSystem.saveProgress();
    questSystem.saveProgress();
}

void Game::loadProgress() {
    std::ifstream file("game_progress.dat");
    if (!file.is_open()) {
        shop.items[0].isOwned = true;
        return;
    }

    for (auto& level : levelManager.levels) {
        file >> level.unlocked >> level.bestScore;
    }
    file >> player.totalCoins;
    file >> player.equippedSkinIndex;
    file >> player.level;
    file >> player.xp;
    file >> player.xpToNextLevel;
    file >> player.totalLevelsCompleted;
    file >> player.totalPowerupsCollected;
    file >> player.bestComboAchieved;

    // (Thêm kiểm tra file.fail() nếu cần, nhưng nếu tệp cũ thì các biến sẽ là 0)
    if(file.fail()) {
        player.totalPowerupsCollected = 0;
        player.bestComboAchieved = 0;
        // Xóa cờ lỗi để tiếp tục đọc phần còn lại của tệp
        file.clear();
    }

    for (auto& item : shop.items) {
        file >> item.isOwned;
    }
    shop.items[0].isOwned = true;
    file.close();

    achievementSystem.loadProgress();
    questSystem.loadProgress();
}

void Game::resetGame() {
    LevelInfo& level = levelManager.getCurrentLevelInfo();
    obstacleManager.clear();
    obstacleManager.spawnInterval = level.spawnInterval;
    scoreManager.reset();
    powerUpManager.reset();
    comboSystem.reset();
    difficultyManager.reset();
    questSystem.resetSessionStats();
    dayNightCycle.reset();
    player.x = 50;
    player.y = GROUND_Y;
    player.vy = 0;
    player.isOnGround = true;
    gameOver = false;
    state = GameState::PLAYING;
}

void Game::startLevel(int levelIndex) {
    levelManager.setCurrentLevel(levelIndex);
    LevelInfo& level = levelManager.getCurrentLevelInfo();

    // Set map theme cho level
    mapTheme.setTheme(level.themeType);
    dayNightCycle.reset();

    obstacleManager = ObstacleManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);
    scoreManager = ScoreManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);
    powerUpManager = PowerUpManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);

    player.x = 50;
    player.y = GROUND_Y;
    player.vy = 0;
    player.isOnGround = true;

    comboSystem.reset();
    difficultyManager.reset();
    questSystem.resetSessionStats();

    gameOver = false;
    state = GameState::PLAYING;
}

SDL_Texture* Game::renderText(TTF_Font* font, const std::string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "TTF_RenderText Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void Game::renderCenteredText(TTF_Font* font, const std::string& text, SDL_Color color, int y, int screenW) {
    SDL_Texture* tex = renderText(font, text, color);
    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { (screenW - w) / 2, y, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
}

void Game::renderLeftText(TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
    SDL_Texture* tex = renderText(font, text, color);
    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { x, y, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
}

void Game::cleanup() {
    saveProgress();
    shop.cleanup();

    if (backgroundMusic) {
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
    Mix_CloseAudio();
    Mix_Quit();

    if (fontBig) TTF_CloseFont(fontBig);
    if (fontMedium) TTF_CloseFont(fontMedium);
    if (fontSmall) TTF_CloseFont(fontSmall);
    if (fontTiny) TTF_CloseFont(fontTiny);

    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

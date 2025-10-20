#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <functional>
#include <algorithm> // Cần cho std::remove_if

// Include all game systems
#include "player.h"
#include "obstacle.h"
#include "score.h"
#include "powerup.h"
#include "combo_achievement.h"
#include "quest_system.h"
#include "shop.h"
#include "quest_screen.h"

enum class GameState {
    MENU,
    LEVEL_SELECT,
    PLAYING,
    GAME_OVER,
    LEVEL_COMPLETE,
    SHOP,
    QUEST
};

// ===================== Level Info Structure =====================
struct LevelInfo {
    int levelNumber;
    std::string name;
    int obstacleSpeed;
    int spawnInterval;
    int meteorInterval;
    int targetScore;
    bool unlocked;
    int bestScore;
};

// ===================== Level Manager =====================
class LevelManager {
public:
    std::vector<LevelInfo> levels;
    int currentLevel;

    LevelManager() {
        currentLevel = 0;
        initializeLevels();
        loadProgress();
    }

    void initializeLevels() {
        levels = {
            {1, "Easy Valley", 6, 90, 250, 50, true, 0},
            {2, "Rocky Hills", 7, 80, 220, 100, false, 0},
            {3, "Desert Storm", 8, 70, 200, 150, false, 0},
            {4, "Thunder Plains", 9, 60, 180, 200, false, 0},
            {5, "Volcano Peak", 10, 50, 150, 300, false, 0}
        };
    }

    void saveProgress() {
        std::ofstream file("game_progress.dat");
        if (file.is_open()) {
            for (const auto& level : levels) {
                file << level.unlocked << " " << level.bestScore << "\n";
            }
            file.close();
        }
    }

    void loadProgress() {
        std::ifstream file("game_progress.dat");
        if (file.is_open()) {
            for (auto& level : levels) {
                file >> level.unlocked >> level.bestScore;
            }
            file.close();
        }
    }

    void unlockNextLevel() {
        if (static_cast<size_t>(currentLevel) < levels.size() - 1) {
            levels[currentLevel + 1].unlocked = true;
            saveProgress();
        }
    }

    void updateBestScore(int score) {
        if (score > levels[currentLevel].bestScore) {
            levels[currentLevel].bestScore = score;
            saveProgress();
        }
    }

    bool isLevelComplete(int score) {
        return score >= levels[currentLevel].targetScore;
    }

    LevelInfo& getCurrentLevelInfo() {
        return levels[currentLevel];
    }

    void setCurrentLevel(int index) {
        if (index >= 0 && static_cast<size_t>(index) < levels.size()) {
            currentLevel = index;
        }
    }
};

// ===================== Helper to Render Text =====================
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "TTF_RenderText Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                        SDL_Color color, int y, int screenW) {
    SDL_Texture* tex = renderText(renderer, font, text, color);
    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { (screenW - w) / 2, y, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
}

void renderLeftText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                    SDL_Color color, int x, int y) {
    SDL_Texture* tex = renderText(renderer, font, text, color);
    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { x, y, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
}

// ===================== Save/Load Progress =====================
void savePlayerProgress(const Player& player, const Shop& shop, LevelManager& levelManager,
                       AchievementSystem& achievements, QuestSystem& quests) {
    std::ofstream file("game_progress.dat", std::ios::trunc);
    if (file.is_open()) {
        // Lưu tiến trình level
        for (const auto& level : levelManager.levels) {
            file << level.unlocked << " " << level.bestScore << "\n";
        }

        // Lưu thông tin người chơi
        file << player.totalCoins << "\n";
        file << player.equippedSkinIndex << "\n";
        file << player.level << "\n";
        file << player.xp << "\n";
        file << player.xpToNextLevel << "\n";

        // Lưu trạng thái shop
        for (const auto& item : shop.items) {
            file << item.isOwned << " ";
        }
        file << "\n";

        file.close();
    }

    // Lưu achievements và quests
    achievements.saveProgress();
    quests.saveProgress();
}

void loadPlayerProgress(Player& player, Shop& shop, LevelManager& levelManager, AchievementSystem& achievements, QuestSystem& quests) {
    std::ifstream file("game_progress.dat");
    if (!file.is_open()) { // Nếu file không tồn tại, khởi tạo giá trị mặc định
        shop.items[0].isOwned = true;
        return;
    }

    // Load level progress
    for (auto& level : levelManager.levels) {
        file >> level.unlocked >> level.bestScore;
    }

    // Load player data
    file >> player.totalCoins;
    file >> player.equippedSkinIndex;
    file >> player.level;
    file >> player.xp;
    file >> player.xpToNextLevel;

    // Load shop data
    for (auto& item : shop.items) {
        file >> item.isOwned;
    }
    shop.items[0].isOwned = true; // Skin mặc định luôn sở hữu

    file.close();

    // Load achievements and quests
    achievements.loadProgress();
    quests.loadProgress();
}


// ===================== Main =====================
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 480;
    const int GROUND_Y = 380;

    SDL_Window* window = SDL_CreateWindow("Dino Game - Complete System",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* fontBig = TTF_OpenFont("NotoSans-Regular.ttf", 48);
    TTF_Font* fontMedium = TTF_OpenFont("NotoSans-Regular.ttf", 32);
    TTF_Font* fontSmall = TTF_OpenFont("NotoSans-Regular.ttf", 24);
    TTF_Font* fontTiny = TTF_OpenFont("NotoSans-Regular.ttf", 18);
    if (!fontBig || !fontSmall || !fontMedium || !fontTiny) {
        std::cerr << "Failed to load font! Error: " << TTF_GetError() << std::endl;
        IMG_Quit(); TTF_Quit(); SDL_Quit();
        return 1;
    }

    Player player;
    player.groundY = GROUND_Y;
    player.y = GROUND_Y;

    ObstacleManager obstacleManager(GROUND_Y, 6, SCREEN_WIDTH);
    ScoreManager scoreManager(GROUND_Y, 6, SCREEN_WIDTH);
    PowerUpManager powerUpManager(GROUND_Y, 6, SCREEN_WIDTH);
    LevelManager levelManager;
    Shop shop;
    ComboSystem comboSystem;
    AchievementSystem achievementSystem;
    DifficultyManager difficultyManager;
    QuestSystem questSystem;
    QuestScreen questScreen;

    shop.initialize(renderer);
    loadPlayerProgress(player, shop, levelManager, achievementSystem, questSystem);

    GameState state = GameState::MENU;
    bool running = true;
    bool gameOver = false;
    SDL_Event e;

    SDL_Rect playBtn = { SCREEN_WIDTH / 2 - 150, 140, 300, 60 };
    SDL_Rect shopBtn = { SCREEN_WIDTH / 2 - 150, 220, 300, 60 };
    SDL_Rect questBtn = { SCREEN_WIDTH / 2 - 150, 300, 300, 60 };
    SDL_Rect quitBtn = { SCREEN_WIDTH / 2 - 150, 380, 300, 60 };

    auto saveCallback = [&]() {
        savePlayerProgress(player, shop, levelManager, achievementSystem, questSystem);
    };

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (state == GameState::MENU && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;
                if (mx >= playBtn.x && mx <= playBtn.x + playBtn.w && my >= playBtn.y && my <= playBtn.y + playBtn.h) state = GameState::LEVEL_SELECT;
                else if (mx >= shopBtn.x && mx <= shopBtn.x + shopBtn.w && my >= shopBtn.y && my <= shopBtn.y + shopBtn.h) state = GameState::SHOP;
                else if (mx >= questBtn.x && mx <= questBtn.x + questBtn.w && my >= questBtn.y && my <= questBtn.y + questBtn.h) state = GameState::QUEST;
                else if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w && my >= quitBtn.y && my <= quitBtn.y + quitBtn.h) running = false;
            }
            else if (state == GameState::SHOP) {
                if (shop.handleInput(e, player, player.equippedSkinIndex, saveCallback)) state = GameState::MENU;
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) state = GameState::MENU;
            }
            else if (state == GameState::QUEST) {
                if (questScreen.handleInput(e, questSystem, player)) { saveCallback(); state = GameState::MENU; }
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) { saveCallback(); state = GameState::MENU; }
            }
            else if (state == GameState::LEVEL_SELECT && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;
                for (size_t i = 0; i < levelManager.levels.size(); i++) {
                    int btnX = 50 + (i % 3) * 250;
                    int btnY = 120 + (i / 3) * 100;
                    SDL_Rect levelBtn = { btnX, btnY, 200, 80 };
                    if (mx >= levelBtn.x && mx <= levelBtn.x + levelBtn.w && my >= levelBtn.y && my <= levelBtn.y + levelBtn.h && levelManager.levels[i].unlocked) {
                        levelManager.setCurrentLevel(i);
                        LevelInfo& level = levelManager.getCurrentLevelInfo();

                        obstacleManager = ObstacleManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);
                        scoreManager = ScoreManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);
                        powerUpManager = PowerUpManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);

                        player.x = 50; player.y = GROUND_Y; player.vy = 0; player.isOnGround = true;

                        comboSystem.reset();
                        difficultyManager.reset();
                        questSystem.resetSessionStats();

                        gameOver = false;
                        state = GameState::PLAYING;
                        break;
                    }
                }
                SDL_Rect backBtnLS = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 70, 200, 50 };
                if (mx >= backBtnLS.x && mx <= backBtnLS.x + backBtnLS.w && my >= backBtnLS.y && my <= backBtnLS.y + backBtnLS.h) state = GameState::MENU;
            }
            else if (state == GameState::PLAYING && !gameOver && e.type == SDL_KEYDOWN) {
                if ((e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_UP) && player.isOnGround) {
                    player.vy = -12.0f; player.isOnGround = false; questSystem.onJump();
                } else if (e.key.keysym.sym == SDLK_d && powerUpManager.canDash()) {
                    player.x += 100; powerUpManager.useDash();
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    levelManager.updateBestScore(scoreManager.getCurrentScore());
                    saveCallback(); state = GameState::LEVEL_SELECT;
                }
            }
            else if (state == GameState::GAME_OVER && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_r) {
                    LevelInfo& level = levelManager.getCurrentLevelInfo();
                    obstacleManager.clear(); obstacleManager.spawnInterval = level.spawnInterval;
                    scoreManager.reset(); powerUpManager.reset();
                    comboSystem.reset(); difficultyManager.reset(); questSystem.resetSessionStats();
                    player.x = 50; player.y = GROUND_Y; player.vy = 0; player.isOnGround = true;
                    gameOver = false; state = GameState::PLAYING;
                } else if (e.key.keysym.sym == SDLK_m) {
                    saveCallback(); state = GameState::LEVEL_SELECT;
                }
            }
            else if (state == GameState::LEVEL_COMPLETE && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_n) {
                    if (static_cast<size_t>(levelManager.currentLevel) < levelManager.levels.size() - 1) {
                        levelManager.setCurrentLevel(levelManager.currentLevel + 1);
                        LevelInfo& level = levelManager.getCurrentLevelInfo();

                        obstacleManager = ObstacleManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);
                        scoreManager = ScoreManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);
                        powerUpManager = PowerUpManager(GROUND_Y, level.obstacleSpeed, SCREEN_WIDTH);

                        comboSystem.reset(); difficultyManager.reset(); questSystem.resetSessionStats();
                        player.x = 50; player.y = GROUND_Y; player.vy = 0; player.isOnGround = true;
                        gameOver = false; state = GameState::PLAYING;
                    }
                } else if (e.key.keysym.sym == SDLK_m) {
                    saveCallback(); state = GameState::LEVEL_SELECT;
                }
            }
        }

        if (state == GameState::PLAYING && !gameOver) {
            if (!player.isOnGround) player.vy += player.gravity;
            player.y += player.vy;
            if (player.y >= GROUND_Y) { player.y = GROUND_Y; player.vy = 0; player.isOnGround = true; }

            difficultyManager.update();
            obstacleManager.setSpeed(difficultyManager.getSpeed());
            obstacleManager.update();
            scoreManager.setSpeed(difficultyManager.getSpeed());
            scoreManager.update(player);

            questSystem.onScoreUpdate(scoreManager.getCurrentScore());
            for (auto& coin : scoreManager.coins) {
                if (!coin.collected && coin.checkCollision(player.x, player.y, player.width, player.height)) {
                    comboSystem.addCombo(); questSystem.onCoinCollected();
                    // Di chuyển logic thu thập vào trong ScoreManager::update
                }
            }

            powerUpManager.update(player);
            for (auto& pu : powerUpManager.powerUps) {
                if (!pu.collected && pu.checkCollision(player.x, player.y, player.width, player.height)) {
                    questSystem.onPowerupCollected();
                    // Logic kích hoạt powerup đã nằm trong PowerUpManager::update
                }
            }

            comboSystem.update();
            questSystem.onComboUpdate(comboSystem.currentCombo);
            questSystem.onSurvivalTimeUpdate();
            questSystem.updateQuests();
            achievementSystem.update();

            achievementSystem.checkAchievements(scoreManager.getCurrentScore(), player.totalCoins, comboSystem.getMaxCombo(), levelManager.currentLevel + 1);

            if (levelManager.isLevelComplete(scoreManager.getCurrentScore())) {
                levelManager.updateBestScore(scoreManager.getCurrentScore());
                levelManager.unlockNextLevel();
                player.addXp(50);
                player.totalCoins += achievementSystem.getTotalRewardsEarned();
                achievementSystem.clearSessionRewards();
                questSystem.onLevelComplete();
                saveCallback();
                state = GameState::LEVEL_COMPLETE;
            }

            if (obstacleManager.checkCollisionWithPlayer(player.x, player.y, player.width, player.height) && !powerUpManager.shieldActive) {
                gameOver = true;
                questSystem.onDamageTaken();
                levelManager.updateBestScore(scoreManager.getCurrentScore());
                player.totalCoins += achievementSystem.getTotalRewardsEarned();
                achievementSystem.clearSessionRewards();
                saveCallback();
                state = GameState::GAME_OVER;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
        SDL_RenderClear(renderer);

        if (state == GameState::MENU) {
            SDL_Color white = { 255, 255, 255, 255 }, black = { 0, 0, 0, 255 };
            renderCenteredText(renderer, fontBig, "DINO ADVENTURE", black, 50, SCREEN_WIDTH);
            int mx, my; SDL_GetMouseState(&mx, &my);
            bool hovPlay = (mx >= playBtn.x && mx <= playBtn.x + playBtn.w && my >= playBtn.y && my <= playBtn.y + playBtn.h);
            SDL_SetRenderDrawColor(renderer, hovPlay ? 80 : 50, hovPlay ? 160 : 100, 50, 255); SDL_RenderFillRect(renderer, &playBtn);
            SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &playBtn);
            renderCenteredText(renderer, fontSmall, "PLAY", white, playBtn.y + 15, SCREEN_WIDTH);

            bool hovShop = (mx >= shopBtn.x && mx <= shopBtn.x + shopBtn.w && my >= shopBtn.y && my <= shopBtn.y + shopBtn.h);
            SDL_SetRenderDrawColor(renderer, hovShop ? 160:100, hovShop ? 160:100, 50, 255); SDL_RenderFillRect(renderer, &shopBtn);
            SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &shopBtn);
            renderCenteredText(renderer, fontSmall, "SHOP", white, shopBtn.y + 15, SCREEN_WIDTH);

            bool hovQuest = (mx >= questBtn.x && mx <= questBtn.x + questBtn.w && my >= questBtn.y && my <= questBtn.y + questBtn.h);
            SDL_SetRenderDrawColor(renderer, hovQuest ? 100:70, hovQuest ? 180:140, hovQuest ? 100:70, 255); SDL_RenderFillRect(renderer, &questBtn);
            SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &questBtn);
            renderCenteredText(renderer, fontSmall, "QUESTS", white, questBtn.y + 15, SCREEN_WIDTH);

            bool hovQuit = (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w && my >= quitBtn.y && my <= quitBtn.y + quitBtn.h);
            SDL_SetRenderDrawColor(renderer, hovQuit ? 160 : 100, 50, 50, 255); SDL_RenderFillRect(renderer, &quitBtn);
            SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &quitBtn);
            renderCenteredText(renderer, fontSmall, "QUIT", white, quitBtn.y + 15, SCREEN_WIDTH);
        }
        else if (state == GameState::SHOP) {
            SDL_SetRenderDrawColor(renderer, 230, 230, 250, 255); SDL_RenderClear(renderer);
            shop.render(renderer, fontBig, fontSmall, fontTiny, player);
        }
        else if (state == GameState::QUEST) {
            SDL_SetRenderDrawColor(renderer, 220, 240, 220, 255); SDL_RenderClear(renderer);
            questScreen.render(renderer, fontBig, fontSmall, fontTiny, questSystem, player);
        }
        else if (state == GameState::LEVEL_SELECT) {
            SDL_Color black = {0,0,0,255}, gray = {128,128,128,255}, green = {0,200,0,255};
            renderCenteredText(renderer, fontBig, "SELECT LEVEL", black, 30, SCREEN_WIDTH);
            int mx, my; SDL_GetMouseState(&mx, &my);
            for (size_t i = 0; i < levelManager.levels.size(); i++) {
                LevelInfo& level = levelManager.levels[i];
                int btnX = 50 + (i % 3) * 250, btnY = 120 + (i / 3) * 100;
                SDL_Rect levelBtn = { btnX, btnY, 200, 80 };
                bool hovered = (mx >= levelBtn.x && mx <= levelBtn.x + levelBtn.w && my >= levelBtn.y && my <= levelBtn.y + levelBtn.h);
                SDL_SetRenderDrawColor(renderer, level.unlocked ? (hovered ? 100:50) : 80, level.unlocked ? (hovered ? 180:150) : 80, level.unlocked ? 50:80, 255);
                SDL_RenderFillRect(renderer, &levelBtn);
                SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &levelBtn);
                renderLeftText(renderer, fontSmall, "Level " + std::to_string(level.levelNumber), level.unlocked ? black:gray, btnX + 10, btnY + 10);
                if (level.unlocked) {
                    renderLeftText(renderer, fontTiny, level.name, black, btnX+10, btnY+40);
                    renderLeftText(renderer, fontTiny, "Goal: " + std::to_string(level.targetScore), green, btnX+10, btnY+60);
                } else {
                    renderLeftText(renderer, fontTiny, "LOCKED", gray, btnX+50, btnY+45);
                }
            }
            SDL_Rect backBtnLS = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 70, 200, 50 };
            bool hovBackLS = (mx >= backBtnLS.x && mx <= backBtnLS.x + backBtnLS.w && my >= backBtnLS.y && my <= backBtnLS.y + backBtnLS.h);
            SDL_SetRenderDrawColor(renderer, hovBackLS ? 150 : 100, 50, 50, 255); SDL_RenderFillRect(renderer, &backBtnLS);
            SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &backBtnLS);
            renderCenteredText(renderer, fontSmall, "BACK", {255,255,255,255}, backBtnLS.y + 10, SCREEN_WIDTH);
        }
        else if (state == GameState::PLAYING || state == GameState::GAME_OVER || state == GameState::LEVEL_COMPLETE) {
            SDL_SetRenderDrawColor(renderer, 10, 139, 34, 255);
            SDL_Rect ground = { 0, GROUND_Y, SCREEN_WIDTH, SCREEN_HEIGHT - GROUND_Y };
            SDL_RenderFillRect(renderer, &ground);

            obstacleManager.render(renderer);
            scoreManager.render(renderer);
            powerUpManager.render(renderer);

            SDL_Texture* skin = shop.items[player.equippedSkinIndex].texture;
            if (skin) {
                SDL_Rect rect = { player.x, player.y - (int)player.height, (int)player.width, (int)player.height };
                SDL_RenderCopy(renderer, skin, nullptr, &rect);
            }

            SDL_Color white = {255,255,255,255}, yellow = {255,255,0,255}, black = {0,0,0,255};
            LevelInfo& level = levelManager.getCurrentLevelInfo();
            renderLeftText(renderer, fontSmall, "Level " + std::to_string(level.levelNumber) + ": " + level.name, black, 10, 10);
            renderLeftText(renderer, fontSmall, "Score: " + std::to_string(scoreManager.getCurrentScore()) + " / " + std::to_string(level.targetScore), black, 10, 40);
            renderLeftText(renderer, fontTiny, "Coins: " + std::to_string(player.totalCoins), yellow, SCREEN_WIDTH - 150, 10);
            renderLeftText(renderer, fontTiny, "Lvl " + std::to_string(player.level) + " (XP: " + std::to_string(player.xp) + "/" + std::to_string(player.xpToNextLevel) + ")", white, SCREEN_WIDTH - 150, 30);

            comboSystem.render(renderer, fontSmall, SCREEN_WIDTH);
            achievementSystem.render(renderer, fontMedium, fontSmall, SCREEN_WIDTH, SCREEN_HEIGHT);
            questSystem.renderNotification(renderer, fontSmall, SCREEN_WIDTH);

            if (state == GameState::GAME_OVER) {
                renderCenteredText(renderer, fontBig, "GAME OVER", white, 120, SCREEN_WIDTH);
                renderCenteredText(renderer, fontMedium, "Final Score: " + std::to_string(scoreManager.getCurrentScore()), yellow, 190, SCREEN_WIDTH);
                renderCenteredText(renderer, fontSmall, "Max Combo: x" + std::to_string(comboSystem.getMaxCombo()), white, 230, SCREEN_WIDTH);
                renderCenteredText(renderer, fontSmall, "PRESS R TO RETRY OR M FOR MENU", white, 280, SCREEN_WIDTH);
            }
            else if (state == GameState::LEVEL_COMPLETE) {
                 SDL_Color green = { 0, 255, 0, 255 }, gold = {255, 215, 0, 255};
                renderCenteredText(renderer, fontBig, "LEVEL COMPLETE!", green, 80, SCREEN_WIDTH);
                renderCenteredText(renderer, fontMedium, "Final Score: " + std::to_string(scoreManager.getCurrentScore()), gold, 190, SCREEN_WIDTH);
                 if (static_cast<size_t>(levelManager.currentLevel) < levelManager.levels.size() - 1) {
                    renderCenteredText(renderer, fontSmall, "Press N for NEXT LEVEL or M for MENU", white, 310, SCREEN_WIDTH);
                } else {
                    renderCenteredText(renderer, fontSmall, "Congratulations! All levels complete!", gold, 270, SCREEN_WIDTH);
                    renderCenteredText(renderer, fontSmall, "Press M for MENU", white, 310, SCREEN_WIDTH);
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    saveCallback();
    shop.cleanup();
    TTF_CloseFont(fontBig); TTF_CloseFont(fontMedium); TTF_CloseFont(fontSmall); TTF_CloseFont(fontTiny);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit(); TTF_Quit(); SDL_Quit();
    return 0;
}

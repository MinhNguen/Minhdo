#ifndef COMBO_ACHIEVEMENT_H_INCLUDED
#define COMBO_ACHIEVEMENT_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <fstream>

// ===================== Combo System =====================
class ComboSystem {
public:
    int currentCombo, maxCombo, comboTimer, comboTimeout, comboPopTimer;
    float comboMultiplier, comboScale;

    ComboSystem() { reset(); comboTimeout = 120; }

    void reset() {
        currentCombo = 0; maxCombo = 0; comboTimer = 0;
        comboMultiplier = 1.0f; comboPopTimer = 0; comboScale = 1.0f;
    }

    void addCombo() {
        currentCombo++;
        comboTimer = comboTimeout;
        if (currentCombo > maxCombo) maxCombo = currentCombo;
        comboMultiplier = 1.0f + (currentCombo / 5) * 0.5f;
        comboPopTimer = 30; comboScale = 1.5f;
    }

    void update() {
        if (currentCombo > 0 && --comboTimer <= 0) {
            currentCombo = 0; comboMultiplier = 1.0f;
        }
        if (comboPopTimer > 0) {
            comboPopTimer--;
            comboScale = 1.5f - (30 - comboPopTimer) * 0.017f;
            if (comboScale < 1.0f) comboScale = 1.0f;
        }
    }

    void render(SDL_Renderer* renderer, TTF_Font* font, int screenWidth) {
        if (currentCombo < 3) return;
        SDL_Color color = (currentCombo < 10) ? SDL_Color{255, 255, 0, 255} : (currentCombo < 20) ? SDL_Color{255, 140, 0, 255} : SDL_Color{255, 50, 50, 255};
        std::string comboText = "COMBO x" + std::to_string(currentCombo);
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, comboText.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            int w = (int)(surface->w * comboScale), h = (int)(surface->h * comboScale);
            SDL_Rect dst = {screenWidth / 2 - w / 2, 100, w, h};
            SDL_RenderCopy(renderer, texture, nullptr, &dst);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
        }
    }

    int getMaxCombo() const { return maxCombo; }
};

// ===================== Achievement System =====================
struct Achievement {
    int id;
    std::string name, description;
    bool unlocked;
    bool rewardClaimed;
    int reward;
    enum Type { SCORE, COINS, COMBO, LEVEL } type;
    int requirement;
    int currentProgress;

    // Constructor để cho phép khởi tạo bằng danh sách khởi tạo tường minh
    Achievement(int qid, const std::string& n, const std::string& desc, int coins, Type qtype, int req)
        : id(qid), name(n), description(desc), unlocked(false), rewardClaimed(false),
          reward(coins), type(qtype), requirement(req), currentProgress(0) {}

    // Constructor mặc định (cần thiết cho std::vector)
    Achievement() : id(0), name(""), description(""), unlocked(false), rewardClaimed(false),
          reward(0), type(SCORE), requirement(0), currentProgress(0) {}
};

class AchievementSystem {
public:
    std::vector<Achievement> achievements;
    std::vector<int> unlockedThisSession;
    int notificationTimer, currentNotification;

    AchievementSystem() {
        notificationTimer = 0; currentNotification = -1;
        initializeAchievements();
        loadProgress();
    }

    void initializeAchievements() {
        achievements = {
            {0, "First Steps", "Score 50 points", 20, Achievement::SCORE, 50},
            {1, "Century", "Score 100 points", 50, Achievement::SCORE, 100},
            {2, "High Scorer", "Score 200 points", 100, Achievement::SCORE, 200},
            {4, "Coin Collector", "Collect 50 coins", 30, Achievement::COINS, 50},
            {7, "Combo Starter", "Reach 10x combo", 50, Achievement::COMBO, 10},
            {10, "Explorer", "Complete Level 2", 75, Achievement::LEVEL, 2},
        };
    }

    bool isRewardClaimed(int achievementId) {
        for (const auto& ach : achievements) {
            if (ach.id == achievementId) {
                return ach.rewardClaimed;
            }
        }
        return false;
    }

    void checkAchievements(int score, int coins, int maxCombo, int levelCompleted) {
        for (auto& ach : achievements) {
            if (!ach.unlocked) {
                bool shouldUnlock = false;
                if (ach.type == Achievement::SCORE) shouldUnlock = (score >= ach.requirement);
                else if (ach.type == Achievement::COINS) shouldUnlock = (coins >= ach.requirement);
                else if (ach.type == Achievement::COMBO) shouldUnlock = (maxCombo >= ach.requirement);
                else if (ach.type == Achievement::LEVEL) shouldUnlock = (levelCompleted >= ach.requirement);
                if (shouldUnlock) {
                    ach.unlocked = true;
                    unlockedThisSession.push_back(ach.id);
                    notificationTimer = 180;
                    currentNotification = ach.id;
                }
            }
        }
    }

    void update() { if (notificationTimer > 0) notificationTimer--; }

    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall, int screenWidth, int screenHeight) {
        if (notificationTimer > 0 && currentNotification != -1) {
             Achievement* ach = nullptr;
             for(auto& a : achievements) if(a.id == currentNotification) ach = &a;
             if(!ach) return;

            int boxW = 400, boxH = 100, boxX = screenWidth/2-boxW/2, boxY = 50;
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 240);
            SDL_Rect bg = {boxX, boxY, boxW, boxH}; SDL_RenderFillRect(renderer, &bg);
            // Text rendering here
        }
    }

    void saveProgress() {
        std::ofstream file("achievements.dat");
        if (file.is_open()) { for (const auto& ach : achievements) file << ach.unlocked << " "; file.close(); }
    }
    void loadProgress() {
        std::ifstream file("achievements.dat");
        if (file.is_open()) { for (auto& ach : achievements) file >> ach.unlocked; file.close(); }
    }
    int getTotalRewardsEarned() {
        int total = 0;
        for (int id : unlockedThisSession) for(const auto& a : achievements) if(a.id == id) total += a.reward;
        return total;
    }
    void clearSessionRewards() { unlockedThisSession.clear(); }
};

// ===================== Dynamic Difficulty System =====================
class DifficultyManager {
public:
    int survivalTime;
    float baseSpeed, currentSpeed, speedIncrement;
    int speedIncreaseInterval, nextSpeedIncrease;

    DifficultyManager() : baseSpeed(6.0f), speedIncrement(0.5f), speedIncreaseInterval(600) { reset(); }

    void update() {
        survivalTime++;
        if (survivalTime >= nextSpeedIncrease) {
            currentSpeed += speedIncrement;
            nextSpeedIncrease += speedIncreaseInterval;
        }
    }
    void reset() {
        survivalTime = 0;
        currentSpeed = baseSpeed;
        nextSpeedIncrease = speedIncreaseInterval;
    }
    float getSpeed() const { return currentSpeed; }
};

#endif // COMBO_ACHIEVEMENT_H_INCLUDED

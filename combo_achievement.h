#ifndef COMBO_ACHIEVEMENT_H_INCLUDED
#define COMBO_ACHIEVEMENT_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <fstream>
#include "player.h"

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

enum class AchievementTab {
    ALL,
    LOCKED,
    UNLOCKED,
    RARE
};

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
        // [SỬA] Mở rộng đáng kể danh sách thành tích
        achievements = {
            // Loại: Điểm số (SCORE)
            {0, "First Steps", "Score 50 points", 20, Achievement::SCORE, 50},
            {1, "Century", "Score 100 points", 50, Achievement::SCORE, 100},
            {2, "High Scorer", "Score 200 points", 100, Achievement::SCORE, 200},
            {3, "Pro Gamer", "Score 500 points", 250, Achievement::SCORE, 500},
            {100, "Legendary", "Score 1000 points", 500, Achievement::SCORE, 1000}, // Rare (reward >= 500)

            // Loại: Tiền (COINS) - Giả định đây là TỔNG tiền tích lũy
            {4, "Coin Collector", "Collect 50 coins", 30, Achievement::COINS, 50},
            {5, "Getting Rich", "Collect 200 coins", 75, Achievement::COINS, 200},
            {6, "Wealthy", "Collect 500 coins", 150, Achievement::COINS, 500},
            {101, "Millionaire", "Collect 1000 coins", 500, Achievement::COINS, 1000}, // Rare

            // Loại: Combo (COMBO)
            {7, "Combo Starter", "Reach 10x combo", 50, Achievement::COMBO, 10},
            {8, "Combo Master", "Reach 20x combo", 150, Achievement::COMBO, 20},
            {102, "Untouchable", "Reach 30x combo", 500, Achievement::COMBO, 30}, // Rare

            // Loại: Hoàn thành màn chơi (LEVEL) - Giả định đây là TỔNG số màn chơi đã qua
            {10, "Explorer", "Complete Level 2", 75, Achievement::LEVEL, 2},
            {11, "Adventurer", "Complete Level 3", 100, Achievement::LEVEL, 3},
            {12, "Conqueror", "Complete Level 5", 250, Achievement::LEVEL, 5},
            {103, "World Wanderer", "Complete Level 10", 600, Achievement::LEVEL, 10} // Rare
        };
    }

    // [THÊM] HÀM MỚI: Lấy 3 thành tích để hiển thị
    std::vector<Achievement*> getDisplayAchievements(AchievementTab currentTab) {
        std::vector<Achievement*> displayList;

        // Helper lambda 1: Kiểm tra xem có hiếm (rare) không
        auto isRare = [](const Achievement& ach) {
            return ach.reward >= 500; // Định nghĩa "Rare" là thưởng >= 500
        };

        // Helper lambda 2: Kiểm tra xem có khớp với tab đang chọn không
        auto matchesTab = [&](const Achievement& ach) {
            switch(currentTab) {
                case AchievementTab::ALL:      return true;
                case AchievementTab::LOCKED:   return !ach.unlocked;
                case AchievementTab::UNLOCKED: return ach.unlocked && !isRare(ach);
                case AchievementTab::RARE:     return ach.unlocked && isRare(ach);
            }
            return false;
        };

        // Helper lambda 3: Kiểm tra xem đã có trong danh sách hiển thị chưa
        auto isAdded = [&](int achId) {
            for(auto* p : displayList) if(p->id == achId) return true;
            return false;
        };

        // Ưu tiên 1: Đã mở khóa, CHƯA nhận (Claimable)
        for (auto& ach : achievements) {
            if (ach.unlocked && !ach.rewardClaimed && matchesTab(ach)) {
                displayList.push_back(&ach);
                if (displayList.size() >= 3) return displayList;
            }
        }

        // Ưu tiên 2: CHƯA mở khóa (In-Progress)
        for (auto& ach : achievements) {
            if (!ach.unlocked && matchesTab(ach) && !isAdded(ach.id)) {
                displayList.push_back(&ach);
                if (displayList.size() >= 3) return displayList;
            }
        }

        // Ưu tiên 3: Đã mở khóa, ĐÃ nhận (Claimed)
        for (auto& ach : achievements) {
            if (ach.unlocked && ach.rewardClaimed && matchesTab(ach) && !isAdded(ach.id)) {
                displayList.push_back(&ach);
                if (displayList.size() >= 3) return displayList;
            }
        }

        return displayList; // Trả về (có thể ít hơn 3)
    }

    void claimReward(int achievementId, Player& player) {
        for (auto& ach : achievements) {
            if (ach.id == achievementId && ach.unlocked && !ach.rewardClaimed) {
                player.totalCoins += ach.reward;
                player.addXp(ach.reward);
                ach.rewardClaimed = true;
                saveProgress();
                return;
            }
        }
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
            // 1. Cập nhật tiến độ (progress) TRƯỚC
            if (ach.type == Achievement::SCORE) {
                // Dùng std::max để lưu điểm số cao nhất đạt được trong 1 lần chạy
                ach.currentProgress = std::max(ach.currentProgress, score);
            } else if (ach.type == Achievement::COINS) {
                ach.currentProgress = coins; // `coins` là tổng số coin (player.totalCoins), nên gán thẳng
            } else if (ach.type == Achievement::COMBO) {
                ach.currentProgress = std::max(ach.currentProgress, maxCombo); // Lưu combo cao nhất
            } else if (ach.type == Achievement::LEVEL) {
                ach.currentProgress = std::max(ach.currentProgress, levelCompleted); // Lưu màn chơi cao nhất
            }

            // 2. Kiểm tra mở khóa (unlock) SAU
            if (!ach.unlocked && ach.currentProgress >= ach.requirement) {
                ach.unlocked = true;
                unlockedThisSession.push_back(ach.id);
                notificationTimer = 180;
                currentNotification = ach.id;
                saveProgress(); // Lưu ngay khi mở khóa
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
        if (file.is_open()) {
            // Ghi số lượng thành tích để tải dễ dàng hơn
            file << achievements.size() << "\n";
            for (const auto& ach : achievements) {
                // Ghi tất cả dữ liệu trên một dòng, cách nhau bằng dấu cách
                file << ach.id << " "
                     << ach.unlocked << " "
                     << ach.rewardClaimed << " "
                     << ach.currentProgress << "\n";
            }
            file.close();
        }
    }
    void loadProgress() {
        std::ifstream file("achievements.dat");
        if (!file.is_open()) return; // Không có tệp lưu, dùng giá trị mặc định

        int count = 0;
        file >> count; // Đọc số lượng thành tích đã lưu
        if (file.fail() || count == 0) {
            file.close();
            return; // Tệp rỗng hoặc bị lỗi
        }

        for (int i = 0; i < count; ++i) {
            int id;
            bool unlocked;
            bool rewardClaimed;
            int currentProgress;

            // Đọc dữ liệu từ tệp
            file >> id >> unlocked >> rewardClaimed >> currentProgress;
            if (file.fail()) break; // Dừng nếu đọc lỗi

            // Tìm thành tích trong danh sách `achievements` bằng ID và cập nhật
            for (auto& ach : achievements) {
                if (ach.id == id) {
                    ach.unlocked = unlocked;
                    ach.rewardClaimed = rewardClaimed;
                    ach.currentProgress = currentProgress;
                    break; // Đã tìm thấy, chuyển sang dòng tiếp theo
                }
            }
        }
        file.close();
    }
    int getTotalRewardsEarned() {
        int total = 0;
        for (int id : unlockedThisSession) for(const auto& a : achievements) if(a.id == id) total += a.reward;
        return total;
    }
    void clearSessionRewards() { unlockedThisSession.clear(); }
    void resetAchievements() {
        initializeAchievements();
        saveProgress();
    }
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

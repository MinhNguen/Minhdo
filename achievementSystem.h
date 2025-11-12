#ifndef COMBO_ACHIEVEMENT_H_INCLUDED
#define COMBO_ACHIEVEMENT_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <fstream>
#include "player.h"

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

    Achievement(int qid, const std::string& n, const std::string& desc, int coins, Type qtype, int req)
        : id(qid), name(n), description(desc), unlocked(false), rewardClaimed(false),
          reward(coins), type(qtype), requirement(req), currentProgress(0) {}

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
            {3, "Pro Gamer", "Score 500 points", 250, Achievement::SCORE, 500},
            {100, "Legendary", "Score 1000 points", 500, Achievement::SCORE, 1000},

            {4, "Coin Collector", "Collect 50 coins", 30, Achievement::COINS, 50},
            {5, "Getting Rich", "Collect 200 coins", 75, Achievement::COINS, 200},
            {6, "Wealthy", "Collect 500 coins", 150, Achievement::COINS, 500},
            {101, "Millionaire", "Collect 1000 coins", 500, Achievement::COINS, 1000},

            {7, "Combo Starter", "Reach 10x combo", 50, Achievement::COMBO, 10},
            {8, "Combo Master", "Reach 20x combo", 150, Achievement::COMBO, 20},
            {102, "Untouchable", "Reach 30x combo", 500, Achievement::COMBO, 30},

            {10, "Explorer", "Complete Level 2", 75, Achievement::LEVEL, 2},
            {11, "Adventurer", "Complete Level 3", 100, Achievement::LEVEL, 3},
            {12, "Conqueror", "Complete Level 5", 250, Achievement::LEVEL, 5},
            {103, "World Wanderer", "Complete Level 10", 600, Achievement::LEVEL, 10}
        };
    }

    std::vector<Achievement*> getDisplayAchievements(AchievementTab currentTab) {
        std::vector<Achievement*> displayList;
        auto isRare = [](const Achievement& ach) {
            return ach.reward >= 500;
        };
        auto matchesTab = [&](const Achievement& ach) {
            switch(currentTab) {
                case AchievementTab::ALL:      return true;
                case AchievementTab::LOCKED:   return !ach.unlocked;
                case AchievementTab::UNLOCKED: return ach.unlocked && !isRare(ach);
                case AchievementTab::RARE:     return ach.unlocked && isRare(ach);
            }
            return false;
        };


        auto isAdded = [&](int achId) {
            for(auto* p : displayList) if(p->id == achId) return true;
            return false;
        };


        for (auto& ach : achievements) {
            if (ach.unlocked && !ach.rewardClaimed && matchesTab(ach)) {
                displayList.push_back(&ach);
                if (displayList.size() >= 3) return displayList;
            }
        }

        for (auto& ach : achievements) {
            if (!ach.unlocked && matchesTab(ach) && !isAdded(ach.id)) {
                displayList.push_back(&ach);
                if (displayList.size() >= 3) return displayList;
            }
        }


        for (auto& ach : achievements) {
            if (ach.unlocked && ach.rewardClaimed && matchesTab(ach) && !isAdded(ach.id)) {
                displayList.push_back(&ach);
                if (displayList.size() >= 3) return displayList;
            }
        }

        return displayList;
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
            if (ach.type == Achievement::SCORE) {
                ach.currentProgress = std::max(ach.currentProgress, score);
            } else if (ach.type == Achievement::COINS) {
                ach.currentProgress = coins;
            } else if (ach.type == Achievement::COMBO) {
                ach.currentProgress = std::max(ach.currentProgress, maxCombo);
            } else if (ach.type == Achievement::LEVEL) {
                ach.currentProgress = std::max(ach.currentProgress, levelCompleted);
            }

            if (!ach.unlocked && ach.currentProgress >= ach.requirement) {
                ach.unlocked = true;
                unlockedThisSession.push_back(ach.id);
                notificationTimer = 180;
                currentNotification = ach.id;
                saveProgress();
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
        }
    }

    void saveProgress() {
        std::ofstream file("achievements.dat");
        if (file.is_open()) {
            file << achievements.size() << "\n";
            for (const auto& ach : achievements) {
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
        if (!file.is_open()) return;

        int count = 0;
        file >> count;
        if (file.fail() || count == 0) {
            file.close();
            return;
        }

        for (int i = 0; i < count; ++i) {
            int id;
            bool unlocked;
            bool rewardClaimed;
            int currentProgress;

            file >> id >> unlocked >> rewardClaimed >> currentProgress;
            if (file.fail()) break;
            for (auto& ach : achievements) {
                if (ach.id == id) {
                    ach.unlocked = unlocked;
                    ach.rewardClaimed = rewardClaimed;
                    ach.currentProgress = currentProgress;
                    break;
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


#endif // COMBO_ACHIEVEMENT_H_INCLUDED

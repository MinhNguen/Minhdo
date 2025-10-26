#ifndef QUEST_SYSTEM_H_INCLUDED
#define QUEST_SYSTEM_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <fstream>

struct Quest {
    int id;
    std::string title, description;
    bool isCompleted, isActive, rewardClaimed;
    enum Type { COLLECT_COINS, REACH_SCORE, COMPLETE_LEVEL, COLLECT_POWERUPS, REACH_COMBO, SURVIVE_TIME, JUMP_COUNT, NO_DAMAGE } type;
    int requirement, currentProgress;
    int coinReward, xpReward;

    Quest(int qid, const std::string& t, const std::string& desc, Type qtype, int req, int coins, int xp)
        : id(qid), title(t), description(desc), isCompleted(false), isActive(false), rewardClaimed(false),
          type(qtype), requirement(req), currentProgress(0), coinReward(coins), xpReward(xp) {}

    float getProgressPercent() const { return requirement == 0 ? 0.0f : (float)currentProgress / requirement; }
    bool checkCompletion() { if (!isCompleted && currentProgress >= requirement) isCompleted = true; return isCompleted; }
};

class QuestSystem {
public:
    std::vector<Quest> dailyQuests, mainQuests;
    int notificationTimer;
    std::string notificationText;
    int sessionCoinsCollected, sessionScore, sessionPowerupsCollected, sessionMaxCombo, sessionSurvivalTime, sessionJumpCount, sessionLevelsCompleted;
    bool sessionNoDamage;

    QuestSystem() {
        notificationTimer = 0;
        initializeQuests();
        resetSessionStats();
        loadProgress();
    }

    void resetSessionStats() {
        sessionCoinsCollected = 0; sessionScore = 0; sessionPowerupsCollected = 0; sessionMaxCombo = 0;
        sessionSurvivalTime = 0; sessionJumpCount = 0; sessionNoDamage = true; sessionLevelsCompleted = 0;
    }

    void initializeQuests() {
        dailyQuests = { Quest(0, "Coin Collector", "Collect 50 coins", Quest::COLLECT_COINS, 50, 50, 25), Quest(1, "Score Hunter", "Reach 100 points", Quest::REACH_SCORE, 100, 40, 20) };
        mainQuests = { Quest(100, "First Steps", "Complete Level 1", Quest::COMPLETE_LEVEL, 1, 75, 50), Quest(101, "Getting Stronger", "Collect 200 coins total", Quest::COLLECT_COINS, 200, 150, 75) };
    }

    void updateQuests() {
        for (auto& quest : dailyQuests) if (quest.isActive && !quest.isCompleted) updateQuestProgress(quest);
        for (auto& quest : mainQuests) if (quest.isActive && !quest.isCompleted) updateQuestProgress(quest);
        if (notificationTimer > 0) notificationTimer--;
    }

    void updateQuestProgress(Quest& quest) {
        switch (quest.type) {
            case Quest::COLLECT_COINS: quest.currentProgress = sessionCoinsCollected; break;
            case Quest::REACH_SCORE: quest.currentProgress = sessionScore; break;
            case Quest::COLLECT_POWERUPS: quest.currentProgress = sessionPowerupsCollected; break;
            case Quest::REACH_COMBO: quest.currentProgress = sessionMaxCombo; break;
            case Quest::SURVIVE_TIME: quest.currentProgress = sessionSurvivalTime; break;
            case Quest::JUMP_COUNT: quest.currentProgress = sessionJumpCount; break;
            case Quest::COMPLETE_LEVEL: quest.currentProgress = sessionLevelsCompleted; break;
            case Quest::NO_DAMAGE: quest.currentProgress = sessionNoDamage ? 1 : 0; break;
        }
        if (quest.checkCompletion() && !quest.rewardClaimed) showNotification("Quest Completed: " + quest.title);
    }

    void activateQuest(int questId, bool isDaily) {
        auto& questList = isDaily ? dailyQuests : mainQuests;
        for (auto& quest : questList) if (quest.id == questId) { quest.isActive = true; break; }
    }

    void showNotification(const std::string& text) { notificationText = text; notificationTimer = 180; }

    void onCoinCollected() { sessionCoinsCollected++; }
    void onScoreUpdate(int score) { sessionScore = score; }
    void onPowerupCollected() { sessionPowerupsCollected++; }
    void onComboUpdate(int combo) { if (combo > sessionMaxCombo) sessionMaxCombo = combo; }
    void onSurvivalTimeUpdate() { sessionSurvivalTime++; }
    void onJump() { sessionJumpCount++; }
    void onDamageTaken() { sessionNoDamage = false; }
    void onLevelComplete() { sessionLevelsCompleted++; }

    void renderNotification(SDL_Renderer* renderer, TTF_Font* font, int screenWidth) {
        if (notificationTimer > 0) {
            // Render logic
        }
    }

    void resetDailyQuests() {
        // Tái tạo lại danh sách Daily Quests ban đầu và reset trạng thái
        dailyQuests = {
            Quest(0, "Coin Collector", "Collect 50 coins", Quest::COLLECT_COINS, 50, 50, 25),
            Quest(1, "Score Hunter", "Reach 100 points", Quest::REACH_SCORE, 100, 40, 20)
        };
        for (auto& quest : dailyQuests) {
             // Reset trạng thái
             quest.isActive = false;
             quest.isCompleted = false;
             quest.rewardClaimed = false;
             quest.currentProgress = 0;
        }
        // Ghi chú: mainQuests là nhiệm vụ chính nên không bị reset hàng ngày.
        std::cout << "Daily Quests have been reset." << std::endl;
    }

    void saveProgress() {
        std::ofstream file("quests.dat");
        if (!file.is_open()) return;
        file << dailyQuests.size() << "\n";
        for (const auto& q : dailyQuests) file << q.id << " " << q.isActive << " " << q.isCompleted << " " << q.rewardClaimed << " " << q.currentProgress << "\n";
        file << mainQuests.size() << "\n";
        for (const auto& q : mainQuests) file << q.id << " " << q.isActive << " " << q.isCompleted << " " << q.rewardClaimed << " " << q.currentProgress << "\n";
        file.close();
    }

    void loadProgress() {
        std::ifstream file("quests.dat");
        if (!file.is_open()) return;
        int count;
        file >> count;
        for (int i = 0; i < count; ++i) { int id; file >> id; for (auto& q : dailyQuests) if (q.id == id) file >> q.isActive >> q.isCompleted >> q.rewardClaimed >> q.currentProgress; }
        file >> count;
        for (int i = 0; i < count; ++i) { int id; file >> id; for (auto& q : mainQuests) if (q.id == id) file >> q.isActive >> q.isCompleted >> q.rewardClaimed >> q.currentProgress; }
        file.close();
    }

    int getActiveQuestCount() {
        int count = 0;
        for(const auto& q : dailyQuests) if(q.isActive && !q.isCompleted) count++;
        for(const auto& q : mainQuests) if(q.isActive && !q.isCompleted) count++;
        return count;
    }

    int getCompletedQuestCount() {
        int count = 0;
        for(const auto& q : dailyQuests) if(q.isCompleted) count++;
        for(const auto& q : mainQuests) if(q.isCompleted) count++;
        return count;
    }
};

#endif // QUEST_SYSTEM_H_INCLUDED

#ifndef QUEST_SYSTEM_H_INCLUDED
#define QUEST_SYSTEM_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include "player.h"

struct Quest {
    int id;
    std::string title, description;
    bool isCompleted, isActive, rewardClaimed;
    enum Type { COLLECT_COINS, REACH_SCORE, COMPLETE_LEVEL, COLLECT_POWERUPS, REACH_COMBO, SURVIVE_TIME, JUMP_COUNT, NO_DAMAGE } type;
    int requirement, currentProgress;
    int coinReward, xpReward;
    bool isAccumulative;

    Quest(int qid, const std::string& t, const std::string& desc, Type qtype, int req, int coins, int xp, bool accumulative = false)
        : id(qid), title(t), description(desc), isCompleted(false), isActive(false), rewardClaimed(false),
          type(qtype), requirement(req), currentProgress(0), coinReward(coins), xpReward(xp),
          isAccumulative(accumulative) {}

    float getProgressPercent() const { return requirement == 0 ? 0.0f : (float)currentProgress / requirement; }
    bool checkCompletion() { if (!isCompleted && currentProgress >= requirement) isCompleted = true; return isCompleted; }
};

class QuestSystem {
public:
    std::vector<Quest> dailyQuests, mainQuests;
    std::vector<Quest> dailyQuestPool;
    std::vector<Quest> mainQuestPool;

    int notificationTimer;
    std::string notificationText;
    int sessionCoinsCollected, sessionScore, sessionPowerupsCollected, sessionMaxCombo, sessionSurvivalTime, sessionJumpCount, sessionLevelsCompleted;
    bool sessionNoDamage;

    QuestSystem() {
        notificationTimer = 0;
        initializeQuests();
        resetSessionStats();

        resetMainQuests(true);
        resetDailyQuests(true);

        loadProgress();
    }

    void resetSessionStats() {
        sessionCoinsCollected = 0; sessionScore = 0; sessionPowerupsCollected = 0; sessionMaxCombo = 0;
        sessionSurvivalTime = 0; sessionJumpCount = 0; sessionNoDamage = true; sessionLevelsCompleted = 0;
    }

    void initializeQuests() {
        dailyQuestPool = {
            Quest(0, "Thu Thập Nhanh", "Thu thập 50 xu trong 1 lần chạy", Quest::COLLECT_COINS, 50, 50, 25, false),
            Quest(1, "Thợ Săn Điểm", "Đạt 100 điểm trong 1 lần chạy", Quest::REACH_SCORE, 100, 40, 20, false),
            Quest(2, "Nhảy Nhót", "Nhảy 20 lần trong 1 lần chạy", Quest::JUMP_COUNT, 20, 30, 15, false),
            Quest(3, "Tăng Lực", "Thu thập 3 vật phẩm hỗ trợ", Quest::COLLECT_POWERUPS, 3, 60, 30, false),
            Quest(4, "Combo Ngắn", "Đạt 5x combo", Quest::REACH_COMBO, 5, 50, 25, false),
            Quest(5, "Sinh Tồn", "Sống sót 1 phút (60s) trong 1 lần chạy", Quest::SURVIVE_TIME, 1, 40, 20, false),
            Quest(6, "Tay Săn Xu", "Thu thập 100 xu trong 1 lần chạy", Quest::COLLECT_COINS, 100, 100, 50, false),
            Quest(7, "Chuyên Gia Né Tránh", "Đạt 150 điểm mà không nhận sát thương", Quest::NO_DAMAGE, 150, 150, 75, false),
            Quest(8, "Vua Combo", "Đạt 10x combo", Quest::REACH_COMBO, 10, 100, 50, false),
            Quest(9, "Marathon Mini", "Sống sót 2 phút (120s)", Quest::SURVIVE_TIME, 2, 80, 40, false)
        };

        // [SỬA] Mở rộng Main Quest Pool (tất cả đều là 'true' - tích lũy)
        mainQuestPool = {
            Quest(100, "Bước Đầu Tiên", "Hoàn thành 1 màn chơi (bất kỳ)", Quest::COMPLETE_LEVEL, 1, 75, 50, true),
            Quest(101, "Giàu Có", "Thu thập tổng cộng 200 xu", Quest::COLLECT_COINS, 200, 150, 75, true),
            Quest(102, "Chuyên Gia", "Hoàn thành tổng cộng 5 màn chơi", Quest::COMPLETE_LEVEL, 5, 200, 100, true),
            Quest(103, "Triệu Phú Xu", "Thu thập tổng cộng 1000 xu", Quest::COLLECT_COINS, 1000, 500, 250, true),
            // [THÊM]
            Quest(104, "Nhà Sưu Tầm", "Thu thập tổng cộng 50 vật phẩm", Quest::COLLECT_POWERUPS, 50, 200, 100, true),
            Quest(105, "Bậc Thầy Combo", "Đạt 20x combo (cao nhất)", Quest::REACH_COMBO, 20, 250, 150, true),
            Quest(106, "Người Du Hành", "Hoàn thành tổng cộng 10 màn chơi", Quest::COMPLETE_LEVEL, 10, 400, 200, true),
            Quest(107, "Kho Bạc", "Thu thập tổng cộng 5000 xu", Quest::COLLECT_COINS, 5000, 1000, 500, true)
        };
    }

    void updateQuests(Player& player) {
        for (auto& quest : dailyQuests) if (!quest.isCompleted) updateQuestProgress(quest, player);
        for (auto& quest : mainQuests) if (!quest.isCompleted) updateQuestProgress(quest, player);

        if (notificationTimer > 0) notificationTimer--;
    }

    void updateQuestProgress(Quest& quest, Player& player) {
        switch (quest.type) {
            case Quest::COLLECT_COINS:
                quest.currentProgress = quest.isAccumulative ? player.totalCoins : std::max(quest.currentProgress, sessionCoinsCollected);
                break;
            case Quest::REACH_SCORE:
                quest.currentProgress = std::max(quest.currentProgress, sessionScore);
                break;
            case Quest::COLLECT_POWERUPS:
                quest.currentProgress = quest.isAccumulative ? player.totalPowerupsCollected : std::max(quest.currentProgress, sessionPowerupsCollected);
                break;
            case Quest::REACH_COMBO:
                quest.currentProgress = quest.isAccumulative ? player.bestComboAchieved : std::max(quest.currentProgress, sessionMaxCombo);
                break;
            case Quest::SURVIVE_TIME:
                quest.currentProgress = std::max(quest.currentProgress, sessionSurvivalTime / 60);
                break;
            case Quest::JUMP_COUNT:
                quest.currentProgress = std::max(quest.currentProgress, sessionJumpCount);
                break;
            case Quest::COMPLETE_LEVEL:
                quest.currentProgress = quest.isAccumulative ? player.totalLevelsCompleted : std::max(quest.currentProgress, sessionLevelsCompleted);
                break;
            case Quest::NO_DAMAGE:
                if (!sessionNoDamage) {
                    quest.currentProgress = 0;
                } else {
                    quest.currentProgress = sessionScore;
                }
                break;
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

    void resetDailyQuests(bool isInitialLoad = false) {
        if (dailyQuestPool.empty()) return; // Không có gì để reset

        // 1. Lưu tiến độ của các nhiệm vụ hàng ngày cũ trước khi xóa
        if (!isInitialLoad) {
            saveProgress();
        }

        dailyQuests.clear();

        // 2. Thiết lập bộ sinh số ngẫu nhiên
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 g(seed);

        // 3. Tạo một danh sách các chỉ số (index) và xáo trộn chúng
        std::vector<int> indices(dailyQuestPool.size());
        for (size_t i = 0; i < indices.size(); ++i) indices[i] = i;
        std::shuffle(indices.begin(), indices.end(), g);

        // 4. Chọn 3 nhiệm vụ đầu tiên từ danh sách đã xáo trộn
        int numToSelect = std::min(3, (int)dailyQuestPool.size()); // Chọn 3 hoặc ít hơn nếu pool nhỏ
        for (int i = 0; i < numToSelect; ++i) {
            dailyQuests.push_back(dailyQuestPool[indices[i]]); // Thêm BẢN SAO vào danh sách active
        }

        // 5. Lưu lại danh sách nhiệm vụ mới (rỗng)
        if (!isInitialLoad) {
            saveProgress();
            std::cout << "Daily Quests have been reset with new random quests." << std::endl;
        }
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
        // Đọc các nhiệm vụ hàng ngày đã lưu
        for (int i = 0; i < count; ++i) {
            int id;
            bool isActive, isCompleted, rewardClaimed;
            int currentProgress;
            file >> id >> isActive >> isCompleted >> rewardClaimed >> currentProgress;
            if (file.fail()) break;
            // Tìm quest trong danh sách active dailyQuests và cập nhật
            for (auto& q : dailyQuests) {
                if (q.id == id) {
                    q.isActive = isActive; q.isCompleted = isCompleted; q.rewardClaimed = rewardClaimed; q.currentProgress = currentProgress;
                    break;
                }
            }
        }

        file >> count;
        // Đọc các nhiệm vụ chính đã lưu
        for (int i = 0; i < count; ++i) {
            int id;
            bool isActive, isCompleted, rewardClaimed;
            int currentProgress;
            file >> id >> isActive >> isCompleted >> rewardClaimed >> currentProgress;
            if (file.fail()) break;
            // Tìm quest trong danh sách active mainQuests và cập nhật
            for (auto& q : mainQuests) {
                if (q.id == id) {
                    q.isActive = isActive; q.isCompleted = isCompleted; q.rewardClaimed = rewardClaimed; q.currentProgress = currentProgress;
                    break;
                }
            }
        }
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

    std::vector<Quest*> getDisplayMainQuests() {
        std::vector<Quest*> displayList;

        // Ưu tiên 1: Thêm các nhiệm vụ đã hoàn thành nhưng CHƯA nhận thưởng
        for (auto& quest : mainQuests) {
            if (quest.isCompleted && !quest.rewardClaimed) {
                displayList.push_back(&quest);
                if (displayList.size() >= 3) return displayList;
            }
        }

        // Ưu tiên 2: Thêm các nhiệm vụ đang tiến hành (chưa hoàn thành)
        for (auto& quest : mainQuests) {
            if (!quest.isCompleted) {
                bool already_added = false;
                for(Quest* q_ptr : displayList) {
                    if (q_ptr->id == quest.id) {
                        already_added = true;
                        break;
                    }
                }

                if (!already_added) {
                     displayList.push_back(&quest);
                    if (displayList.size() >= 3) return displayList;
                }
            }
        }

        return displayList;
    }

    // [CHUẨN] Hàm này reset TOÀN BỘ danh sách
    void resetMainQuests(bool isInitialLoad = false) {
        if (mainQuestPool.empty()) return; // Không có gì để reset

        // 1. Lưu tiến độ của các nhiệm vụ hàng ngày cũ trước khi xóa
        if (!isInitialLoad) {
            saveProgress();
        }

        mainQuests.clear();

        // 2. Thiết lập bộ sinh số ngẫu nhiên
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 g(seed);

        // 3. Tạo một danh sách các chỉ số (index) và xáo trộn chúng
        std::vector<int> indices(mainQuestPool.size());
        for (size_t i = 0; i < indices.size(); ++i) indices[i] = i;
        std::shuffle(indices.begin(), indices.end(), g);

        // 4. Chọn 3 nhiệm vụ đầu tiên từ danh sách đã xáo trộn
        int numToSelect = std::min(3, (int)mainQuestPool.size()); // Chọn 3 hoặc ít hơn nếu pool nhỏ
        for (int i = 0; i < numToSelect; ++i) {
            mainQuests.push_back(mainQuestPool[indices[i]]); // Thêm BẢN SAO vào danh sách active
        }

        // 5. Lưu lại danh sách nhiệm vụ mới (rỗng)
        if (!isInitialLoad) {
            saveProgress();
            std::cout << "Main Quests have been reset with new random quests." << std::endl;
        }
    }
};

#endif // QUEST_SYSTEM_H_INCLUDED

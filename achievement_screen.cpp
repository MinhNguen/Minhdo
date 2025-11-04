#include "achievement_screen.h"
#include <iostream>
#include <sstream> // Cần thiết cho việc định dạng text
#include "player.h"
#include "combo_achievement.h"
// Định nghĩa Constructor của AchievementParticle
AchievementParticle::AchievementParticle(float px, float py) {
    x = px; y = py;
    vx = -2.0f + (rand() % 40) / 10.0f;
    vy = -3.0f - (rand() % 30) / 10.0f;
    lifetime = 60 + rand() % 40;
    maxLifetime = lifetime;
    size = 3.0f + (rand() % 30) / 10.0f;

    int colorChoice = rand() % 3;
    if (colorChoice == 0) color = {255, 215, 0, 255}; // Gold
    else if (colorChoice == 1) color = {255, 140, 0, 255}; // Orange
    else color = {255, 255, 0, 255}; // Yellow
}

// Định nghĩa Phương thức update của AchievementParticle
void AchievementParticle::update() {
    x += vx;
    y += vy;
    vy += 0.1f; // Gravity
    lifetime--;
}

// Định nghĩa Phương thức render của AchievementParticle
void AchievementParticle::render(SDL_Renderer* renderer) {
    if (lifetime > 0) {
        float ratio = (float)lifetime / maxLifetime;
        int alpha = (int)(ratio * 255);

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
        SDL_Rect rect = { (int)x, (int)y, (int)size, (int)size };
        SDL_RenderFillRect(renderer, &rect);
    }
}


// ====================================================================
// TRIỂN KHAI CLASS AchievementScreen
// ====================================================================

// Định nghĩa Constructor
AchievementScreen::AchievementScreen() : currentTab(ALL) {}

// Hàm helper: Render Text (Lấy từ snippet của quest_screen.h để đảm bảo có hàm này)
void AchievementScreen::renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int w = surface->w, h = surface->h;
    SDL_FreeSurface(surface);
    if (!texture) return;
    SDL_Rect dst = { x, y, w, h };
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}
// Hàm helper: Render Centered Text (Lấy từ snippet của quest_screen.h)
void AchievementScreen::renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int y, int screenW) {

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { (screenW - w) / 2, y, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
}

// Định nghĩa Phương thức render
void AchievementScreen::render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontMedium, TTF_Font* fontSmall,
                              int screenW, int screenH, AchievementSystem& achievementSystem, Player& player)
{
    SDL_Color white = {255, 255, 255, 255}, gold = {255, 215, 0, 255}, gray = {150, 150, 150, 255}, green = {0, 200, 0, 255}, red = {200, 0, 0, 255};

    // Background và Tiêu đề
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderFillRect(renderer, NULL);
    renderCenteredText(renderer, fontBig, "ACHIEVEMENTS", gold, 30, screenW);

    // Render tabs
    SDL_Rect allTab = {100, 95, 150, 50};
    SDL_Rect lockedTab = {250, 95, 150, 50};
    SDL_Rect unlockedTab = {400, 95, 200, 50};
    SDL_Rect rareTab = {600, 95, 150, 50};

    SDL_Color activeColor = {0, 120, 255, 255};
    SDL_Color inactiveColor = {50, 50, 50, 255};

    SDL_SetRenderDrawColor(renderer, (currentTab == ALL) ? activeColor.r : inactiveColor.r,
                                     (currentTab == ALL) ? activeColor.g : inactiveColor.g,
                                     (currentTab == ALL) ? activeColor.b : inactiveColor.b, 255);
    SDL_RenderFillRect(renderer, &allTab);
    renderText(renderer, fontMedium, "ALL", white, allTab.x, allTab.y + 5);

    SDL_SetRenderDrawColor(renderer, (currentTab == LOCKED) ? activeColor.r : inactiveColor.r,
                                     (currentTab == LOCKED) ? activeColor.g : inactiveColor.g,
                                     (currentTab == LOCKED) ? activeColor.b : inactiveColor.b, 255);
    SDL_RenderFillRect(renderer, &lockedTab);
    renderText(renderer, fontMedium, "LOCKED", white, lockedTab.x, lockedTab.y + 5);

    SDL_SetRenderDrawColor(renderer, (currentTab == UNLOCKED) ? activeColor.r : inactiveColor.r,
                                     (currentTab == UNLOCKED) ? activeColor.g : inactiveColor.g,
                                     (currentTab == UNLOCKED) ? activeColor.b : inactiveColor.b, 255);
    SDL_RenderFillRect(renderer, &unlockedTab);
    renderText(renderer, fontMedium, "UNLOCKED", white, unlockedTab.x, unlockedTab.y + 5);

    SDL_SetRenderDrawColor(renderer, (currentTab == RARE) ? activeColor.r : inactiveColor.r,
                                     (currentTab == RARE) ? activeColor.g : inactiveColor.g,
                                     (currentTab == RARE) ? activeColor.b : inactiveColor.b, 255);
    SDL_RenderFillRect(renderer, &rareTab);
    renderText(renderer, fontMedium, "RARE", white, rareTab.x, rareTab.y + 5);


    // Lọc danh sách achievement dựa trên tab
    std::vector<Achievement> filteredAchievements;
    for (const auto& ach : achievementSystem.achievements) {
        bool isRare = (ach.reward >= 500); // Giả định điều kiện "Rare"

        if (currentTab == ALL) filteredAchievements.push_back(ach);
        else if (currentTab == LOCKED && !ach.unlocked) filteredAchievements.push_back(ach);
        else if (currentTab == UNLOCKED && ach.unlocked && !isRare) filteredAchievements.push_back(ach);
        else if (currentTab == RARE && ach.unlocked && isRare) filteredAchievements.push_back(ach);
    }

    // Render danh sách
    renderAchievementList(renderer, fontMedium, fontSmall, 150, screenW, filteredAchievements, achievementSystem, player);

    // Nút BACK
    SDL_Rect backBtn = {screenW / 2 - 100, screenH - 70, 200, 50};
    SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255);
    SDL_RenderFillRect(renderer, &backBtn);
    renderCenteredText(renderer, fontMedium, "BACK", white, backBtn.y + 10, screenW);

    SDL_Rect resetBtn = {screenW - 130, screenH - 70, 120, 50};
    SDL_SetRenderDrawColor(renderer, 180, 100, 0, 255); // Màu cam đậm
    SDL_RenderFillRect(renderer, &resetBtn);
    renderCenteredText(renderer, fontSmall, "RESET ALL", white, resetBtn.y + 15, resetBtn.x + resetBtn.w / 2);

    // Render Particles
    for (auto& p : particles) {
        p.render(renderer);
    }
}

// Định nghĩa Phương thức renderAchievementList (Private Helper)
void AchievementScreen::renderAchievementList(SDL_Renderer* renderer, TTF_Font* fontMedium, TTF_Font* fontSmall,
                                             int startY, int screenW, const std::vector<Achievement>& achievements,
                                             AchievementSystem& achievementSystem, Player& player)
{
    SDL_Color white = {255, 255, 255, 255}, gold = {255, 215, 0, 255}, gray = {150, 150, 150, 255};
    int currentY = startY;
    int spacing = 10;
    int achHeight = 60;

    for (const auto& ach : achievements) {
        SDL_Rect rect = {50, currentY, screenW - 100, achHeight};
        SDL_Color bgColor = ach.unlocked ? (ach.reward >= 500 ? (SDL_Color){255, 140, 0, 255} : (SDL_Color){0, 100, 0, 255}) : (SDL_Color){50, 50, 50, 255};

        // Background của achievement
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderFillRect(renderer, &rect);

        // Tên achievement
        renderText(renderer, fontMedium, ach.name, white, rect.x + 10, rect.y + 5);

        // Mô tả/Tiến độ
        std::string progressText = ach.unlocked ? "Unlocked!" : ("Progress: " + std::to_string(ach.currentProgress) + "/" + std::to_string(ach.requirement));
        renderText(renderer, fontSmall, progressText, ach.unlocked ? gold : gray, rect.x + 10, rect.y + 35);
        // Phần thưởng
        std::stringstream ss;
        ss << "+" << ach.reward << " COINS";
        renderText(renderer, fontMedium, ss.str(), gold, rect.x + rect.w - 150, rect.y + 15);

        // Nút Claim (Nếu đã mở khóa và chưa nhận)
        if (ach.unlocked && !achievementSystem.isRewardClaimed(ach.id)) {
            SDL_Rect claimBtn = {rect.x + rect.w - 80, rect.y + 10, 70, 40};
            SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
            SDL_RenderFillRect(renderer, &claimBtn);
            renderCenteredText(renderer, fontSmall, "CLAIM", white, claimBtn.y + 10, claimBtn.x + claimBtn.w / 2);
        } else if (ach.unlocked && achievementSystem.isRewardClaimed(ach.id)) {
            renderCenteredText(renderer, fontSmall, "CLAIMED", gray, rect.y + 25, rect.x + rect.w - 45);
        }

        currentY += achHeight + spacing;
    }
}


// Định nghĩa Phương thức handleInput
bool AchievementScreen::handleInput(SDL_Event& e, int screenW, AchievementSystem& achievementSystem, Player& player) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mx = e.button.x, my = e.button.y;

        // Nút Back
        SDL_Rect backBtn = {screenW / 2 - 100, 530, 200, 50}; // Giả định screenH là 600
        if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w && my >= backBtn.y && my <= backBtn.y + backBtn.h) {
            return true; // Thoát màn hình achievement
        }

        SDL_Rect resetBtn = {screenW - 130, 530, 120, 50};
        if (mx >= resetBtn.x && mx <= resetBtn.x + resetBtn.w && my >= resetBtn.y && my <= resetBtn.y + resetBtn.h) {
            achievementSystem.resetAchievements(); // Gọi hàm reset
            return false; // Không thoát màn hình, chỉ reset
        }

        // Tab clicks
        SDL_Rect allTab = {100, 95, 150, 50};
        SDL_Rect lockedTab = {250, 95, 150, 50};
        SDL_Rect unlockedTab = {400, 95, 150, 50};
        SDL_Rect rareTab = {600, 95, 150, 50};

        if (mx >= allTab.x && mx <= allTab.x + allTab.w && my >= allTab.y && my <= allTab.y + allTab.h)
            currentTab = ALL;
        else if (mx >= lockedTab.x && mx <= lockedTab.x + lockedTab.w && my >= lockedTab.y && my <= lockedTab.y + lockedTab.h)
            currentTab = LOCKED;
        else if (mx >= unlockedTab.x && mx <= unlockedTab.x + unlockedTab.w && my >= unlockedTab.y && my <= unlockedTab.y + unlockedTab.h)
            currentTab = UNLOCKED;
        else if (mx >= rareTab.x && mx <= rareTab.x + rareTab.w && my >= rareTab.y && my <= rareTab.y + rareTab.h)
            currentTab = RARE;

        // Xử lý Claim Reward (Phần này cần truy cập vào Player/QuestSystem/AchievementSystem)
        int startY = 150, achHeight = 60, spacing = 10;
        int currentY = startY;

        // Để đơn giản, ta lặp qua tất cả và kiểm tra vị trí button
        for (auto& ach : achievementSystem.achievements) {
            SDL_Rect rect = {50, currentY, screenW - 100, achHeight};

            if (ach.unlocked && !achievementSystem.isRewardClaimed(ach.id)) {
                SDL_Rect claimBtn = {rect.x + rect.w - 80, rect.y + 10, 70, 40};

                if (mx >= claimBtn.x && mx <= claimBtn.x + claimBtn.w && my >= claimBtn.y && my <= claimBtn.y + claimBtn.h) {
                    // 1. Gọi phương thức nhận thưởng
                    achievementSystem.claimReward(ach.id, player);

                    // 2. Trigger hiệu ứng (nếu cần)
                    triggerParticleBurst((float)claimBtn.x + claimBtn.w / 2, (float)claimBtn.y + claimBtn.h / 2, 20);

                    return false;
                    std::cout << "Attempting to claim achievement: " << ach.name << std::endl;
                    return false;
                }
            }
            currentY += achHeight + spacing;
        }
    }
    return false;
}

// Định nghĩa Phương thức updateParticles
void AchievementScreen::updateParticles() {
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->update();
        if (it->lifetime <= 0) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

// Định nghĩa Phương thức triggerParticleBurst
void AchievementScreen::triggerParticleBurst(float x, float y, int count) {
    for (int i = 0; i < count; ++i) {
        particles.emplace_back(x, y);
    }
}

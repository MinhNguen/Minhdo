#ifndef ACHIEVEMENT_SCREEN_H_INCLUDED
#define ACHIEVEMENT_SCREEN_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "combo_achievement.h"
#include "player.h"
#include <cmath>

// Particle effect cho achievement unlock
struct AchievementParticle {
    float x, y, vx, vy;
    SDL_Color color;
    int lifetime, maxLifetime;
    float size;

    AchievementParticle(float px, float py) {
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

    void update() {
        x += vx;
        y += vy;
        vy += 0.15f; // Gravity
        lifetime--;
    }

    bool isAlive() const { return lifetime > 0; }

    void render(SDL_Renderer* renderer) {
        float alpha = (float)lifetime / maxLifetime;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, (Uint8)(alpha * 255));
        SDL_Rect rect = {(int)(x - size/2), (int)(y - size/2), (int)size, (int)size};
        SDL_RenderFillRect(renderer, &rect);
    }
};

class AchievementScreen {
public:
    enum Tab { ALL, LOCKED, UNLOCKED, RARE };
    Tab currentTab;
    int scrollOffset;
    std::vector<AchievementParticle> particles;

    AchievementScreen() : currentTab(ALL), scrollOffset(0) {}

    SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface) return nullptr;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return texture;
    }

    void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int y, int screenW) {
        SDL_Texture* tex = renderText(renderer, font, text, color);
        if (tex) {
            int w, h;
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
            SDL_Rect dst = {(screenW - w) / 2, y, w, h};
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
    }

    void renderLeftText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
        SDL_Texture* tex = renderText(renderer, font, text, color);
        if (tex) {
            int w, h;
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
            SDL_Rect dst = {x, y, w, h};
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
    }

    void spawnParticles(int centerX, int centerY, int count) {
        for (int i = 0; i < count; i++) {
            particles.emplace_back(centerX, centerY);
        }
    }

    void updateParticles() {
        for (auto& p : particles) p.update();
        particles.erase(
            std::remove_if(particles.begin(), particles.end(), [](const AchievementParticle& p) { return !p.isAlive(); }),
            particles.end()
        );
    }

    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall, TTF_Font* fontTiny,
                AchievementSystem& achSystem, const Player& player, UIRenderer& uiRenderer) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color black = {0, 0, 0, 255};
        SDL_Color gold = {255, 215, 0, 255};
        SDL_Color green = {0, 200, 0, 255};
        SDL_Color gray = {128, 128, 128, 255};
        SDL_Color darkGray = {64, 64, 64, 255};

        // Title panel
        SDL_FRect titlePanel = {200, 5, 400, 70};
        uiRenderer.drawEnhancedGlassPanel(titlePanel, {150, 100, 200, 180});
        uiRenderer.renderTextCentered("ACHIEVEMENTS", 400, 40, fontBig, white);

        // Stats panel
        int unlockedCount = 0, totalRewards = 0;
        for (const auto& ach : achSystem.achievements) {
            if (ach.unlocked) {
                unlockedCount++;
                totalRewards += ach.reward;
            }
        }

        SDL_FRect statsPanel = {150, 80, 500, 40};
        uiRenderer.drawEnhancedGlassPanel(statsPanel, {80, 80, 150, 160});

        std::string statsText = "Unlocked: " + std::to_string(unlockedCount) + "/" +
                               std::to_string(achSystem.achievements.size()) +
                               " | Rewards: " + std::to_string(totalRewards) + " coins";
        uiRenderer.renderTextCentered(statsText, 400, 100, fontTiny, white);

        // Tabs with enhanced buttons
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        SDL_FRect allTab = {100, 130, 120, 35};
        SDL_FRect lockedTab = {230, 130, 120, 35};
        SDL_FRect unlockedTab = {360, 130, 140, 35};
        SDL_FRect rareTab = {510, 130, 120, 35};

        auto renderTab = [&](SDL_FRect rect, const std::string& text, Tab tab) {
            bool hovered = (mx >= rect.x && mx <= rect.x + rect.w &&
                           my >= rect.y && my <= rect.y + rect.h);
            bool selected = (currentTab == tab);

            SDL_Color tabColor;
            if (selected) tabColor = {100, 180, 255, 255};
            else if (hovered) tabColor = {150, 150, 150, 255};
            else tabColor = {100, 100, 100, 200};

            uiRenderer.renderEnhancedButton(rect, selected || hovered, text, fontTiny, tabColor);
        };

        renderTab(allTab, "All", ALL);
        renderTab(lockedTab, "Locked", LOCKED);
        renderTab(unlockedTab, "Unlocked", UNLOCKED);
        renderTab(rareTab, "Rare", RARE);

        // Filter achievements
        std::vector<Achievement*> filteredAchs;
        for (auto& ach : achSystem.achievements) {
            bool shouldShow = false;
            switch (currentTab) {
                case ALL: shouldShow = true; break;
                case LOCKED: shouldShow = !ach.unlocked; break;
                case UNLOCKED: shouldShow = ach.unlocked; break;
                case RARE: shouldShow = (ach.reward >= 100); break;
            }
            if (shouldShow) filteredAchs.push_back(&ach);
        }

        // Render achievements with enhanced panels
        int startY = 180;
        int achHeight = 90;
        int spacing = 12;

        for (size_t i = 0; i < filteredAchs.size(); i++) {
            Achievement* ach = filteredAchs[i];
            int y = startY + i * (achHeight + spacing) - scrollOffset;

            if (y < 170 || y > 480) continue;

            SDL_FRect achRect = {50, (float)y, 700, (float)achHeight};

            // Enhanced background
            SDL_Color bgColor;
            if (ach->unlocked) {
                if (ach->reward >= 100) bgColor = {255, 215, 0, 200}; // Rare - Gold
                else bgColor = {100, 200, 100, 180}; // Common - Green
            } else {
                bgColor = {80, 80, 80, 160}; // Locked
            }

            uiRenderer.drawEnhancedGlassPanel(achRect, bgColor);

            // Icon with glow
            SDL_FRect iconRect = {achRect.x + 15, achRect.y + 15, 60, 60};
            SDL_Color iconColor;
            if (ach->unlocked) {
                if (ach->reward >= 100) iconColor = {255, 215, 0, 255};
                else iconColor = {50, 150, 255, 255};
            } else {
                iconColor = {50, 50, 50, 255};
            }
            uiRenderer.drawRoundedRect(iconRect, 10, iconColor);

            // Text
            float textX = achRect.x + 90;
            std::string displayName = ach->unlocked ? ach->name : "???";
            std::string displayDesc = ach->unlocked ? ach->description : "Hidden achievement";

            uiRenderer.renderTextLeft(displayName, textX, achRect.y + 15, fontSmall, white);
            uiRenderer.renderTextLeft(displayDesc, textX, achRect.y + 43, fontTiny, {200, 200, 200, 255});

            // Reward/Status
            if (ach->unlocked) {
                std::string rewardText = "Reward: " + std::to_string(ach->reward) + " coins";
                uiRenderer.renderTextLeft(rewardText, textX, achRect.y + 65, fontTiny, gold);

                // Badge
                SDL_FRect badge = {achRect.x + achRect.w - 110, achRect.y + 10, 95, 25};
                uiRenderer.renderEnhancedButton(badge, false, "UNLOCKED", fontTiny, {0, 200, 0, 255});
            } else {
                SDL_FRect badge = {achRect.x + achRect.w - 90, achRect.y + 10, 75, 25};
                uiRenderer.renderEnhancedButton(badge, false, "LOCKED", fontTiny, {100, 100, 100, 255});
            }

            // Rarity indicator
            if (ach->reward >= 100) {
                SDL_FRect rareBadge = {textX + 200, achRect.y + 15, 65, 22};
                uiRenderer.renderEnhancedButton(rareBadge, false, "RARE", fontTiny, {200, 150, 0, 255});
            }
        }

        // Particles
        for (auto& p : particles) p.render(renderer);

        // Back button with enhanced style
        SDL_FRect backBtn = {300, 420, 200, 50};
        bool hovBack = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                        my >= backBtn.y && my <= backBtn.y + backBtn.h);
        uiRenderer.renderEnhancedButton(backBtn, hovBack, "BACK", fontSmall, {180, 50, 50, 255});

        // Scroll indicator
        if (filteredAchs.size() * (achHeight + spacing) > 270) {
            uiRenderer.renderTextCentered("Use Mouse Wheel to Scroll", 400, 465, fontTiny, gray);
        }
    }

    bool handleInput(SDL_Event& e) {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;

            // Back button
            SDL_Rect backBtn = {300, 420, 200, 50};
            if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w && my >= backBtn.y && my <= backBtn.y + backBtn.h) {
                return true; // Exit achievement screen
            }

            // Tab clicks
            SDL_Rect allTab = {100, 95, 120, 35};
            SDL_Rect lockedTab = {230, 95, 120, 35};
            SDL_Rect unlockedTab = {360, 95, 140, 35};
            SDL_Rect rareTab = {510, 95, 120, 35};

            if (mx >= allTab.x && mx <= allTab.x + allTab.w && my >= allTab.y && my <= allTab.y + allTab.h)
                currentTab = ALL;
            else if (mx >= lockedTab.x && mx <= lockedTab.x + lockedTab.w && my >= lockedTab.y && my <= lockedTab.y + lockedTab.h)
                currentTab = LOCKED;
            else if (mx >= unlockedTab.x && mx <= unlockedTab.x + unlockedTab.w && my >= unlockedTab.y && my <= unlockedTab.y + unlockedTab.h)
                currentTab = UNLOCKED;
            else if (mx >= rareTab.x && mx <= rareTab.x + rareTab.w && my >= rareTab.y && my <= rareTab.y + rareTab.h)
                currentTab = RARE;
        }
        else if (e.type == SDL_MOUSEWHEEL) {
            scrollOffset -= e.wheel.y * 20;
            if (scrollOffset < 0) scrollOffset = 0;
        }
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            return true;
        }

        return false;
    }
};

#endif // ACHIEVEMENT_SCREEN_H_INCLUDED

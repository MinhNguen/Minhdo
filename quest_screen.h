#ifndef QUEST_SCREEN_H_INCLUDED
#define QUEST_SCREEN_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "quest_system.h"
#include "player.h"

class QuestScreen {
public:
    enum Tab { DAILY, MAIN };
    Tab currentTab;

    QuestScreen() : currentTab(DAILY) {}

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
            SDL_Rect dst = { (screenW - w) / 2, y, w, h };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
    }

    void renderLeftText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
        SDL_Texture* tex = renderText(renderer, font, text, color);
        if (tex) {
            int w, h;
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
            SDL_Rect dst = { x, y, w, h };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
    }

    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall, TTF_Font* fontTiny,
                QuestSystem& questSystem, Player& player, UIRenderer& uiRenderer) {
        SDL_Color white={255,255,255,255}, black={0,0,0,255}, green={0,200,0,255},
                  yellow={255,215,0,255}, orange={255,140,0,255}, gray={128,128,128,255},
                  darkGray={80,80,80,255};

        // Title panel
        SDL_FRect titlePanel = {250, 5, 300, 70};
        uiRenderer.drawEnhancedGlassPanel(titlePanel, {100, 150, 200, 180});
        uiRenderer.renderTextCentered("QUESTS", 400, 40, fontBig, white);

        // Tabs with enhanced buttons
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        SDL_FRect dailyTab = {150, 85, 200, 40};
        SDL_FRect mainTab = {400, 85, 200, 40};

        // Daily Tab
        bool hovDaily = (mx >= dailyTab.x && mx <= dailyTab.x + dailyTab.w &&
                         my >= dailyTab.y && my <= dailyTab.y + dailyTab.h);
        bool isDaily = (currentTab == DAILY);
        SDL_Color dailyColor = isDaily ? SDL_Color{100, 180, 255, 255} : SDL_Color{120, 120, 120, 200};
        uiRenderer.renderEnhancedButton(dailyTab, hovDaily || isDaily, "DAILY QUESTS", fontTiny, dailyColor);

        // Main Tab
        bool hovMain = (mx >= mainTab.x && mx <= mainTab.x + mainTab.w &&
                        my >= mainTab.y && my <= mainTab.y + mainTab.h);
        bool isMain = (currentTab == MAIN);
        SDL_Color mainColor = isMain ? SDL_Color{100, 180, 255, 255} : SDL_Color{120, 120, 120, 200};
        uiRenderer.renderEnhancedButton(mainTab, hovMain || isMain, "MAIN QUESTS", fontTiny, mainColor);

        // Render quests with enhanced panels
        std::vector<Quest>& quests = (currentTab == DAILY) ? questSystem.dailyQuests : questSystem.mainQuests;
        int startY = 140, questHeight = 100, spacing = 10;

        for (size_t i = 0; i < quests.size(); i++) {
            Quest& quest = quests[i];
            int y = startY + i * (questHeight + spacing);
            SDL_FRect questRect = {50, (float)y, 700, (float)questHeight};

            // Background color based on status
            SDL_Color bgColor;
            if (quest.isCompleted && quest.rewardClaimed) {
                bgColor = {80, 80, 80, 180};
            } else if (quest.isCompleted) {
                bgColor = {255, 215, 0, 200};
            } else if (quest.isActive) {
                bgColor = {70, 170, 70, 180};
            } else {
                bgColor = {100, 100, 100, 160};
            }

            uiRenderer.drawEnhancedGlassPanel(questRect, bgColor);

            // Quest title and description
            uiRenderer.renderTextLeft(quest.title, questRect.x + 15, questRect.y + 12, fontSmall, white);
            uiRenderer.renderTextLeft(quest.description, questRect.x + 15, questRect.y + 37,
                                     fontTiny, {220, 220, 220, 255});

            // Progress Bar with enhanced style
            if (quest.isActive && !quest.isCompleted) {
                SDL_FRect bgBar = {questRect.x + 15, questRect.y + 62, 200, 18};
                uiRenderer.drawRoundedRect(bgBar, 9, {50, 50, 50, 255});

                float progress = quest.getProgressPercent();
                SDL_FRect progBar = {questRect.x + 15, questRect.y + 62, 200 * progress, 18};

                // Gradient progress bar
                SDL_Color progColor = uiRenderer.hsvToRgb(120 * progress, 0.8f, 0.9f);
                uiRenderer.drawRoundedRect(progBar, 9, progColor);

                // Progress text
                std::string progText = std::to_string(quest.currentProgress) + "/" +
                                      std::to_string(quest.requirement);
                uiRenderer.renderTextLeft(progText, questRect.x + 225, questRect.y + 65, fontTiny, white);
            }

            // Reward info
            std::string rewardText = "Reward: " + std::to_string(quest.coinReward) +
                                    " coins, " + std::to_string(quest.xpReward) + " XP";
            uiRenderer.renderTextLeft(rewardText, questRect.x + 15, questRect.y + 82, fontTiny, orange);

            // Status button with enhanced style
            SDL_FRect statusBtn = {questRect.x + questRect.w - 125, questRect.y + 12, 110, 35};
            bool hovStatus = (mx >= statusBtn.x && mx <= statusBtn.x + statusBtn.w &&
                             my >= statusBtn.y && my <= statusBtn.y + statusBtn.h);

            SDL_Color btnColor;
            std::string btnText;

            if (quest.isCompleted && quest.rewardClaimed) {
                btnColor = {100, 100, 100, 255};
                btnText = "CLAIMED";
            } else if (quest.isCompleted) {
                btnColor = {255, 200, 0, 255};
                btnText = "CLAIM";
            } else if (quest.isActive) {
                btnColor = {0, 150, 0, 255};
                btnText = "ACTIVE";
            } else {
                btnColor = {70, 100, 200, 255};
                btnText = "START";
            }

            uiRenderer.renderEnhancedButton(statusBtn, hovStatus, btnText, fontTiny, btnColor);
        }

        // Back button
        SDL_FRect backBtn = {300, 420, 200, 50};
        bool hovBack = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                        my >= backBtn.y && my <= backBtn.y + backBtn.h);
        uiRenderer.renderEnhancedButton(backBtn, hovBack, "BACK", fontSmall, {180, 50, 50, 255});
    }

    bool handleInput(SDL_Event& e, QuestSystem& questSystem, Player& player) {
        if (e.type != SDL_MOUSEBUTTONDOWN) return false;
        int mx = e.button.x, my = e.button.y;

        // Back button
        SDL_Rect backBtn = {300, 420, 200, 50};
        if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w && my >= backBtn.y && my <= backBtn.y + backBtn.h)
            return true;

        // Tab switching
        SDL_Rect dailyTab = {150, 85, 200, 40};
        if (mx >= dailyTab.x && mx <= dailyTab.x + dailyTab.w && my >= dailyTab.y && my <= dailyTab.y + dailyTab.h) {
            currentTab = DAILY;
            return false;
        }

        SDL_Rect mainTab = {400, 85, 200, 40};
        if (mx >= mainTab.x && mx <= mainTab.x + mainTab.w && my >= mainTab.y && my <= mainTab.y + mainTab.h) {
            currentTab = MAIN;
            return false;
        }

        // Quest interaction
        std::vector<Quest>& quests = (currentTab == DAILY) ? questSystem.dailyQuests : questSystem.mainQuests;
        int startY = 140, questHeight = 100, spacing = 10;

        for (size_t i = 0; i < quests.size(); i++) {
            Quest& quest = quests[i];
            int y = startY + i * (questHeight + spacing);
            SDL_Rect statusBtn = {700, y + 10, 110, 35};

            if (mx >= statusBtn.x && mx <= statusBtn.x + statusBtn.w && my >= statusBtn.y && my <= statusBtn.y + statusBtn.h) {
                if (quest.isCompleted && !quest.rewardClaimed) {
                    // Claim reward
                    player.totalCoins += quest.coinReward;
                    player.addXp(quest.xpReward);
                    quest.rewardClaimed = true;
                    questSystem.saveProgress();
                } else if (!quest.isActive && !quest.isCompleted) {
                    // Start/Activate quest
                    questSystem.activateQuest(quest.id, currentTab == DAILY);
                    questSystem.saveProgress();
                }
                break;
            }
        }

        return false;
    }
};

#endif // QUEST_SCREEN_H_INCLUDED

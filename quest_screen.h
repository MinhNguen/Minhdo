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
        if (tex) { int w, h; SDL_QueryTexture(tex, NULL, NULL, &w, &h); SDL_Rect dst = { (screenW - w) / 2, y, w, h }; SDL_RenderCopy(renderer, tex, NULL, &dst); SDL_DestroyTexture(tex); }
    }

    void renderLeftText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
        SDL_Texture* tex = renderText(renderer, font, text, color);
        if (tex) { int w, h; SDL_QueryTexture(tex, NULL, NULL, &w, &h); SDL_Rect dst = { x, y, w, h }; SDL_RenderCopy(renderer, tex, NULL, &dst); SDL_DestroyTexture(tex); }
    }

    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall, TTF_Font* fontTiny, QuestSystem& questSystem, Player& player) {
        SDL_Color white={255,255,255,255}, black={0,0,0,255}, green={0,200,0,255}, yellow={255,215,0,255}, orange={255,140,0,255}, gray={128,128,128,255}, blue={50,150,255,255};
        renderCenteredText(renderer, fontBig, "QUESTS", black, 15, 800);

        std::vector<Quest>& quests = (currentTab == DAILY) ? questSystem.dailyQuests : questSystem.mainQuests;
        int startY = 140, questHeight = 100, spacing = 10;
        int mx, my; SDL_GetMouseState(&mx, &my);

        for (size_t i = 0; i < quests.size(); i++) {
            Quest& quest = quests[i];
            int y = startY + i * (questHeight + spacing);
            SDL_Rect questRect = {50, y, 700, questHeight};

            // Background
            if (quest.isCompleted && quest.rewardClaimed) SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            else if (quest.isCompleted) SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
            else if (quest.isActive) SDL_SetRenderDrawColor(renderer, 70, 170, 70, 255);
            else SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
            SDL_RenderFillRect(renderer, &questRect);

            // Text
            renderLeftText(renderer, fontSmall, quest.title, black, questRect.x + 10, questRect.y + 10);
            renderLeftText(renderer, fontTiny, quest.description, gray, questRect.x + 10, questRect.y + 35);

            // Progress Bar
            if (quest.isActive && !quest.isCompleted) {
                 SDL_Rect bgBar = {questRect.x+10, questRect.y+60, 200, 15};
                 SDL_SetRenderDrawColor(renderer, 50,50,50,255); SDL_RenderFillRect(renderer, &bgBar);
                 SDL_Rect progBar = {questRect.x+10, questRect.y+60, (int)(200 * quest.getProgressPercent()), 15};
                 SDL_SetRenderDrawColor(renderer, 0,200,0,255); SDL_RenderFillRect(renderer, &progBar);
            }
        }
    }

    bool handleInput(SDL_Event& e, QuestSystem& questSystem, Player& player) {
        if (e.type != SDL_MOUSEBUTTONDOWN) return false;
        int mx = e.button.x, my = e.button.y;

        SDL_Rect backBtn = {300, 420, 200, 50};
        if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w && my >= backBtn.y && my <= backBtn.y + backBtn.h) return true;

        SDL_Rect dailyTab = {150, 85, 200, 40}; if(mx >= dailyTab.x && mx <= dailyTab.x + dailyTab.w && my >= dailyTab.y && my <= dailyTab.y+dailyTab.h) { currentTab=DAILY; return false; }
        SDL_Rect mainTab = {400, 85, 200, 40}; if(mx >= mainTab.x && mx <= mainTab.x + mainTab.w && my >= mainTab.y && my <= mainTab.y+mainTab.h) { currentTab=MAIN; return false; }

        std::vector<Quest>& quests = (currentTab == DAILY) ? questSystem.dailyQuests : questSystem.mainQuests;
        int startY = 140, questHeight = 100, spacing = 10;
        for (size_t i = 0; i < quests.size(); i++) {
            Quest& quest = quests[i];
            int y = startY + i * (questHeight + spacing);
            SDL_Rect questRect = {50, y, 700, questHeight};
            if (mx >= questRect.x && mx <= questRect.x + questRect.w && my >= questRect.y && my <= questRect.y + questRect.h) {
                if (quest.isCompleted && !quest.rewardClaimed) {
                    player.totalCoins += quest.coinReward;
                    player.addXp(quest.xpReward);
                    quest.rewardClaimed = true;
                    questSystem.saveProgress();
                } else if (!quest.isActive) {
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

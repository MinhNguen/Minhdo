#ifndef QUEST_SCREEN_H_INCLUDED
#define QUEST_SCREEN_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "quest_system.h"
#include "player.h"
#include "ui_renderer.h"

class QuestScreen {
public:
    enum Tab { DAILY, MAIN };
    Tab currentTab;

    QuestScreen();

    // Public methods
    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall, TTF_Font* fontTiny,
                QuestSystem& questSystem, Player& player, UIRenderer& uiRenderer);
    bool handleInput(SDL_Event& e, QuestSystem& questSystem, Player& player);

private:
    // Private helper methods
    SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color);
    void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int y, int screenW);
    void renderLeftText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y);
};

#endif // QUEST_SCREEN_H_INCLUDED

#ifndef ACHIEVEMENT_SCREEN_H_INCLUDED
#define ACHIEVEMENT_SCREEN_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "combo_achievement.h"
#include "quest_system.h"
#include "player.h"
#include <cmath>
#include <vector>

// Particle effect cho achievement unlock
struct AchievementParticle {
    float x, y, vx, vy;
    SDL_Color color;
    int lifetime, maxLifetime;
    float size;

    AchievementParticle(float px, float py);
    void update();
    void render(SDL_Renderer* renderer);
};

class AchievementScreen {
public:
    AchievementTab currentTab;
    std::vector<AchievementParticle> particles;

    AchievementScreen();
    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontMedium, TTF_Font* fontSmall,
                int screenW, int screenH, AchievementSystem& achievementSystem, Player& player);

    bool handleInput(SDL_Event& e, int screenW, int screenH, AchievementSystem& achievementSystem, QuestSystem& questSystem, Player& player);

    void updateParticles();
    void triggerParticleBurst(float x, float y, int count);

private:
    void renderAchievementList(SDL_Renderer* renderer, TTF_Font* fontMedium, TTF_Font* fontSmall,
                               int startY, int screenW, const std::vector<Achievement*>& achievements,
                               AchievementSystem& achievementSystem, Player& player);
    void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y);
    void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int y, int screenW);
};

#endif // ACHIEVEMENT_SCREEN_H_INCLUDED

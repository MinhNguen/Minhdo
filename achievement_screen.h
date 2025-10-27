#ifndef ACHIEVEMENT_SCREEN_H_INCLUDED
#define ACHIEVEMENT_SCREEN_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "combo_achievement.h"
#include "player.h"
#include <cmath>
#include <vector>

// Particle effect cho achievement unlock
struct AchievementParticle {
    float x, y, vx, vy;
    SDL_Color color;
    int lifetime, maxLifetime;
    float size;

    AchievementParticle(float px, float py); // Khai báo Constructor
    void update(); // Khai báo Phương thức
    void render(SDL_Renderer* renderer); // Khai báo Phương thức
};

class AchievementScreen {
public:
    enum Tab { ALL, LOCKED, UNLOCKED, RARE };
    Tab currentTab;
    std::vector<AchievementParticle> particles;

    AchievementScreen(); // Khai báo Constructor

    // Khai báo các phương thức
    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontMedium, TTF_Font* fontSmall,
                int screenW, int screenH, AchievementSystem& achievementSystem, Player& player);
    bool handleInput(SDL_Event& e, int screenW, AchievementSystem& achievementSystem);
    void updateParticles();
    void triggerParticleBurst(float x, float y, int count);

private:
    // Khai báo các hàm helper
    void renderAchievementList(SDL_Renderer* renderer, TTF_Font* fontMedium, TTF_Font* fontSmall,
                               int startY, int screenW, const std::vector<Achievement>& achievements,
                               AchievementSystem& achievementSystem, Player& player);
    void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y);
    void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int y, int screenW);
};

#endif // ACHIEVEMENT_SCREEN_H_INCLUDED

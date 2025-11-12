#ifndef MAP_THEME_H_INCLUDED
#define MAP_THEME_H_INCLUDED

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <cmath>
#include "DayNightCycle.h"


// Particle cho hiệu ứng môi trường
struct EnvironmentParticle {
    float x, y, vx, vy;
    SDL_Color color;
    float size;
    int lifetime, maxLifetime;

    EnvironmentParticle(float px, float py, float velX, float velY, SDL_Color c, float s, int life);
    void update();
    void render(SDL_Renderer* renderer);
    bool isDead() const;
};

// Class Map Theme
class MapTheme {
private:
    MapThemeType type;
    std::string name;
    SDL_Color groundColor;
    SDL_Color accentColor;
    std::vector<EnvironmentParticle> particles;
    int particleSpawnTimer;

public:
    MapTheme(MapThemeType themeType = GRASSLAND);
    void setupTheme();
    void setTheme(MapThemeType newType);
    void update(int screenWidth, int screenHeight, const DayNightCycle& dayNight);
    void spawnParticles(int screenWidth, int screenHeight, const DayNightCycle& dayNight);
    int getParticleSpawnInterval() const;
    void renderBackground(SDL_Renderer* renderer, int screenWidth, int screenHeight,
                         const DayNightCycle& dayNight);
    void renderStars(SDL_Renderer* renderer, int screenWidth, int screenHeight,
                    const DayNightCycle& dayNight);
    void renderCelestialBody(SDL_Renderer* renderer, int screenWidth, int screenHeight,
                            const DayNightCycle& dayNight);
    void renderGround(SDL_Renderer* renderer, int groundY, int screenWidth, int screenHeight,
                     const DayNightCycle& dayNight);
    void renderGroundDetails(SDL_Renderer* renderer, int groundY, int screenWidth,
                           int screenHeight, const DayNightCycle& dayNight);
    void renderParticles(SDL_Renderer* renderer);

    std::string getName() const { return name; }
    MapThemeType getType() const { return type; }
    SDL_Color getGroundColor() const { return groundColor; }
    SDL_Color getAccentColor() const { return accentColor; }

private:
    void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius);
    void fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius);
};

#endif // MAP_THEME_H_INCLUDED

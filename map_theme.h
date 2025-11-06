#ifndef MAP_THEME_H_INCLUDED
#define MAP_THEME_H_INCLUDED

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <cmath>

// Chu kỳ ngày đêm
enum TimeOfDay {
    MORNING,    // 6:00 - 10:00
    DAY,        // 10:00 - 16:00
    EVENING,    // 16:00 - 19:00
    NIGHT       // 19:00 - 6:00
};

// Theme của map
enum MapThemeType {
    GRASSLAND,
    DESERT,
    FOREST,
    MOUNTAIN,
    VOLCANO
};

// Class quản lý chu kỳ ngày đêm
class DayNightCycle {
private:
    float timeOfDayProgress; // 0.0 - 1.0 (0 = midnight, 0.5 = noon)
    float cycleSpeed;
    TimeOfDay currentTimeOfDay;

public:
    DayNightCycle(float speed = 0.0005f);
    void update();
    void updateTimeOfDay();
    TimeOfDay getCurrentTimeOfDay() const;
    float getTimeProgress() const;
    SDL_Color getSkyColor() const;
    SDL_Color getGroundBaseColor(const SDL_Color& baseColor) const;
    float getBrightness() const;
    void reset();

private:
    SDL_Color lerpColor(const SDL_Color& a, const SDL_Color& b, float t) const;
};

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

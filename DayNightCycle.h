#ifndef DAYNIGHTCYCLE_H_INCLUDED
#define DAYNIGHTCYCLE_H_INCLUDED
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

enum MapThemeType {
    GRASSLAND,
    DESERT,
    FOREST,
    MOUNTAIN,
    VOLCANO
};
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

#endif // DAYNIGHTCYCLE_H_INCLUDED

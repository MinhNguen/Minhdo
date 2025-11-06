#include "map_theme.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

// ===================== DAYNIGHTCYCLE IMPLEMENTATION =====================

DayNightCycle::DayNightCycle(float speed) : timeOfDayProgress(0.25f), cycleSpeed(speed) {
    updateTimeOfDay();
}

void DayNightCycle::update() {
    timeOfDayProgress += cycleSpeed;
    if (timeOfDayProgress >= 1.0f) {
        timeOfDayProgress = 0.0f;
    }
    updateTimeOfDay();
}

void DayNightCycle::updateTimeOfDay() {
    float hour = timeOfDayProgress * 24.0f;

    if (hour >= 6.0f && hour < 10.0f) {
        currentTimeOfDay = MORNING;
    } else if (hour >= 10.0f && hour < 16.0f) {
        currentTimeOfDay = DAY;
    } else if (hour >= 16.0f && hour < 19.0f) {
        currentTimeOfDay = EVENING;
    } else {
        currentTimeOfDay = NIGHT;
    }
}

TimeOfDay DayNightCycle::getCurrentTimeOfDay() const {
    return currentTimeOfDay;
}

float DayNightCycle::getTimeProgress() const {
    return timeOfDayProgress;
}

SDL_Color DayNightCycle::getSkyColor() const {
    float hour = timeOfDayProgress * 24.0f;

    // Morning (6-10): Gradient từ dark blue -> light blue
    if (hour >= 6.0f && hour < 10.0f) {
        float t = (hour - 6.0f) / 4.0f;
        return lerpColor({30, 60, 100, 255}, {135, 206, 235, 255}, t);
    }
    // Day (10-16): Light blue
    else if (hour >= 10.0f && hour < 16.0f) {
        return {135, 206, 235, 255};
    }
    // Evening (16-19): Gradient light blue -> orange -> dark
    else if (hour >= 16.0f && hour < 19.0f) {
        float t = (hour - 16.0f) / 3.0f;
        if (t < 0.5f) {
            return lerpColor({135, 206, 235, 255}, {255, 140, 50, 255}, t * 2.0f);
        } else {
            return lerpColor({255, 140, 50, 255}, {30, 30, 60, 255}, (t - 0.5f) * 2.0f);
        }
    }
    // Night (19-6): Dark blue with stars
    else {
        return {15, 15, 40, 255};
    }
}

SDL_Color DayNightCycle::getGroundBaseColor(const SDL_Color& baseColor) const {
    float brightness = getBrightness();
    return {
        static_cast<Uint8>(baseColor.r * brightness),
        static_cast<Uint8>(baseColor.g * brightness),
        static_cast<Uint8>(baseColor.b * brightness),
        255
    };
}

float DayNightCycle::getBrightness() const {
    float hour = timeOfDayProgress * 24.0f;

    if (hour >= 6.0f && hour < 10.0f) {
        // Morning: 0.5 -> 1.0
        return 0.5f + (hour - 6.0f) / 8.0f;
    } else if (hour >= 10.0f && hour < 16.0f) {
        // Day: 1.0
        return 1.0f;
    } else if (hour >= 16.0f && hour < 19.0f) {
        // Evening: 1.0 -> 0.5
        return 1.0f - (hour - 16.0f) / 6.0f;
    } else {
        // Night: 0.3 - 0.5
        return 0.3f + 0.2f * sin(timeOfDayProgress * M_PI * 2.0f);
    }
}

void DayNightCycle::reset() {
    timeOfDayProgress = 0.25f; // Start at morning
    updateTimeOfDay();
}

SDL_Color DayNightCycle::lerpColor(const SDL_Color& a, const SDL_Color& b, float t) const {
    t = std::max(0.0f, std::min(1.0f, t));
    return {
        static_cast<Uint8>(a.r + (b.r - a.r) * t),
        static_cast<Uint8>(a.g + (b.g - a.g) * t),
        static_cast<Uint8>(a.b + (b.b - a.b) * t),
        static_cast<Uint8>(a.a + (b.a - a.a) * t)
    };
}

// ===================== ENVIRONMENTPARTICLE IMPLEMENTATION =====================

EnvironmentParticle::EnvironmentParticle(float px, float py, float velX, float velY, SDL_Color c, float s, int life)
    : x(px), y(py), vx(velX), vy(velY), color(c), size(s), lifetime(life), maxLifetime(life) {}

void EnvironmentParticle::update() {
    x += vx;
    y += vy;
    lifetime--;
}

void EnvironmentParticle::render(SDL_Renderer* renderer) {
    if (lifetime > 0) {
        float alpha = (float)lifetime / maxLifetime;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b,
                              static_cast<Uint8>(color.a * alpha));
        SDL_Rect rect = {static_cast<int>(x), static_cast<int>(y),
                       static_cast<int>(size), static_cast<int>(size)};
        SDL_RenderFillRect(renderer, &rect);
    }
}

bool EnvironmentParticle::isDead() const { return lifetime <= 0; }

// ===================== MAPTHEME IMPLEMENTATION =====================

MapTheme::MapTheme(MapThemeType themeType) : type(themeType), particleSpawnTimer(0) {
    setupTheme();
}

void MapTheme::setupTheme() {
    switch (type) {
        case GRASSLAND:
            name = "Grassland";
            groundColor = {34, 139, 34, 255};
            accentColor = {0, 200, 0, 255};
            break;

        case DESERT:
            name = "Desert";
            groundColor = {210, 180, 140, 255};
            accentColor = {255, 215, 0, 255};
            break;

        case FOREST:
            name = "Forest";
            groundColor = {45, 100, 45, 255};
            accentColor = {0, 150, 0, 255};
            break;

        case MOUNTAIN:
            name = "Mountain";
            groundColor = {120, 120, 140, 255};
            accentColor = {200, 200, 220, 255};
            break;

        case VOLCANO:
            name = "Volcano";
            groundColor = {80, 40, 30, 255};
            accentColor = {255, 69, 0, 255};
            break;
    }
}

void MapTheme::setTheme(MapThemeType newType) {
    type = newType;
    setupTheme();
    particles.clear();
}

void MapTheme::update(int screenWidth, int screenHeight, const DayNightCycle& dayNight) {
    // Update existing particles
    for (auto& p : particles) {
        p.update();
    }

    // Remove dead particles
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                      [](const EnvironmentParticle& p) { return p.isDead(); }),
        particles.end()
    );

    // Spawn new particles based on theme
    particleSpawnTimer++;
    if (particleSpawnTimer >= getParticleSpawnInterval()) {
        spawnParticles(screenWidth, screenHeight, dayNight);
        particleSpawnTimer = 0;
    }
}

void MapTheme::spawnParticles(int screenWidth, int screenHeight, const DayNightCycle& dayNight) {
    switch (type) {
        case GRASSLAND:
            // Butterflies or leaves
            if (dayNight.getCurrentTimeOfDay() != NIGHT && rand() % 100 < 30) {
                particles.emplace_back(
                    rand() % screenWidth,
                    rand() % (screenHeight / 2),
                    -1.0f - (rand() % 10) / 10.0f,
                    (rand() % 20 - 10) / 10.0f,
                    SDL_Color{255, 200, 0, 255},
                    3.0f,
                    200 + rand() % 100
                );
            }
            break;

        case DESERT:
            // Sand particles
            if (rand() % 100 < 20) {
                particles.emplace_back(
                    screenWidth,
                    300 + rand() % 100,
                    -2.0f - (rand() % 10) / 10.0f,
                    (rand() % 10 - 5) / 10.0f,
                    SDL_Color{220, 200, 150, 200},
                    2.0f,
                    100 + rand() % 50
                );
            }
            break;

        case FOREST:
            // Fireflies at night, leaves during day
            if (dayNight.getCurrentTimeOfDay() == NIGHT) {
                if (rand() % 100 < 15) {
                    particles.emplace_back(
                        rand() % screenWidth,
                        200 + rand() % 200,
                        (rand() % 20 - 10) / 20.0f,
                        (rand() % 20 - 10) / 20.0f,
                        SDL_Color{255, 255, 100, 255},
                        4.0f,
                        150 + rand() % 100
                    );
                }
            } else {
                if (rand() % 100 < 25) {
                    particles.emplace_back(
                        rand() % screenWidth,
                        rand() % (screenHeight / 2),
                        -1.5f - (rand() % 10) / 10.0f,
                        0.5f + (rand() % 10) / 10.0f,
                        SDL_Color{100, 200, 100, 255},
                        3.0f,
                        180 + rand() % 80
                    );
                }
            }
            break;

        case MOUNTAIN:
            // Snow particles
            if (rand() % 100 < 35) {
                particles.emplace_back(
                    rand() % screenWidth,
                    -10,
                    -0.5f - (rand() % 10) / 20.0f,
                    1.0f + (rand() % 10) / 10.0f,
                    SDL_Color{255, 255, 255, 255},
                    3.0f,
                    300 + rand() % 200
                );
            }
            break;

        case VOLCANO:
            // Ash and embers
            if (rand() % 100 < 40) {
                bool isEmber = rand() % 100 < 30;
                if (isEmber) {
                    particles.emplace_back(
                        200 + rand() % (screenWidth - 400),
                        screenHeight,
                        (rand() % 20 - 10) / 10.0f,
                        -2.0f - (rand() % 15) / 10.0f,
                        SDL_Color{255, 100 + rand() % 100, 0, 255},
                        3.0f + (rand() % 20) / 10.0f,
                        120 + rand() % 80
                    );
                } else {
                    particles.emplace_back(
                        rand() % screenWidth,
                        -10,
                        -0.3f - (rand() % 10) / 20.0f,
                        0.8f + (rand() % 10) / 10.0f,
                        SDL_Color{80, 80, 80, 200},
                        2.0f,
                        200 + rand() % 150
                    );
                }
            }
            break;
    }
}

int MapTheme::getParticleSpawnInterval() const {
    switch (type) {
        case GRASSLAND: return 30;
        case DESERT: return 20;
        case FOREST: return 25;
        case MOUNTAIN: return 15;
        case VOLCANO: return 10;
        default: return 30;
    }
}

void MapTheme::renderBackground(SDL_Renderer* renderer, int screenWidth, int screenHeight,
                         const DayNightCycle& dayNight) {
    // Sky gradient
    SDL_Color skyColor = dayNight.getSkyColor();

    for (int y = 0; y < screenHeight; y++) {
        float t = (float)y / screenHeight;
        SDL_Color currentColor = {
            static_cast<Uint8>(skyColor.r + (255 - skyColor.r) * t * 0.2f),
            static_cast<Uint8>(skyColor.g + (255 - skyColor.g) * t * 0.2f),
            static_cast<Uint8>(skyColor.b + (255 - skyColor.b) * t * 0.2f),
            255
        };
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g,
                              currentColor.b, currentColor.a);
        SDL_RenderDrawLine(renderer, 0, y, screenWidth, y);
    }

    // Stars at night
    if (dayNight.getCurrentTimeOfDay() == NIGHT) {
        renderStars(renderer, screenWidth, screenHeight, dayNight);
    }

    // Sun/Moon
    renderCelestialBody(renderer, screenWidth, screenHeight, dayNight);
}

void MapTheme::renderStars(SDL_Renderer* renderer, int screenWidth, int screenHeight,
                    const DayNightCycle& dayNight) {
    srand(12345); // Fixed seed for consistent star positions
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = 0; i < 100; i++) {
        int x = rand() % screenWidth;
        int y = rand() % (screenHeight / 2);

        // Twinkling effect
        float twinkle = 0.5f + 0.5f * sin(dayNight.getTimeProgress() * M_PI * 4.0f + i);
        if (rand() % 100 < static_cast<int>(twinkle * 100)) {
            SDL_RenderDrawPoint(renderer, x, y);
            if (rand() % 100 < 20) { // Some bigger stars
                SDL_RenderDrawPoint(renderer, x + 1, y);
                SDL_RenderDrawPoint(renderer, x, y + 1);
            }
        }
    }
}

void MapTheme::renderCelestialBody(SDL_Renderer* renderer, int screenWidth, int screenHeight,
                            const DayNightCycle& dayNight) {
    float progress = dayNight.getTimeProgress();
    float angle = progress * M_PI * 2.0f - M_PI / 2.0f; // Start from top

    int centerX = screenWidth / 2 + static_cast<int>(cos(angle) * screenWidth * 0.4f);
    int centerY = screenHeight / 3 + static_cast<int>(sin(angle) * screenHeight * 0.25f);

    TimeOfDay tod = dayNight.getCurrentTimeOfDay();

    if (tod == DAY || tod == MORNING || tod == EVENING) {
        // Sun
        int radius = 30;
        SDL_Color sunColor = {255, 220, 0, 255};

        // Sun glow
        for (int r = radius + 20; r >= radius; r -= 2) {
            float alpha = (float)(radius + 20 - r) / 20.0f;
            SDL_SetRenderDrawColor(renderer, sunColor.r, sunColor.g, sunColor.b,
                                  static_cast<Uint8>(100 * alpha));
            drawCircle(renderer, centerX, centerY, r);
        }

        // Sun body
        SDL_SetRenderDrawColor(renderer, sunColor.r, sunColor.g, sunColor.b, 255);
        fillCircle(renderer, centerX, centerY, radius);
    } else {
        // Moon
        int radius = 25;
        SDL_Color moonColor = {220, 220, 240, 255};

        // Moon glow
        for (int r = radius + 15; r >= radius; r -= 2) {
            float alpha = (float)(radius + 15 - r) / 15.0f;
            SDL_SetRenderDrawColor(renderer, moonColor.r, moonColor.g, moonColor.b,
                                  static_cast<Uint8>(80 * alpha));
            drawCircle(renderer, centerX, centerY, r);
        }

        // Moon body
        SDL_SetRenderDrawColor(renderer, moonColor.r, moonColor.g, moonColor.b, 255);
        fillCircle(renderer, centerX, centerY, radius);

        // Moon craters
        SDL_SetRenderDrawColor(renderer, 180, 180, 200, 255);
        fillCircle(renderer, centerX - 8, centerY - 5, 6);
        fillCircle(renderer, centerX + 5, centerY + 8, 4);
    }
}

void MapTheme::renderGround(SDL_Renderer* renderer, int groundY, int screenWidth, int screenHeight,
                     const DayNightCycle& dayNight) {
    SDL_Color adjustedGroundColor = dayNight.getGroundBaseColor(groundColor);
    SDL_SetRenderDrawColor(renderer, adjustedGroundColor.r, adjustedGroundColor.g,
                          adjustedGroundColor.b, adjustedGroundColor.a);
    SDL_Rect ground = {0, groundY, screenWidth, screenHeight - groundY};
    SDL_RenderFillRect(renderer, &ground);

    // Add texture/details based on theme
    renderGroundDetails(renderer, groundY, screenWidth, screenHeight, dayNight);
}

void MapTheme::renderGroundDetails(SDL_Renderer* renderer, int groundY, int screenWidth,
                           int screenHeight, const DayNightCycle& dayNight) {
    float brightness = dayNight.getBrightness();

    switch (type) {
        case GRASSLAND:
            // Grass stripes
            SDL_SetRenderDrawColor(renderer,
                static_cast<Uint8>(20 * brightness),
                static_cast<Uint8>(100 * brightness),
                static_cast<Uint8>(20 * brightness), 255);
            for (int i = 0; i < 50; i++) {
                int x = (i * 20) % screenWidth;
                SDL_RenderDrawLine(renderer, x, groundY, x + 10, groundY + 15);
            }
            break;

        case DESERT:
            // Sand dunes
            SDL_SetRenderDrawColor(renderer,
                static_cast<Uint8>(180 * brightness),
                static_cast<Uint8>(150 * brightness),
                static_cast<Uint8>(100 * brightness), 100);
            for (int i = 0; i < screenWidth; i += 50) {
                for (int j = 0; j < 20; j++) {
                    SDL_RenderDrawLine(renderer, i, groundY + j * 2,
                                     i + 40, groundY + j * 2);
                }
            }
            break;

        case MOUNTAIN:
            // Rocky texture
            srand(54321);
            SDL_SetRenderDrawColor(renderer,
                static_cast<Uint8>(100 * brightness),
                static_cast<Uint8>(100 * brightness),
                static_cast<Uint8>(120 * brightness), 255);
            for (int i = 0; i < 100; i++) {
                int x = rand() % screenWidth;
                int y = groundY + rand() % (screenHeight - groundY);
                SDL_RenderDrawPoint(renderer, x, y);
            }
            break;

        case VOLCANO:
            // Lava cracks
            SDL_SetRenderDrawColor(renderer, 255, 69, 0,
                static_cast<Uint8>(150 + 50 * sin(dayNight.getTimeProgress() * 10.0f)));
            for (int i = 0; i < screenWidth; i += 80) {
                SDL_RenderDrawLine(renderer, i, groundY + 20, i + 30, groundY + 40);
                SDL_RenderDrawLine(renderer, i + 30, groundY + 40, i + 50, groundY + 25);
            }
            break;

        default:
            break;
    }
}

void MapTheme::renderParticles(SDL_Renderer* renderer) {
    for (auto& p : particles) {
        p.render(renderer);
    }
}

void MapTheme::drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void MapTheme::fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

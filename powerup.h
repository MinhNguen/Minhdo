#ifndef POWERUP_H_INCLUDED
#define POWERUP_H_INCLUDED

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "player.h"
#include "score.h"

// Loại power-up
enum class PowerUpType {
    SHIELD,       // Khiên bảo vệ
    SPEED_BOOST,  // Tăng tốc
    COIN_MAGNET,  // Hút xu
    DASH          // Lướt tới
};

class PowerUp {
public:
    int x, y;
    int width, height;
    int speed;
    bool active;
    bool collected;
    PowerUpType type;
    float animFrame; // Cho hiệu ứng animation

    PowerUp(int startX, int groundY, int gameSpeed, PowerUpType puType);

    void update();
    void render(SDL_Renderer* renderer);
    bool checkCollision(int px, int py, int pwidth, int pheight);

private:
    void renderShieldEffect(SDL_Renderer* renderer, const SDL_Rect& rect);
    void renderSpeedBoostEffect(SDL_Renderer* renderer, const SDL_Rect& rect);
    void renderCoinMagnetEffect(SDL_Renderer* renderer, const SDL_Rect& rect);
    void renderDashEffect(SDL_Renderer* renderer, const SDL_Rect& rect);
};

class PowerUpManager {
public:
    std::vector<PowerUp> powerUps;
    int spawnTimer;
    int spawnInterval;
    int groundY;
    int speed;
    int screenWidth;

    // Trạng thái hiệu ứng
    bool shieldActive;
    int shieldTimer;

    bool speedBoostActive;
    int speedBoostTimer;
    float originalSpeed;

    bool coinMagnetActive;
    int coinMagnetTimer;
    static const int MAGNET_RADIUS = 150;

    int dashCharges;
    int dashCooldown;
    bool dashActive;
    int dashTimer;

    PowerUpManager(int ground, int gameSpeed, int width);
    PowerUpManager(const PowerUpManager& other);

    void reset();
    void update(Player& player);
    void update(Player& player, ScoreManager* scoreManager);
    void activate(PowerUpType type, Player& player);
    void updateEffects(Player& player, ScoreManager* scoreManager);
    void applyCoinMagnetEffect(Player& player, ScoreManager& scoreManager);
    void spawn();
    void render(SDL_Renderer* renderer);
    void renderActiveEffectsUI(SDL_Renderer* renderer);
    bool canDash() const;
    void useDash();
    void useDash(Player& player);
    void setSpeed(int newSpeed);

    bool isShieldActive() const { return shieldActive; }
    bool isSpeedBoostActive() const { return speedBoostActive; }
    bool isCoinMagnetActive() const { return coinMagnetActive; }
    bool isDashActive() const { return dashActive; }
    int getDashCharges() const { return dashCharges; }
};

#endif // POWERUP_H_INCLUDED

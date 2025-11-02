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

    PowerUp(int startX, int groundY, int gameSpeed, PowerUpType puType) {
        x = startX;
        speed = gameSpeed;
        type = puType;
        active = true;
        collected = false;
        width = 30;
        height = 30;
        animFrame = 0.0f;
        y = groundY - 80 - (rand() % 50); // Vị trí ngẫu nhiên trên không
    }

    void update() {
        x -= speed;
        animFrame += 0.1f; // Cập nhật animation
        if (x + width < 0) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active && !collected) {
            // Hiệu ứng floating
            float floatOffset = sin(animFrame) * 3.0f;
            int currentY = y + (int)floatOffset;

            SDL_Rect rect = { x, currentY - height, width, height };

            // Màu sắc và hiệu ứng dựa trên loại power-up
            switch (type) {
                case PowerUpType::SHIELD:
                    renderShieldEffect(renderer, rect);
                    break;
                case PowerUpType::SPEED_BOOST:
                    renderSpeedBoostEffect(renderer, rect);
                    break;
                case PowerUpType::COIN_MAGNET:
                    renderCoinMagnetEffect(renderer, rect);
                    break;
                case PowerUpType::DASH:
                    renderDashEffect(renderer, rect);
                    break;
            }
        }
    }

    void renderShieldEffect(SDL_Renderer* renderer, const SDL_Rect& rect) {
        SDL_SetRenderDrawColor(renderer, 0, 150, 255, 180);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 200, 230, 255, 100);
        SDL_Rect inner = { rect.x + 5, rect.y + 5, rect.w - 10, rect.h - 10 };
        SDL_RenderFillRect(renderer, &inner);
    }

    void renderSpeedBoostEffect(SDL_Renderer* renderer, const SDL_Rect& rect) {
        SDL_SetRenderDrawColor(renderer, 255, 200, 0, 200);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 100, 0, 150);
        for (int i = 0; i < 3; i++) {
            SDL_Rect flame = {
                rect.x - 5 - i*2,
                rect.y + rect.h/2 - 2,
                5 + i*2,
                4
            };
            SDL_RenderFillRect(renderer, &flame);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, rect.x + 8, rect.y + 8, rect.x + rect.w - 8, rect.y + rect.h - 8);
        SDL_RenderDrawLine(renderer, rect.x + rect.w - 8, rect.y + 8, rect.x + 8, rect.y + rect.h - 8);
    }

    void renderCoinMagnetEffect(SDL_Renderer* renderer, const SDL_Rect& rect) {
        SDL_SetRenderDrawColor(renderer, 200, 100, 255, 180);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 150, 50, 200, 255);
        for (int i = 0; i < 4; i++) {
            float angle = animFrame + i * M_PI / 2;
            int lineX = rect.x + rect.w/2 + (int)(cos(angle) * 8);
            int lineY = rect.y + rect.h/2 + (int)(sin(angle) * 8);
            SDL_RenderDrawLine(renderer, rect.x + rect.w/2, rect.y + rect.h/2, lineX, lineY);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, rect.x + 5, rect.y + rect.h/2, rect.x + rect.w - 5, rect.y + rect.h/2);
    }

    void renderDashEffect(SDL_Renderer* renderer, const SDL_Rect& rect) {
        SDL_SetRenderDrawColor(renderer, 150, 150, 200, 200);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 200, 200, 255, 150);
        for (int i = 0; i < 4; i++) {
            SDL_Rect wind = {
                rect.x - 3 - i*2,
                rect.y + i*3,
                3,
                rect.h - i*6
            };
            SDL_RenderFillRect(renderer, &wind);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, rect.x + 5, rect.y + rect.h/2, rect.x + rect.w - 5, rect.y + rect.h/2);
        SDL_RenderDrawLine(renderer, rect.x + rect.w - 10, rect.y + 5, rect.x + rect.w - 5, rect.y + rect.h/2);
        SDL_RenderDrawLine(renderer, rect.x + rect.w - 10, rect.y + rect.h - 5, rect.x + rect.w - 5, rect.y + rect.h/2);
    }

    bool checkCollision(int px, int py, int pwidth, int pheight) {
        if (!active || collected) return false;
        SDL_Rect a{ px, py - pheight, pwidth, pheight };
        SDL_Rect b{ x, y - height, width, height };
        return SDL_HasIntersection(&a, &b);
    }
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

    PowerUpManager(int ground, int gameSpeed, int width)
        : groundY(ground), speed(gameSpeed), screenWidth(width) {
        spawnTimer = 0;
        spawnInterval = 300;
        reset();
    }

    PowerUpManager(const PowerUpManager& other)
        : groundY(other.groundY), speed(other.speed), screenWidth(other.screenWidth) {
        spawnTimer = other.spawnTimer;
        spawnInterval = other.spawnInterval;
        reset();
    }

    void reset() {
        powerUps.clear();
        shieldActive = false;
        shieldTimer = 0;
        speedBoostActive = false;
        speedBoostTimer = 0;
        originalSpeed = 0;
        coinMagnetActive = false;
        coinMagnetTimer = 0;
        dashCharges = 0;
        dashCooldown = 0;
        dashActive = false;
        dashTimer = 0;
    }

    void update(Player& player) {
        update(player, nullptr);
    }

    void update(Player& player, ScoreManager* scoreManager) {
        for (auto& pu : powerUps) {
            pu.update();
            if (pu.checkCollision(player.x, player.y, player.width, player.height) && !pu.collected) {
                pu.collected = true;
                pu.active = false;
                activate(pu.type, player);
            }
        }

        powerUps.erase(
            std::remove_if(powerUps.begin(), powerUps.end(), [](const PowerUp& p) { return !p.active; }),
            powerUps.end()
        );

        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawn();
            spawnTimer = 0;
            spawnInterval = 400 + (rand() % 200);
        }

        updateEffects(player, scoreManager);
    }

    void activate(PowerUpType type, Player& player) {
        switch (type) {
            case PowerUpType::SHIELD:
                shieldActive = true;
                shieldTimer = 300;
                break;
            case PowerUpType::SPEED_BOOST:
                speedBoostActive = true;
                speedBoostTimer = 240;
                if (originalSpeed == 0) {
                    originalSpeed = static_cast<float>(speed);
                }
                speed = static_cast<int>(originalSpeed * 1.5f);
                break;
            case PowerUpType::COIN_MAGNET:
                coinMagnetActive = true;
                coinMagnetTimer = 360;
                break;
            case PowerUpType::DASH:
                dashCharges++;
                break;
        }
    }

    void updateEffects(Player& player, ScoreManager* scoreManager) {
        if (shieldActive) {
            shieldTimer--;
            if (shieldTimer <= 0) {
                shieldActive = false;
            }
        }

        if (speedBoostActive) {
            speedBoostTimer--;
            if (speedBoostTimer <= 0) {
                speedBoostActive = false;
                speed = static_cast<int>(originalSpeed);
                originalSpeed = 0;
            }
        }

        if (coinMagnetActive && scoreManager) {
            coinMagnetTimer--;
            applyCoinMagnetEffect(player, *scoreManager);
            if (coinMagnetTimer <= 0) {
                coinMagnetActive = false;
            }
        }

        if (dashActive) {
            dashTimer--;
            if (dashTimer <= 0) {
                dashActive = false;
            }
        }

        if (dashCooldown > 0) {
            dashCooldown--;
        }
    }

    void applyCoinMagnetEffect(Player& player, ScoreManager& scoreManager) {
        for (auto& coin : scoreManager.coins) {
            if (!coin.active || coin.collected) continue;

            int coinCenterX = coin.x + coin.width/2;
            int coinCenterY = coin.y - coin.height/2;
            int playerCenterX = player.x + player.width/2;
            int playerCenterY = player.y - player.height/2;

            int distanceX = coinCenterX - playerCenterX;
            int distanceY = coinCenterY - playerCenterY;
            float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

            if (distance <= MAGNET_RADIUS) {
                float pullSpeed = 5.0f + (MAGNET_RADIUS - distance) / 10.0f;

                if (distanceX > 0) {
                    coin.x -= std::min(static_cast<int>(pullSpeed), distanceX);
                } else {
                    coin.x += std::min(static_cast<int>(pullSpeed), -distanceX);
                }

                if (distanceY > 0) {
                    coin.y -= std::min(static_cast<int>(pullSpeed), distanceY);
                } else {
                    coin.y += std::min(static_cast<int>(pullSpeed), -distanceY);
                }
            }
        }
    }

    void spawn() {
        int typeIndex = rand() % 4;
        PowerUpType type = static_cast<PowerUpType>(typeIndex);
        powerUps.push_back(PowerUp(screenWidth, groundY, speed, type));
    }

    void render(SDL_Renderer* renderer) {
        for (auto& pu : powerUps) {
            pu.render(renderer);
        }
        renderActiveEffectsUI(renderer);
    }

    void renderActiveEffectsUI(SDL_Renderer* renderer) {
        int iconSize = 20;
        int startX = 10;
        int startY = 100;
        int spacing = 25;

        if (shieldActive) {
            SDL_Rect shieldIcon = { startX, startY, iconSize, iconSize };
            SDL_SetRenderDrawColor(renderer, 0, 150, 255, 200);
            SDL_RenderFillRect(renderer, &shieldIcon);
            float shieldPercent = (float)shieldTimer / 300.0f;
            int shieldWidth = (int)(iconSize * shieldPercent);
            SDL_Rect shieldTimeBar = { startX, startY + iconSize + 2, shieldWidth, 3 };
            SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
            SDL_RenderFillRect(renderer, &shieldTimeBar);
        }

        if (speedBoostActive) {
            SDL_Rect speedIcon = { startX, startY + spacing, iconSize, iconSize };
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 200);
            SDL_RenderFillRect(renderer, &speedIcon);
            float speedPercent = (float)speedBoostTimer / 240.0f;
            int speedWidth = (int)(iconSize * speedPercent);
            SDL_Rect speedTimeBar = { startX, startY + spacing + iconSize + 2, speedWidth, 3 };
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
            SDL_RenderFillRect(renderer, &speedTimeBar);
        }

        if (coinMagnetActive) {
            SDL_Rect magnetIcon = { startX, startY + spacing * 2, iconSize, iconSize };
            SDL_SetRenderDrawColor(renderer, 200, 100, 255, 200);
            SDL_RenderFillRect(renderer, &magnetIcon);
            float magnetPercent = (float)coinMagnetTimer / 360.0f;
            int magnetWidth = (int)(iconSize * magnetPercent);
            SDL_Rect magnetTimeBar = { startX, startY + spacing * 2 + iconSize + 2, magnetWidth, 3 };
            SDL_SetRenderDrawColor(renderer, 200, 100, 255, 255);
            SDL_RenderFillRect(renderer, &magnetTimeBar);
        }

        if (dashCharges > 0) {
            SDL_Rect dashIcon = { startX, startY + spacing * 3, iconSize, iconSize };
            SDL_SetRenderDrawColor(renderer, 150, 150, 200, 200);
            SDL_RenderFillRect(renderer, &dashIcon);
        }
    }

    bool canDash() const {
        return dashCharges > 0 && dashCooldown <= 0 && !dashActive;
    }

    void useDash() {
        Player tempPlayer;
        useDash(tempPlayer);
    }

    void useDash(Player& player) {
        if (canDash()) {
            dashCharges--;
            dashActive = true;
            dashTimer = 20;
            dashCooldown = 120;
            player.x += 150;
            if (player.x > screenWidth - player.width) {
                player.x = screenWidth - player.width;
            }
        }
    }

    void setSpeed(int newSpeed) {
        if (!speedBoostActive) {
            speed = newSpeed;
            originalSpeed = static_cast<float>(newSpeed);
        }
        for (auto& pu : powerUps) {
            pu.speed = speed;
        }
    }

    bool isShieldActive() const { return shieldActive; }
    bool isSpeedBoostActive() const { return speedBoostActive; }
    bool isCoinMagnetActive() const { return coinMagnetActive; }
    bool isDashActive() const { return dashActive; }
    int getDashCharges() const { return dashCharges; }
};

#endif // POWERUP_H_INCLUDED

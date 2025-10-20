#ifndef POWERUP_H_INCLUDED
#define POWERUP_H_INCLUDED

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <algorithm>
#include "player.h"

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

    PowerUp(int startX, int groundY, int gameSpeed, PowerUpType puType) {
        x = startX;
        speed = gameSpeed;
        type = puType;
        active = true;
        collected = false;
        width = 30;
        height = 30;
        y = groundY - 80 - (rand() % 50); // Vị trí ngẫu nhiên trên không
    }

    void update() {
        x -= speed;
        if (x + width < 0) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active && !collected) {
            SDL_Rect rect = { x, y - height, width, height };
            switch (type) {
                case PowerUpType::SHIELD:
                    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255); // Xanh dương
                    break;
                case PowerUpType::SPEED_BOOST:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Vàng
                    break;
                case PowerUpType::COIN_MAGNET:
                     SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // Tím
                    break;
                case PowerUpType::DASH:
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Xám
                    break;
            }
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &rect);
        }
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

    int dashCharges;
    int dashCooldown;


    PowerUpManager(int ground, int gameSpeed, int width)
        : groundY(ground), speed(gameSpeed), screenWidth(width) {
        spawnTimer = 0;
        spawnInterval = 300; // Power-up xuất hiện hiếm hơn
        reset();
    }

    void reset() {
        powerUps.clear();
        shieldActive = false;
        shieldTimer = 0;
        dashCharges = 0;
        dashCooldown = 0;
    }

    void update(Player& player) {
        // Cập nhật các power-up đang có trên màn hình
        for (auto& pu : powerUps) {
            pu.update();
            if (pu.checkCollision(player.x, player.y, player.width, player.height) && !pu.collected) {
                pu.collected = true;
                pu.active = false;
                activate(pu.type);
            }
        }

        powerUps.erase(
            std::remove_if(powerUps.begin(), powerUps.end(), [](const PowerUp& p) { return !p.active; }),
            powerUps.end()
        );

        // Spawn power-up mới
        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawn();
            spawnTimer = 0;
            spawnInterval = 400 + (rand() % 200); // 400-600 frames
        }

        // Cập nhật hiệu ứng
        if (shieldActive) {
            shieldTimer--;
            if (shieldTimer <= 0) {
                shieldActive = false;
            }
        }
        if (dashCooldown > 0) {
            dashCooldown--;
        }
    }

    void activate(PowerUpType type) {
        switch (type) {
            case PowerUpType::SHIELD:
                shieldActive = true;
                shieldTimer = 300; // 5 giây
                break;
            case PowerUpType::DASH:
                dashCharges++;
                break;
            // Các power-up khác sẽ được thêm logic sau
            case PowerUpType::SPEED_BOOST:
                break;
            case PowerUpType::COIN_MAGNET:
                break;
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
        // Có thể vẽ UI cho hiệu ứng tại đây (vd: icon khiên)
    }

    bool canDash() const {
        return dashCharges > 0 && dashCooldown <= 0;
    }

    void useDash() {
        if (canDash()) {
            dashCharges--;
            dashCooldown = 60; // 1 giây cooldown
        }
    }

    void setSpeed(int newSpeed) {
        speed = newSpeed;
        for (auto& pu : powerUps) {
            pu.speed = newSpeed;
        }
    }
};

#endif // POWERUP_H_INCLUDED

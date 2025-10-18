#ifndef SCORE_H_INCLUDED
#define SCORE_H_INCLUDED

#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include "player.h" // Include player.h to access the Player struct

// Loại điểm thưởng
enum CoinType {
    NORMAL_COIN,   // Xu vàng thường (+10 điểm)
    SILVER_COIN,   // Xu bạc (+5 điểm)
    GOLD_COIN,     // Xu vàng đặc biệt (+20 điểm)
    XP_COIN        // Xu kinh nghiệm (+25 XP)
};

class Coin {
public:
    int x, y;
    int width, height;
    int speed;
    bool active;
    bool collected;
    CoinType type;
    int value;
    int xpValue; // XP awarded by this coin

    // Hiệu ứng animation
    float animFrame;
    float animSpeed;

    Coin(int startX, int groundY, int coinSpeed, CoinType coinType = NORMAL_COIN) {
        x = startX;
        speed = coinSpeed;
        active = true;
        collected = false;
        type = coinType;

        width = 20;
        height = 20;
        xpValue = 0; // Default to 0 XP

        animFrame = 0;
        animSpeed = 0.2f;

        // Xác định giá trị và vị trí dựa trên loại xu
        if (type == SILVER_COIN) {
            value = 5;
            y = groundY - 60;  // Thấp hơn
        } else if (type == GOLD_COIN) {
            value = 20;
            width = 25;
            height = 25;
            y = groundY - 130; // Cao nhất - khó lấy nhất
        } else if (type == XP_COIN) {
            value = 0; // No score value
            xpValue = 25;
            width = 22;
            height = 22;
            y = groundY - 90;
        } else { // NORMAL_COIN
            value = 10;
            // Random độ cao
            int heightLevel = rand() % 3;
            if (heightLevel == 0) {
                y = groundY - 70;
            } else if (heightLevel == 1) {
                y = groundY - 100;
            } else {
                y = groundY - 120;
            }
        }
    }

    void update() {
        x -= speed;
        animFrame += animSpeed;
        if (animFrame >= 360) animFrame = 0;

        if (x + width < 0) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active && !collected) {
            // Hiệu ứng nhấp nháy/quay
            int offset = (int)(sin(animFrame * 0.1f) * 3);

            if (type == SILVER_COIN) {
                // Xu bạc (màu xám bạc)
                SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
            } else if (type == GOLD_COIN) {
                // Xu vàng đặc biệt (vàng óng ánh)
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
            } else if (type == XP_COIN) {
                // Xu XP (màu tím)
                SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
            }
            else {
                // Xu thường (vàng)
                SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
            }

            SDL_Rect rect = { x, y - height + offset, width, height };
            SDL_RenderFillRect(renderer, &rect);

            // Viền
            SDL_SetRenderDrawColor(renderer, 180, 140, 0, 255);
            SDL_RenderDrawRect(renderer, &rect);

            // Vẽ ký hiệu $ hoặc hình tròn bên trong
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect inner = { x + width/4, y - height + offset + height/4,
                              width/2, height/2 };
            SDL_RenderFillRect(renderer, &inner);
        }
    }

    bool checkCollision(int px, int py, int pwidth, int pheight) {
        if (!active || collected) return false;

        SDL_Rect a{ px,        py - pheight, pwidth,  pheight }; // player
        SDL_Rect b{ x,         y  - height,  width,   height  }; // obstacle

        return SDL_HasIntersection(&a, &b) == SDL_TRUE;
    }
};

class ScoreManager {
public:
    int currentScore;
    int highScore;
    int distanceScore;      // Điểm từ quãng đường
    int coinScore;          // Điểm từ xu
    int totalCoinsCollected;

    std::vector<Coin> coins;
    int spawnTimer;
    int spawnInterval;
    int groundY;
    int speed;
    int screenWidth;
    int distanceCounter;    // Đếm frame để tính quãng đường

    ScoreManager(int ground, int gameSpeed, int width) {
        groundY = ground;
        speed = gameSpeed;
        screenWidth = width;
        spawnTimer = 0;
        spawnInterval = 120; // Spawn xu ít hơn chướng ngại vật

        currentScore = 0;
        highScore = 0;
        distanceScore = 0;
        coinScore = 0;
        totalCoinsCollected = 0;
        distanceCounter = 0;

        srand(time(NULL));
    }

    void update(Player& player) { // Pass player by reference
        // Cập nhật xu
        for (auto& coin : coins) {
            coin.update();

            // Kiểm tra va chạm với player
            if (coin.checkCollision(player.x, player.y, player.width, player.height)) {
                if (!coin.collected) {
                    coin.collected = true;
                    coin.active = false;
                    coinScore += coin.value;        // Cộng điểm màn chơi hiện tại
                    currentScore += coin.value;     // Cộng điểm màn chơi hiện tại
                    player.totalCoins++;            // Tăng tổng số xu của người chơi
                    player.addXp(coin.xpValue);
                }
            }
        }

        // Xóa xu không còn active
        coins.erase(
            std::remove_if(coins.begin(), coins.end(),
                [](const Coin& c) { return !c.active; }),
            coins.end()
        );

        // Spawn xu mới
        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawnCoin();
            spawnTimer = 0;
            spawnInterval = 100 + (rand() % 80); // Random 100-180 frames
        }

        // Tính điểm quãng đường (mỗi 30 frames = 1 điểm)
        distanceCounter++;
        if (distanceCounter >= 30) {
            distanceScore++;
            currentScore++;
            distanceCounter = 0;
        }

        // Cập nhật high score
        if (currentScore > highScore) {
            highScore = currentScore;
        }
    }

    void spawnCoin() {
        // 45% normal, 25% silver, 15% gold, 15% XP
        int randVal = rand() % 100;
        CoinType type;

        if (randVal < 45) {
            type = NORMAL_COIN;
        } else if (randVal < 70) {
            type = SILVER_COIN;
        } else if (randVal < 85) {
            type = GOLD_COIN;
        } else {
            type = XP_COIN;
        }

        coins.push_back(Coin(screenWidth, groundY, speed, type));
    }

    void render(SDL_Renderer* renderer) {
        for (auto& coin : coins) {
            coin.render(renderer);
        }
    }

    void reset() {
        coins.clear();
        spawnTimer = 0;
        currentScore = 0;
        distanceScore = 0;
        coinScore = 0;
        totalCoinsCollected = 0;
        distanceCounter = 0;
    }

    void setSpeed(int newSpeed) {
        speed = newSpeed;
        for (auto& coin : coins) {
            coin.speed = newSpeed;
        }
    }

    // Lấy điểm hiện tại
    int getCurrentScore() const { return currentScore; }

    // Lấy high score
    int getHighScore() const { return highScore; }

    // Lấy điểm từ quãng đường
    int getDistanceScore() const { return distanceScore; }

    // Lấy điểm từ xu
    int getCoinScore() const { return coinScore; }

    // Lấy số xu đã thu thập
    int getTotalCoins() const { return totalCoinsCollected; }
};

#endif // SCORE_H_INCLUDED

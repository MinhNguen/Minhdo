#ifndef SCORE_H_INCLUDED
#define SCORE_H_INCLUDED

#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

// Loại điểm thưởng
enum CoinType {
    NORMAL_COIN,   // Xu vàng thường (+10 điểm)
    SILVER_COIN,   // Xu bạc (+5 điểm)
    GOLD_COIN      // Xu vàng đặc biệt (+20 điểm)
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
            } else {
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

        return (px < x + width &&
                px + pwidth > x &&
                py < y + height &&
                py + pheight > y);
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

    void update(int playerX, int playerY, int playerWidth, int playerHeight) {
        // Cập nhật xu
        for (auto& coin : coins) {
            coin.update();

            // Kiểm tra va chạm với player
            if (coin.checkCollision(playerX, playerY, playerWidth, playerHeight)) {
                if (!coin.collected) {
                    coin.collected = true;
                    coin.active = false;
                    coinScore += coin.value;
                    currentScore += coin.value;
                    totalCoinsCollected++;
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
        // 50% xu thường, 30% xu bạc, 20% xu vàng
        int randVal = rand() % 100;
        CoinType type;

        if (randVal < 50) {
            type = NORMAL_COIN;
        } else if (randVal < 80) {
            type = SILVER_COIN;
        } else {
            type = GOLD_COIN;
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

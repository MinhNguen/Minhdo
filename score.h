#ifndef SCORE_H_INCLUDED
#define SCORE_H_INCLUDED

#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include "player.h"

enum CoinType {
    NORMAL_COIN,
    SILVER_COIN,
    GOLD_COIN,
    XP_COIN
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
    int xpValue;
    float animFrame;
    float animSpeed;

    Coin(int startX, int groundY, int coinSpeed, CoinType coinType = NORMAL_COIN) {
        x = startX;
        speed = coinSpeed;
        active = true;
        collected = false;
        type = coinType;
        width = 20; height = 20;
        xpValue = 0;
        animFrame = 0; animSpeed = 0.2f;

        if (type == SILVER_COIN) { value = 5; y = groundY - 60; }
        else if (type == GOLD_COIN) { value = 20; width = 25; height = 25; y = groundY - 130; }
        else if (type == XP_COIN) { value = 0; xpValue = 25; width = 22; height = 22; y = groundY - 90; }
        else { // NORMAL_COIN
            value = 10;
            int heightLevel = rand() % 3;
            if (heightLevel == 0) y = groundY - 70;
            else if (heightLevel == 1) y = groundY - 100;
            else y = groundY - 120;
        }
    }

    void update() {
        x -= speed;
        animFrame += animSpeed;
        if (animFrame >= 360) animFrame = 0;
        if (x + width < 0) active = false;
    }

    void render(SDL_Renderer* renderer) {
        if (active && !collected) {
            int offset = (int)(sin(animFrame * 0.1f) * 3);
            if (type == SILVER_COIN) SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
            else if (type == GOLD_COIN) SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
            else if (type == XP_COIN) SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
            else SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
            SDL_Rect rect = { x, y - height + offset, width, height };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    bool checkCollision(int px, int py, int pwidth, int pheight) {
        if (!active || collected) return false;
        SDL_Rect a{ px, py - pheight, pwidth, pheight };
        SDL_Rect b{ x, y - height, width, height };
        return SDL_HasIntersection(&a, &b);
    }
};

class ScoreManager {
public:
    int currentScore, highScore, distanceScore, coinScore, totalCoinsCollected;
    std::vector<Coin> coins;
    int spawnTimer, spawnInterval, groundY, speed, screenWidth, distanceCounter;

    ScoreManager(int ground, int gameSpeed, int width) {
        groundY = ground; speed = gameSpeed; screenWidth = width;
        spawnTimer = 0; spawnInterval = 120;
        distanceCounter = 0;
        reset();
        srand(time(NULL));
    }

    void reset() {
        coins.clear();
        spawnTimer = 0;
        currentScore = 0; highScore = 0; distanceScore = 0;
        coinScore = 0; totalCoinsCollected = 0; distanceCounter = 0;
    }

    void update(Player& player) {
        for (auto& coin : coins) {
            coin.update();
            if (coin.checkCollision(player.x, player.y, player.width, player.height) && !coin.collected) {
                coin.collected = true;
                coin.active = false;
                coinScore += coin.value;
                currentScore += coin.value;
                player.totalCoins++;
                player.addXp(coin.xpValue);
            }
        }
        coins.erase(std::remove_if(coins.begin(), coins.end(), [](const Coin& c) { return !c.active; }), coins.end());

        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawnCoin();
            spawnTimer = 0;
            spawnInterval = 100 + (rand() % 80);
        }

        distanceCounter++;
        if (distanceCounter >= 30) {
            distanceScore++; currentScore++; distanceCounter = 0;
        }
        if (currentScore > highScore) highScore = currentScore;
    }

    void spawnCoin() {
        int randVal = rand() % 100;
        CoinType type = NORMAL_COIN;
        if (randVal < 45) type = NORMAL_COIN;
        else if (randVal < 70) type = SILVER_COIN;
        else if (randVal < 85) type = GOLD_COIN;
        else type = XP_COIN;
        coins.emplace_back(screenWidth, groundY, speed, type);
    }

    void render(SDL_Renderer* renderer) {
        for (auto& coin : coins) coin.render(renderer);
    }

    void setSpeed(int newSpeed) {
        speed = newSpeed;
        for (auto& coin : coins) coin.speed = newSpeed;
    }

    int getCurrentScore() const { return currentScore; }
};

#endif // SCORE_H_INCLUDED

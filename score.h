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
    float rotation;
    float scale;
    bool scalingUp;
    float glowIntensity;

    // Constructor
    Coin(int startX, int groundY, int coinSpeed, CoinType coinType = NORMAL_COIN);

    // Public methods
    void update();
    void render(SDL_Renderer* renderer);
    bool checkCollision(int px, int py, int pwidth, int pheight);

private:
    // Private helper methods
    void setupCoin(int groundY);
    void renderGlow(SDL_Renderer* renderer, int x, int y, int w, int h);
    void renderCoinBody(SDL_Renderer* renderer, int x, int y, int w, int h);
    void renderShine(SDL_Renderer* renderer, int x, int y, int w, int h);
    void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius);
    void fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius);
    void drawGradientCircle(SDL_Renderer* renderer, int cx, int cy, int radius,
                          SDL_Color centerColor, SDL_Color edgeColor);
};

class ScoreManager {
public:
    int currentScore, highScore, distanceScore, coinScore, totalCoinsCollected;
    std::vector<Coin> coins;
    int spawnTimer, spawnInterval, groundY, speed, screenWidth, distanceCounter;

    // Constructor
    ScoreManager(int ground, int gameSpeed, int width);

    // Public methods
    void reset();
    void update(Player& player);
    void render(SDL_Renderer* renderer);
    void setSpeed(int newSpeed);
    int getCurrentScore() const;

private:
    // Private helper methods
    void spawnCoin();
};

#endif // SCORE_H_INCLUDED

#include "score.h"
#include <iostream>

// ===================== COIN CLASS IMPLEMENTATION =====================

Coin::Coin(int startX, int groundY, int coinSpeed, CoinType coinType) {
    x = startX;
    speed = coinSpeed;
    active = true;
    collected = false;
    type = coinType;
    animFrame = 0;
    animSpeed = 0.15f;
    rotation = 0;
    scale = 1.0f;
    scalingUp = true;
    glowIntensity = 0.0f;

    setupCoin(groundY);
}

void Coin::setupCoin(int groundY) {
    if (type == SILVER_COIN) {
        value = 5;
        width = 22; height = 22;
        y = groundY - 60;
    }
    else if (type == GOLD_COIN) {
        value = 20;
        width = 28; height = 28;
        y = groundY - 130;
    }
    else if (type == XP_COIN) {
        value = 0;
        xpValue = 25;
        width = 24; height = 24;
        y = groundY - 90;
    }
    else { // NORMAL_COIN
        value = 10;
        width = 20; height = 20;
        int heightLevel = rand() % 3;
        if (heightLevel == 0) y = groundY - 70;
        else if (heightLevel == 1) y = groundY - 100;
        else y = groundY - 120;
    }
}

void Coin::update() {
    x -= speed;
    animFrame += animSpeed;
    rotation += 2.0f;
    if (rotation >= 360) rotation = 0;

    // Pulsing scale effect
    if (scalingUp) {
        scale += 0.02f;
        if (scale >= 1.2f) scalingUp = false;
    } else {
        scale -= 0.02f;
        if (scale <= 0.8f) scalingUp = true;
    }

    // Glow effect
    glowIntensity = 0.5f + 0.5f * sin(animFrame * 2.0f);

    if (x + width < 0) active = false;
}

void Coin::render(SDL_Renderer* renderer) {
    if (active && !collected) {
        // Floating animation
        float floatOffset = sin(animFrame) * 5.0f;
        int currentY = y + (int)floatOffset;

        // Calculate scaled dimensions
        int scaledWidth = (int)(width * scale);
        int scaledHeight = (int)(height * scale);
        int drawX = x + (width - scaledWidth) / 2;
        int drawY = currentY - scaledHeight + (height - scaledHeight) / 2;

        // Render glow effect
        renderGlow(renderer, drawX, drawY, scaledWidth, scaledHeight);

        // Render coin body
        renderCoinBody(renderer, drawX, drawY, scaledWidth, scaledHeight);

        // Render shine effect
        renderShine(renderer, drawX, drawY, scaledWidth, scaledHeight);
    }
}

void Coin::renderGlow(SDL_Renderer* renderer, int x, int y, int w, int h) {
    SDL_Color glowColor;
    int baseAlpha = (int)(80 * glowIntensity);

    switch (type) {
        case SILVER_COIN:
            glowColor = {200, 200, 255, (Uint8)baseAlpha};
            break;
        case GOLD_COIN:
            glowColor = {255, 255, 100, (Uint8)baseAlpha};
            break;
        case XP_COIN:
            glowColor = {200, 100, 255, (Uint8)baseAlpha};
            break;
        default: // NORMAL_COIN
            glowColor = {255, 220, 100, (Uint8)baseAlpha};
            break;
    }

    // Draw multiple concentric circles for glow
    for (int i = 3; i >= 1; i--) {
        int glowSize = i * 4;
        SDL_SetRenderDrawColor(renderer, glowColor.r, glowColor.g, glowColor.b, glowColor.a / i);
        drawCircle(renderer, x + w/2, y + h/2, w/2 + glowSize);
    }
}

void Coin::renderCoinBody(SDL_Renderer* renderer, int x, int y, int w, int h) {
    SDL_Color baseColor, highlightColor, shadowColor;

    switch (type) {
        case SILVER_COIN:
            baseColor = {192, 192, 192, 255};
            highlightColor = {240, 240, 240, 255};
            shadowColor = {150, 150, 150, 255};
            break;
        case GOLD_COIN:
            baseColor = {255, 215, 0, 255};
            highlightColor = {255, 255, 150, 255};
            shadowColor = {205, 175, 0, 255};
            break;
        case XP_COIN:
            baseColor = {138, 43, 226, 255}; // Violet
            highlightColor = {186, 85, 211, 255}; // Orchid
            shadowColor = {75, 0, 130, 255}; // Indigo
            break;
        default: // NORMAL_COIN
            baseColor = {255, 200, 0, 255};
            highlightColor = {255, 255, 100, 255};
            shadowColor = {205, 150, 0, 255};
            break;
    }

    // Main coin body with gradient
    drawGradientCircle(renderer, x + w/2, y + h/2, w/2, baseColor, highlightColor);

    // Coin edge
    SDL_SetRenderDrawColor(renderer, shadowColor.r, shadowColor.g, shadowColor.b, 255);
    drawCircle(renderer, x + w/2, y + h/2, w/2);

    // Inner circle for detail
    SDL_SetRenderDrawColor(renderer, shadowColor.r, shadowColor.g, shadowColor.b, 150);
    drawCircle(renderer, x + w/2, y + h/2, w/3);
}

void Coin::renderShine(SDL_Renderer* renderer, int x, int y, int w, int h) {
    // Shine effect based on rotation
    float shineAngle = rotation * M_PI / 180.0f;
    int shineX = x + w/2 + (int)(cos(shineAngle) * (w/4));
    int shineY = y + h/2 + (int)(sin(shineAngle) * (h/4));
    int shineSize = w/4;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    fillCircle(renderer, shineX, shineY, shineSize);

    // Additional small shine spots
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
    for (int i = 0; i < 2; i++) {
        float spotAngle = shineAngle + M_PI + (i * M_PI/2);
        int spotX = x + w/2 + (int)(cos(spotAngle) * (w/3));
        int spotY = y + h/2 + (int)(sin(spotAngle) * (h/3));
        fillCircle(renderer, spotX, spotY, shineSize/2);
    }
}

void Coin::drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void Coin::fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

void Coin::drawGradientCircle(SDL_Renderer* renderer, int cx, int cy, int radius,
                          SDL_Color centerColor, SDL_Color edgeColor) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            float distance = sqrt(x*x + y*y);
            if (distance <= radius) {
                float t = distance / radius;
                SDL_Color color;
                color.r = centerColor.r + (edgeColor.r - centerColor.r) * t;
                color.g = centerColor.g + (edgeColor.g - centerColor.g) * t;
                color.b = centerColor.b + (edgeColor.b - centerColor.b) * t;
                color.a = centerColor.a + (edgeColor.a - centerColor.a) * t;

                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

bool Coin::checkCollision(int px, int py, int pwidth, int pheight) {
    if (!active || collected) return false;

    // Simple circle-rectangle collision
    int coinCenterX = x + width/2;
    int coinCenterY = y - height/2;
    int coinRadius = width/2;

    int closestX = std::max(px, std::min(coinCenterX, px + pwidth));
    int closestY = std::max(py - pheight, std::min(coinCenterY, py));

    int distanceX = coinCenterX - closestX;
    int distanceY = coinCenterY - closestY;

    return (distanceX * distanceX + distanceY * distanceY) <= (coinRadius * coinRadius);
}

// ===================== SCORE MANAGER IMPLEMENTATION =====================

ScoreManager::ScoreManager(int ground, int gameSpeed, int width) {
    groundY = ground; speed = gameSpeed; screenWidth = width;
    spawnTimer = 0; spawnInterval = 120;
    distanceCounter = 0;
    reset();
    srand(time(NULL));
}

void ScoreManager::reset() {
    coins.clear();
    spawnTimer = 0;
    currentScore = 0; highScore = 0; distanceScore = 0;
    coinScore = 0; totalCoinsCollected = 0; distanceCounter = 0;
}

void ScoreManager::update(Player& player) {
    for (auto& coin : coins) {
        coin.update();
        if (coin.checkCollision(player.x, player.y, player.width, player.height) && !coin.collected) {
            coin.collected = true;
            coin.active = false;
            coinScore += coin.value;
            currentScore += coin.value;
            player.totalCoins++;
            player.addXp(coin.xpValue);
            totalCoinsCollected++;
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

void ScoreManager::spawnCoin() {
    int randVal = rand() % 100;
    CoinType type = NORMAL_COIN;
    if (randVal < 45) type = NORMAL_COIN;
    else if (randVal < 70) type = SILVER_COIN;
    else if (randVal < 85) type = GOLD_COIN;
    else type = XP_COIN;
    coins.emplace_back(screenWidth, groundY, speed, type);
}

void ScoreManager::render(SDL_Renderer* renderer) {
    for (auto& coin : coins) coin.render(renderer);
}

void ScoreManager::setSpeed(int newSpeed) {
    speed = newSpeed;
    for (auto& coin : coins) coin.speed = newSpeed;
}

int ScoreManager::getCurrentScore() const {
    return currentScore;
}

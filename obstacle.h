#ifndef OBSTACLE_H_INCLUDED
#define OBSTACLE_H_INCLUDED

#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

// Loại chướng ngại vật
enum ObstacleType {
    GROUND_OBSTACLE,  // Chướng ngại vật đất (xương rồng)
    FLYING_OBSTACLE,  // Chướng ngại vật bay (chim)
    METEOR            // Thiên thạch từ trên trời rơi xuống
};

class Obstacle {
public:
    int x, y;
    int width, height;
    int speed;
    bool active;
    ObstacleType type;

    // Cho thiên thạch
    float vy;           // Vận tốc rơi xuống
    float rotation;     // Góc xoay
    float rotationSpeed;
    int trailTimer;     // Hiệu ứng đuôi lửa

    Obstacle(int startX, int groundY, int obstacleSpeed, ObstacleType obsType = GROUND_OBSTACLE) {
        speed = obstacleSpeed;
        active = true;
        type = obsType;
        rotation = 0;
        rotationSpeed = 5.0f + (rand() % 10);
        trailTimer = 0;

        if (type == GROUND_OBSTACLE) {
            x = startX;
            width = 20 + (rand() % 25);
            height = 50 + (rand() % 30);
            y = groundY;
            vy = 0;
        } else if (type == FLYING_OBSTACLE) {
            x = startX;
            width = 35 + (rand() % 25);
            height = 25 + (rand() % 20);
            vy = 0;
            int heightLevel = rand() % 3;
            if (heightLevel == 0) y = groundY - 50;
            else if (heightLevel == 1) y = groundY - 100;
            else y = groundY - 120;
        } else { // METEOR
            x = startX;
            width = 30 + (rand() % 20);
            height = 30 + (rand() % 20);
            y = -50;
            vy = 3.0f + (rand() % 4);
        }
    }

    void update() {
        if (type == METEOR) {
            y += vy;
            x -= speed / 2;
            rotation += rotationSpeed;
            trailTimer++;
            if (vy < 12) vy += 0.1f;
            if (y > 500 || x + width < 0) active = false;
        } else {
            x -= speed;
            if (x + width < 0) active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (!active) return;
        if (type == GROUND_OBSTACLE) {
            SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
            SDL_Rect rect = { x, y - height, width, height };
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &rect);
        } else if (type == FLYING_OBSTACLE) {
            SDL_SetRenderDrawColor(renderer, 255, 100, 50, 255);
            SDL_Rect rect = { x, y - height, width, height };
            SDL_RenderFillRect(renderer, &rect);
        } else { // METEOR
            if (trailTimer % 2 == 0) {
                for (int i = 1; i <= 3; i++) {
                    SDL_SetRenderDrawColor(renderer, 255, 150 - i * 30, 0, 255 - (i * 60));
                    SDL_Rect trail = { x + width / 2 - 10, y - height / 2 - (i * 15), 20 - i * 3, 15 };
                    SDL_RenderFillRect(renderer, &trail);
                }
            }
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            SDL_Rect meteor = { x, y - height, width, height };
            SDL_RenderFillRect(renderer, &meteor);
            SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);
            SDL_Rect border1 = { x - 2, y - height - 2, width + 4, height + 4 };
            SDL_RenderDrawRect(renderer, &border1);
        }
    }

    bool checkCollision(int px, int py, int pwidth, int pheight) {
        if (!active) return false;
        SDL_Rect a{ px, py - pheight, pwidth, pheight };
        SDL_Rect b{ x, y - height, width, height };
        return SDL_HasIntersection(&a, &b);
    }
};

class ObstacleManager {
public:
    std::vector<Obstacle> obstacles;
    int spawnTimer;
    int spawnInterval;
    int meteorTimer;
    int meteorInterval;
    int groundY;
    int speed;
    int screenWidth;

    ObstacleManager(int ground, int gameSpeed, int width) {
        groundY = ground;
        speed = gameSpeed;
        screenWidth = width;
        spawnTimer = 0;
        spawnInterval = 90;
        meteorTimer = 0;
        meteorInterval = 180;
        srand(time(NULL));
    }

    void update() {
        for (auto& obs : obstacles) {
            obs.update();
        }
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(), [](const Obstacle& o) { return !o.active; }),
            obstacles.end()
        );
        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawnObstacle();
            spawnTimer = 0;
            spawnInterval = 60 + (rand() % 60);
        }
        meteorTimer++;
        if (meteorTimer >= meteorInterval) {
            spawnMeteor();
            meteorTimer = 0;
            meteorInterval = 150 + (rand() % 100);
        }
    }

    void spawnObstacle() {
        ObstacleType type = (rand() % 100 < 40) ? FLYING_OBSTACLE : GROUND_OBSTACLE;
        obstacles.emplace_back(screenWidth, groundY, speed, type);
    }

    void spawnMeteor() {
        int meteorX = 100 + (rand() % (screenWidth - 200));
        obstacles.emplace_back(meteorX, groundY, speed, METEOR);
    }

    void render(SDL_Renderer* renderer) {
        for (auto& obs : obstacles) {
            obs.render(renderer);
        }
    }

    bool checkCollisionWithPlayer(int px, int py, int pwidth, int pheight) {
        for (auto& obs : obstacles) {
            if (obs.checkCollision(px, py, pwidth, pheight)) {
                return true;
            }
        }
        return false;
    }

    void clear() {
        obstacles.clear();
        spawnTimer = 0;
        meteorTimer = 0;
    }

    void setSpeed(int newSpeed) {
        speed = newSpeed;
        for (auto& obs : obstacles) {
            if (obs.type != METEOR) {
                obs.speed = newSpeed;
            }
        }
    }
};

#endif // OBSTACLE_H_INCLUDED

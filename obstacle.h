#ifndef OBSTACLE_H_INCLUDED
#define OBSTACLE_H_INCLUDED

#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// Loại chướng ngại vật
enum ObstacleType {
    GROUND_OBSTACLE,  // Chướng ngại vật đất (xương rồng)
    FLYING_OBSTACLE   // Chướng ngại vật bay (chim)
};

class Obstacle {
public:
    int x, y;
    int width, height;
    int speed;
    bool active;
    ObstacleType type;

    Obstacle(int startX, int groundY, int obstacleSpeed, ObstacleType obsType = GROUND_OBSTACLE) {
        x = startX;
        speed = obstacleSpeed;
        active = true;
        type = obsType;

        if (type == GROUND_OBSTACLE) {
            // Chướng ngại vật đất
            width = 20 + (rand() % 30);   // 20-50
            height = 30 + (rand() % 40);  // 30-70
            y = groundY - height;
        } else {
            // Chướng ngại vật bay
            width = 35 + (rand() % 25);   // 35-60 (rộng hơn)
            height = 25 + (rand() % 20);  // 25-45

            // 3 độ cao khác nhau
            int heightLevel = rand() % 3;
            if (heightLevel == 0) {
                y = groundY - 80;  // Thấp (có thể nhảy qua)
            } else if (heightLevel == 1) {
                y = groundY - 130; // Trung bình (phải nhảy cao)
            } else {
                y = groundY - 180; // Cao (phải cúi xuống)
            }
        }
    }

    void update() {
        x -= speed;

        if (x + width < 0) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            if (type == GROUND_OBSTACLE) {
                // Vẽ xương rồng (màu xanh đậm)
                SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
                SDL_Rect rect = { x, y, width, height };
                SDL_RenderFillRect(renderer, &rect);

                // Viền đen
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);
            } else {
                // Vẽ chim (màu đỏ cam)
                SDL_SetRenderDrawColor(renderer, 255, 100, 50, 255);
                SDL_Rect rect = { x, y, width, height };
                SDL_RenderFillRect(renderer, &rect);

                // Viền đen
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);

                // Vẽ cánh đơn giản
                SDL_SetRenderDrawColor(renderer, 200, 80, 40, 255);
                SDL_Rect wing1 = { x + 5, y - 5, 10, 5 };
                SDL_Rect wing2 = { x + width - 15, y - 5, 10, 5 };
                SDL_RenderFillRect(renderer, &wing1);
                SDL_RenderFillRect(renderer, &wing2);
            }
        }
    }

    bool checkCollision(int px, int py, int pwidth, int pheight) {
        if (!active) return false;

        return (px < x + width &&
                px + pwidth > x &&
                py < y + height &&
                py + pheight > y);
    }
};

class ObstacleManager {
public:
    std::vector<Obstacle> obstacles;
    int spawnTimer;
    int spawnInterval;
    int groundY;
    int speed;
    int screenWidth;

    ObstacleManager(int ground, int gameSpeed, int width) {
        groundY = ground;
        speed = gameSpeed;
        screenWidth = width;
        spawnTimer = 0;
        spawnInterval = 90;
        srand(time(NULL));
    }

    void update() {
        for (auto& obs : obstacles) {
            obs.update();
        }

        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(),
                [](const Obstacle& o) { return !o.active; }),
            obstacles.end()
        );

        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawnObstacle();
            spawnTimer = 0;
            spawnInterval = 60 + (rand() % 60);
        }
    }

    void spawnObstacle() {
        // 40% cơ hội spawn chướng ngại vật bay
        ObstacleType type = (rand() % 100 < 40) ? FLYING_OBSTACLE : GROUND_OBSTACLE;
        obstacles.push_back(Obstacle(screenWidth, groundY, speed, type));
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
    }

    void setSpeed(int newSpeed) {
        speed = newSpeed;
        for (auto& obs : obstacles) {
            obs.speed = newSpeed;
        }
    }
};

#endif // OBSTACLE_H_INCLUDED

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
            // Chướng ngại vật đất - THẤP HƠN, BẮT BUỘC PHẢI NHẢY
            x = startX;
            width = 20 + (rand() % 25);   // 20-45
            height = 50 + (rand() % 30);  // 50-80 (cao hơn player)
            y = groundY;
            vy = 0;
        } else if (type == FLYING_OBSTACLE) {
            // Chướng ngại vật bay
            x = startX;
            width = 35 + (rand() % 25);   // 35-60 (rộng hơn)
            height = 25 + (rand() % 20);  // 25-45
            vy = 0;

            // 3 độ cao khác nhau
            int heightLevel = rand() % 3;
            if (heightLevel == 0) {
                y = groundY - 50;  // Thấp (có thể nhảy qua)
            } else if (heightLevel == 1) {
                y = groundY - 100; // Trung bình (phải nhảy cao)
            } else {
                y = groundY - 120; // Cao (phải cúi xuống)
            }
        } else { // METEOR
            // Thiên thạch rơi từ trên xuống
            x = startX;
            width = 30 + (rand() % 20);   // 30-50
            height = 30 + (rand() % 20);  // 30-50
            y = -50; // Bắt đầu từ trên đỉnh màn hình
            vy = 3.0f + (rand() % 4);     // Tốc độ rơi 3-7
        }
    }

    void update() {
        if (type == METEOR) {
            // Thiên thạch rơi xuống và di chuyển sang trái
            y += vy;
            x -= speed / 2; // Di chuyển chậm hơn để dễ né hơn
            rotation += rotationSpeed;
            trailTimer++;

            // Tăng tốc độ rơi dần
            if (vy < 12) {
                vy += 0.1f;
            }

            // Vô hiệu hóa khi chạm đất hoặc ra khỏi màn hình
            if (y > 500 || x + width < 0) {
                active = false;
            }
        } else {
            // Chướng ngại vật thường
            x -= speed;
            if (x + width < 0) {
                active = false;
            }
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            if (type == GROUND_OBSTACLE) {
                // Vẽ xương rồng (màu xanh đậm)
                SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
                SDL_Rect rect = { x, y - height, width, height };
                SDL_RenderFillRect(renderer, &rect);

                // Viền đen
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);
            } else if (type == FLYING_OBSTACLE) {
                // Vẽ chim (màu đỏ cam)
                SDL_SetRenderDrawColor(renderer, 255, 100, 50, 255);
                SDL_Rect rect = { x, y - height, width, height };
                SDL_RenderFillRect(renderer, &rect);
            } else { // METEOR
                // Vẽ đuôi lửa phía sau thiên thạch
                if (trailTimer % 2 == 0) {
                    for (int i = 1; i <= 3; i++) {
                        int trailAlpha = 255 - (i * 60);
                        SDL_SetRenderDrawColor(renderer, 255, 150 - i*30, 0, trailAlpha);
                        SDL_Rect trail = {
                            x + width/2 - 10,
                            y - height/2 - (i * 15),
                            20 - i*3,
                            15
                        };
                        SDL_RenderFillRect(renderer, &trail);
                    }
                }

                // Vẽ thiên thạch (màu xám đá với viền đỏ cam)
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
                SDL_Rect meteor = { x, y - height, width, height };
                SDL_RenderFillRect(renderer, &meteor);

                // Viền lửa đỏ cam
                SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);
                SDL_Rect border1 = { x - 2, y - height - 2, width + 4, height + 4 };
                SDL_RenderDrawRect(renderer, &border1);

                // Các vết nứt trên thiên thạch
                SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
                SDL_RenderDrawLine(renderer, x + 5, y - height, x + width - 5, y);
                SDL_RenderDrawLine(renderer, x, y - height + 10, x + width, y - height + 10);

                // Hiệu ứng ánh sáng lấp lánh
                if (trailTimer % 4 < 2) {
                    SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
                    SDL_Rect spark = { x + width/2 - 3, y - height/2 - 3, 6, 6 };
                    SDL_RenderFillRect(renderer, &spark);
                }
            }
        }
    }

    bool checkCollision(int px, int py, int pwidth, int pheight) {
        if (!active) return false;
        SDL_Rect a{ px,        py - pheight, pwidth,  pheight }; // player
        SDL_Rect b{ x,         y  - height,  width,   height  }; // obstacle

        return SDL_HasIntersection(&a, &b) == SDL_TRUE;
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
        meteorInterval = 180; // Thiên thạch spawn ít hơn
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

        // Spawn chướng ngại vật thường
        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawnObstacle();
            spawnTimer = 0;
            spawnInterval = 60 + (rand() % 60);
        }

        // Spawn thiên thạch
        meteorTimer++;
        if (meteorTimer >= meteorInterval) {
            spawnMeteor();
            meteorTimer = 0;
            meteorInterval = 150 + (rand() % 100); // Random 150-250 frames
        }
    }

    void spawnObstacle() {
        // 40% cơ hội spawn chướng ngại vật bay
        ObstacleType type = (rand() % 100 < 40) ? FLYING_OBSTACLE : GROUND_OBSTACLE;
        obstacles.push_back(Obstacle(screenWidth, groundY, speed, type));
    }

    void spawnMeteor() {
        // Spawn thiên thạch ở vị trí ngẫu nhiên phía trên
        int meteorX = 100 + (rand() % (screenWidth - 200));
        obstacles.push_back(Obstacle(meteorX, groundY, speed, METEOR));
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

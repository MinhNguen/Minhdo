#ifndef OBSTACLE_H_INCLUDED
#define OBSTACLE_H_INCLUDED

#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

class Obstacle {
public:
    int x, y;
    int width, height;
    int speed;
    bool active;

    Obstacle(int startX, int groundY, int obstacleSpeed) {
        x = startX;
        width = 20 + (rand() % 30);  // Chiều rộng ngẫu nhiên 20-50
        height = 30 + (rand() % 40); // Chiều cao ngẫu nhiên 30-70
        y = groundY - height;
        speed = obstacleSpeed;
        active = true;
    }

    void update() {
        x -= speed;  // Di chuyển sang trái

        // Vô hiệu hóa khi ra khỏi màn hình
        if (x + width < 0) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Màu xám đen
            SDL_Rect rect = { x, y, width, height };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    // Kiểm tra va chạm với player
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
        spawnInterval = 90;  // Spawn mỗi 90 frames (~1.5 giây)
        srand(time(NULL));
    }

    void update() {
        // Cập nhật tất cả chướng ngại vật
        for (auto& obs : obstacles) {
            obs.update();
        }

        // Xóa chướng ngại vật không active
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(),
                [](const Obstacle& o) { return !o.active; }),
            obstacles.end()
        );

        // Spawn chướng ngại vật mới
        spawnTimer++;
        if (spawnTimer >= spawnInterval) {
            spawnObstacle();
            spawnTimer = 0;
            // Tạo khoảng cách ngẫu nhiên
            spawnInterval = 60 + (rand() % 60);  // 60-120 frames
        }
    }

    void spawnObstacle() {
        obstacles.push_back(Obstacle(screenWidth, groundY, speed));
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

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
    CACTUS_SMALL,      // Xương rồng nhỏ
    CACTUS_MEDIUM,     // Xương rồng trung bình
    CACTUS_LARGE,      // Xương rồng lớn
    CACTUS_GROUP,      // Nhóm xương rồng
    BIRD,              // Chim bay
    METEOR,            // Thiên thạch
    ROCK               // Đá
};

class Obstacle {
public:
    int x, y;
    int width, height;
    int speed;
    bool active;
    ObstacleType type;

    // Cho thiên thạch và hiệu ứng
    float vy;
    float rotation;
    float rotationSpeed;
    int trailTimer;
    int variant; // Biến thể của cùng loại vật cản

    // Constructor
    Obstacle(int startX, int groundY, int obstacleSpeed, ObstacleType obsType = CACTUS_SMALL);

    // Public methods
    void update();
    void render(SDL_Renderer* renderer);
    bool checkCollision(int px, int py, int pwidth, int pheight);

private:
    // Private helper methods
    void setupObstacle(int startX, int groundY);
    void renderCactus(SDL_Renderer* renderer);
    void renderCactusGroup(SDL_Renderer* renderer);
    void renderBird(SDL_Renderer* renderer);
    void renderMeteor(SDL_Renderer* renderer);
    void renderRock(SDL_Renderer* renderer);
    void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius);
    void fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius);
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

    // Constructor
    ObstacleManager(int ground, int gameSpeed, int width);

    // Public methods
    void update();
    void render(SDL_Renderer* renderer);
    bool checkCollisionWithPlayer(int px, int py, int pwidth, int pheight);
    void clear();
    void setSpeed(int newSpeed);

private:
    // Private helper methods
    void spawnObstacle();
    void spawnMeteor();
};

#endif // OBSTACLE_H_INCLUDED

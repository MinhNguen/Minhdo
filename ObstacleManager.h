#ifndef OBSTACLEMANAGER_H_INCLUDED
#define OBSTACLEMANAGER_H_INCLUDED
#include "obstacle.h"
#include <vector>

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



#endif // OBSTACLEMANAGER_H_INCLUDED

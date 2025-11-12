#include "ObstacleManager.h"
ObstacleManager::ObstacleManager(int ground, int gameSpeed, int width) {
    groundY = ground;
    speed = gameSpeed;
    screenWidth = width;
    spawnTimer = 0;
    spawnInterval = 90;
    meteorTimer = 0;
    meteorInterval = 300; // Thiên thạch ít xuất hiện hơn
    srand(time(NULL));
}

void ObstacleManager::update() {
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
        spawnInterval = 70 + (rand() % 50); // 70-120 frames
    }

    meteorTimer++;
    if (meteorTimer >= meteorInterval) {
        if (rand() % 100 < 30) { // 30% cơ hội spawn thiên thạch
            spawnMeteor();
        }
        meteorTimer = 0;
        meteorInterval = 400 + (rand() % 200); // 400-600 frames
    }
}

void ObstacleManager::spawnObstacle() {
    int randVal = rand() % 100;
    ObstacleType type;

    if (randVal < 40) {
        type = CACTUS_SMALL; // 40% xương rồng nhỏ
    } else if (randVal < 65) {
        type = CACTUS_MEDIUM; // 25% xương rồng trung
    } else if (randVal < 80) {
        type = CACTUS_LARGE; // 15% xương rồng lớn
    } else if (randVal < 90) {
        type = CACTUS_GROUP; // 10% nhóm xương rồng
    } else if (randVal < 95) {
        type = BIRD; // 5% chim
    } else {
        type = ROCK; // 5% đá
    }

    obstacles.emplace_back(screenWidth, groundY, speed, type);
}

void ObstacleManager::spawnMeteor() {
    int meteorX = 100 + (rand() % (screenWidth - 200));
    obstacles.emplace_back(meteorX, groundY, speed, METEOR);
}

void ObstacleManager::render(SDL_Renderer* renderer) {
    for (auto& obs : obstacles) {
        obs.render(renderer);
    }
}

bool ObstacleManager::checkCollisionWithPlayer(int px, int py, int pwidth, int pheight) {
    for (auto& obs : obstacles) {
        if (obs.checkCollision(px, py, pwidth, pheight)) {
            return true;
        }
    }
    return false;
}

void ObstacleManager::clear() {
    obstacles.clear();
    spawnTimer = 0;
    meteorTimer = 0;
}

void ObstacleManager::setSpeed(int newSpeed) {
    speed = newSpeed;
    for (auto& obs : obstacles) {
        if (obs.type != METEOR) {
            obs.speed = newSpeed;
        }
    }
}

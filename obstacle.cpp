#include "obstacle.h"
#include <iostream>

// ===================== OBSTACLE CLASS IMPLEMENTATION =====================

Obstacle::Obstacle(int startX, int groundY, int obstacleSpeed, ObstacleType obsType) {
    speed = obstacleSpeed;
    active = true;
    type = obsType;
    rotation = 0;
    rotationSpeed = 5.0f + (rand() % 10);
    trailTimer = 0;
    variant = rand() % 3; // 3 biến thể cho mỗi loại

    setupObstacle(startX, groundY);
}

void Obstacle::setupObstacle(int startX, int groundY) {
    x = startX;

    switch (type) {
        case CACTUS_SMALL: {
            width = 15 + (rand() % 10);
            height = 40 + (rand() % 20);
            y = groundY;
            vy = 0;
            break;
        }
        case CACTUS_MEDIUM: {
            width = 20 + (rand() % 15);
            height = 60 + (rand() % 25);
            y = groundY;
            vy = 0;
            break;
        }
        case CACTUS_LARGE: {
            width = 25 + (rand() % 20);
            height = 80 + (rand() % 30);
            y = groundY;
            vy = 0;
            break;
        }
        case CACTUS_GROUP: {
            width = 50 + (rand() % 30);
            height = 60 + (rand() % 40);
            y = groundY;
            vy = 0;
            break;
        }
        case BIRD: {
            width = 35 + (rand() % 20);
            height = 25 + (rand() % 15);
            vy = 0;
            // Chim bay ở các độ cao khác nhau
            int heightLevel = rand() % 4;
            if (heightLevel == 0) y = groundY - 60;
            else if (heightLevel == 1) y = groundY - 100;
            else if (heightLevel == 2) y = groundY - 140;
            else y = groundY - 180;
            break;
        }
        case METEOR: {
            width = 30 + (rand() % 25);
            height = 30 + (rand() % 25);
            y = -50 - (rand() % 100);
            vy = 3.0f + (rand() % 4);
            break;
        }
        case ROCK: {
            width = 25 + (rand() % 20);
            height = 20 + (rand() % 15);
            y = groundY;
            vy = 0;
            break;
        }
    }
}

void Obstacle::update() {
    if (type == METEOR) {
        y += vy;
        x -= speed / 2;
        rotation += rotationSpeed;
        trailTimer++;
        if (vy < 12) vy += 0.1f;
        if (y > 500 || x + width < 0) active = false;
    } else if (type == BIRD) {
        // Chim bay lượn
        x -= speed;
        y += sin(trailTimer * 0.1f) * 2;
        trailTimer++;
        if (x + width < 0) active = false;
    } else {
        x -= speed;
        if (x + width < 0) active = false;
    }
}

void Obstacle::render(SDL_Renderer* renderer) {
    if (!active) return;

    switch (type) {
        case CACTUS_SMALL:
        case CACTUS_MEDIUM:
        case CACTUS_LARGE:
            renderCactus(renderer);
            break;
        case CACTUS_GROUP:
            renderCactusGroup(renderer);
            break;
        case BIRD:
            renderBird(renderer);
            break;
        case METEOR:
            renderMeteor(renderer);
            break;
        case ROCK:
            renderRock(renderer);
            break;
    }
}

void Obstacle::renderCactus(SDL_Renderer* renderer) {
    // Màu xương rồng - gradient từ xanh đậm đến xanh nhạt
    SDL_Color cactusDark = {50, 120, 50, 255};
    SDL_Color cactusMedium = {70, 150, 70, 255};
    SDL_Color cactusLight = {90, 180, 90, 255};
    SDL_Color spineColor = {30, 80, 30, 255};

    SDL_Rect mainBody = { x, y - height, width, height };

    // Vẽ thân chính với gradient
    for (int i = 0; i < height; i++) {
        float ratio = (float)i / height;
        SDL_Color currentColor;
        currentColor.r = cactusDark.r + (cactusLight.r - cactusDark.r) * ratio;
        currentColor.g = cactusDark.g + (cactusLight.g - cactusDark.g) * ratio;
        currentColor.b = cactusDark.b + (cactusLight.b - cactusDark.b) * ratio;
        currentColor.a = 255;

        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
        SDL_RenderDrawLine(renderer, x, y - height + i, x + width, y - height + i);
    }

    // Vẽ các nhánh xương rồng tùy theo loại và biến thể
    std::vector<SDL_Rect> branches;

    if (type == CACTUS_SMALL) {
        // Xương rồng nhỏ: 1-2 nhánh ngắn
        if (variant % 2 == 0) {
            // Nhánh trái
            branches.push_back({x - width/2, y - height/2, width/2, height/3});
        }
        if (variant % 3 == 0) {
            // Nhánh phải
            branches.push_back({x + width, y - height*2/3, width/2, height/4});
        }
    }
    else if (type == CACTUS_MEDIUM) {
        // Xương rồng trung: 2-3 nhánh
        branches.push_back({x - width/2 + 2, y - height/2, width/2, height/3}); // Trái giữa
        branches.push_back({x + width - 2, y - height*3/4, width/2, height/4}); // Phải trên

        if (variant % 2 == 0) {
            branches.push_back({x - width/3, y - height/4, width/3, height/4}); // Trái dưới
        }
    }
    else if (type == CACTUS_LARGE) {
        // Xương rồng lớn: 3-4 nhánh phức tạp
        branches.push_back({x - width*2/3, y - height/2, width*2/3, height/3}); // Nhánh trái lớn
        branches.push_back({x + width, y - height*3/4, width/2, height/3});     // Nhánh phải trên
        branches.push_back({x + width*2/3, y - height/4, width/3, height/4});   // Nhánh phải dưới

        if (variant % 3 == 0) {
            branches.push_back({x - width/2, y - height*3/4, width/2, height/4}); // Nhánh trái trên
        }
    }

    // Vẽ các nhánh
    for (const auto& branch : branches) {
        // Gradient cho nhánh
        for (int i = 0; i < branch.h; i++) {
            float ratio = (float)i / branch.h;
            SDL_Color currentColor;
            currentColor.r = cactusDark.r + (cactusMedium.r - cactusDark.r) * ratio;
            currentColor.g = cactusDark.g + (cactusMedium.g - cactusDark.g) * ratio;
            currentColor.b = cactusDark.b + (cactusMedium.b - cactusDark.b) * ratio;
            currentColor.a = 255;

            SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
            SDL_RenderDrawLine(renderer, branch.x, branch.y + i, branch.x + branch.w, branch.y + i);
        }

        // Viền nhánh
        SDL_SetRenderDrawColor(renderer, 40, 100, 40, 255);
        SDL_RenderDrawRect(renderer, &branch);
    }

    // Vẽ gai/xương cho thân chính và các nhánh
    SDL_SetRenderDrawColor(renderer, spineColor.r, spineColor.g, spineColor.b, 255);

    // Gai trên thân chính - tập trung ở các khớp
    for (int i = 0; i < height; i += 6) {
        // Gai ngang
        SDL_RenderDrawLine(renderer, x - 3, y - height + i, x + width + 3, y - height + i);

        // Gai dọc ở hai bên (mỗi 3 dòng)
        if (i % 12 == 0) {
            // Trái
            for (int j = -2; j <= 2; j++) {
                SDL_RenderDrawPoint(renderer, x - 2, y - height + i + j);
            }
            // Phải
            for (int j = -2; j <= 2; j++) {
                SDL_RenderDrawPoint(renderer, x + width + 1, y - height + i + j);
            }
        }
    }

    // Vẽ gai cho các nhánh
    for (const auto& branch : branches) {
        for (int i = 0; i < branch.h; i += 5) {
            // Gai ngang trên nhánh
            SDL_RenderDrawLine(renderer, branch.x - 2, branch.y + i,
                             branch.x + branch.w + 2, branch.y + i);
        }
    }

    // Vẽ các cụm gai đặc biệt ở ngọn và khớp nhánh
    SDL_SetRenderDrawColor(renderer, 20, 60, 20, 255);

    // Cụm gai ở ngọn thân chính
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            if (abs(i) + abs(j) <= 3) { // Hình thoi
                SDL_RenderDrawPoint(renderer, x + width/2 + i, y - height + j);
            }
        }
    }

    // Cụm gai ở các khớp nhánh
    for (const auto& branch : branches) {
        // Ở gốc nhánh (chỗ nối với thân)
        int jointX = (branch.x < x) ? x : x + width; // Xác định bên trái hay phải
        int jointY = branch.y + branch.h/2;

        for (int i = -3; i <= 3; i++) {
            for (int j = -3; j <= 3; j++) {
                if (abs(i) + abs(j) <= 4) {
                    SDL_RenderDrawPoint(renderer, jointX + i, jointY + j);
                }
            }
        }
    }

    // Viền tổng thể cho thân chính
    SDL_SetRenderDrawColor(renderer, 40, 100, 40, 255);
    SDL_RenderDrawRect(renderer, &mainBody);

    // Làm nổi bật các cạnh
    SDL_SetRenderDrawColor(renderer, 100, 200, 100, 100);
    SDL_RenderDrawLine(renderer, x + 1, y - height + 1, x + width - 1, y - height + 1); // Cạnh trên
    SDL_RenderDrawLine(renderer, x + 1, y - height + 1, x + 1, y - 1); // Cạnh trái
}

void Obstacle::renderCactusGroup(SDL_Renderer* renderer) {
    SDL_Color cactusColor = {60, 140, 60, 255};

    // Vẽ 2-3 cây xương rồng gần nhau
    int numCacti = 2 + (variant % 2); // 2 hoặc 3 cây

    for (int i = 0; i < numCacti; i++) {
        int offsetX = i * (width / numCacti);
        int cactusWidth = width / numCacti - 5;
        int cactusHeight = height - 10 + (rand() % 20);

        SDL_Rect cactus = { x + offsetX, y - cactusHeight, cactusWidth, cactusHeight };

        SDL_SetRenderDrawColor(renderer, cactusColor.r, cactusColor.g, cactusColor.b, 255);
        SDL_RenderFillRect(renderer, &cactus);

        SDL_SetRenderDrawColor(renderer, 30, 80, 30, 255);
        SDL_RenderDrawRect(renderer, &cactus);

        // Vẽ gai
        SDL_SetRenderDrawColor(renderer, 40, 100, 40, 255);
        for (int j = 1; j < cactusHeight; j += 6) {
            SDL_RenderDrawLine(renderer, x + offsetX, y - cactusHeight + j,
                             x + offsetX + cactusWidth, y - cactusHeight + j);
        }
    }
}

void Obstacle::renderBird(SDL_Renderer* renderer) {
    SDL_Color birdColor;

    // Màu sắc khác nhau cho chim
    switch (variant) {
        case 0: birdColor = {255, 100, 100, 255}; break; // Đỏ
        case 1: birdColor = {100, 100, 255, 255}; break; // Xanh
        case 2: birdColor = {255, 200, 100, 255}; break; // Vàng
    }

    // Thân chim
    SDL_Rect body = { x, y - height/2, width, height/2 };
    SDL_SetRenderDrawColor(renderer, birdColor.r, birdColor.g, birdColor.b, 255);
    SDL_RenderFillRect(renderer, &body);

    // Đầu chim
    SDL_Rect head = { x + width - 10, y - height, 15, 15 };
    SDL_RenderFillRect(renderer, &head);

    // Cánh chim - di chuyển lên xuống
    int wingOffset = (int)(sin(trailTimer * 0.2f) * 5);
    SDL_Rect wing = { x + 5, y - height/2 - 10 + wingOffset, width - 10, 8 };
    SDL_SetRenderDrawColor(renderer, birdColor.r * 0.7f, birdColor.g * 0.7f, birdColor.b * 0.7f, 255);
    SDL_RenderFillRect(renderer, &wing);

    // Mắt
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawPoint(renderer, x + width - 3, y - height + 5);
}

void Obstacle::renderMeteor(SDL_Renderer* renderer) {
    // Hiệu ứng đuôi lửa
    if (trailTimer % 2 == 0) {
        for (int i = 1; i <= 4; i++) {
            int alpha = 255 - (i * 50);
            int size = 20 - (i * 3);

            SDL_SetRenderDrawColor(renderer, 255, 150 - i * 20, 0, alpha);
            SDL_Rect trail = {
                x + width / 2 - size/2,
                y - height / 2 - (i * 12),
                size,
                12
            };
            SDL_RenderFillRect(renderer, &trail);
        }
    }

    // Thân thiên thạch
    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_Rect meteor = { x, y - height, width, height };
    SDL_RenderFillRect(renderer, &meteor);

    // Bề mặt gồ ghề
    SDL_SetRenderDrawColor(renderer, 120, 120, 140, 255);
    for (int i = 0; i < 5; i++) {
        int craterX = x + 5 + (rand() % (width - 10));
        int craterY = y - height + 5 + (rand() % (height - 10));
        int craterSize = 3 + (rand() % 4);
        SDL_Rect crater = { craterX, craterY, craterSize, craterSize };
        SDL_RenderFillRect(renderer, &crater);
    }

    // Viền nóng
    SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);
    SDL_RenderDrawRect(renderer, &meteor);
}

void Obstacle::renderRock(SDL_Renderer* renderer) {
    SDL_Color rockColor = {120, 120, 120, 255};
    SDL_Color highlightColor = {150, 150, 150, 255};
    SDL_Color shadowColor = {80, 80, 80, 255};

    SDL_Rect rock = { x, y - height, width, height };

    // Thân đá
    SDL_SetRenderDrawColor(renderer, rockColor.r, rockColor.g, rockColor.b, 255);
    SDL_RenderFillRect(renderer, &rock);

    // Chi tiết bề mặt đá
    SDL_SetRenderDrawColor(renderer, shadowColor.r, shadowColor.g, shadowColor.b, 255);
    for (int i = 0; i < 8; i++) {
        int crackX = x + (rand() % width);
        int crackY = y - height + (rand() % height);
        SDL_RenderDrawLine(renderer, crackX, crackY, crackX + 3, crackY + 3);
    }

    // Điểm sáng
    SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, 255);
    for (int i = 0; i < 3; i++) {
        int spotX = x + (rand() % width);
        int spotY = y - height + (rand() % height);
        SDL_RenderDrawPoint(renderer, spotX, spotY);
    }

    // Viền
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawRect(renderer, &rock);
}

bool Obstacle::checkCollision(int px, int py, int pwidth, int pheight) {
    if (!active) return false;
    SDL_Rect a{ px, py - pheight, pwidth, pheight };
    SDL_Rect b{ x, y - height, width, height };
    return SDL_HasIntersection(&a, &b);
}

void Obstacle::drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void Obstacle::fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

// ===================== OBSTACLE MANAGER IMPLEMENTATION =====================

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

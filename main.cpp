#include <SDL2/SDL.h>
#include <iostream>
#include "player.h"
#include "obstacle.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "❌ SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    SDL_Window* window = SDL_CreateWindow("Dino Game - SDL2",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window) {
        std::cout << "❌ Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "❌ Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Player player;
    ObstacleManager obstacleManager(player.groundY, 5, SCREEN_WIDTH);

    bool running = true;
    bool gameOver = false;
    SDL_Event event;

    // Lưu trạng thái phím bấm liên tục
    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    std::cout << "🎮 Game Started!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - SPACE/UP: Jump" << std::endl;
    std::cout << "  - LEFT/RIGHT: Move" << std::endl;
    std::cout << "  - ESC: Quit" << std::endl;
    std::cout << "  - R: Restart (when game over)" << std::endl;

    while (running) {
        // Xử lý sự kiện
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                case SDLK_UP:
                    if (!gameOver) {
                        // Chỉ nhảy khi đang đứng trên mặt đất
                        if (player.isOnGround) {
                            player.vy = -12.0f;  // Lực nhảy
                            player.isOnGround = false;
                        }
                    }
                    break;
                case SDLK_r:
                    // Restart game
                    if (gameOver) {
                        gameOver = false;
                        player.x = 50;
                        player.y = player.groundY;
                        player.vy = 0;
                        player.isOnGround = true;
                        obstacleManager.clear();
                        std::cout << "🔄 Game Restarted!" << std::endl;
                    }
                    break;
                case SDLK_ESCAPE:
                    running = false;
                    break;
                }
            }
        }

        if (!gameOver) {
            // Di chuyển trái phải (kiểm tra phím giữ liên tục)
            keyState = SDL_GetKeyboardState(NULL);
            if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A]) {
                player.x -= player.vx;
            }
            if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D]) {
                player.x += player.vx;
            }

            // Áp dụng trọng lực
            if (!player.isOnGround) {
                player.vy += player.gravity;  // Tăng vận tốc rơi
            }

            player.y += player.vy;  // Cập nhật vị trí Y

            // Kiểm tra chạm đất
            if (player.y >= player.groundY) {
                player.y = player.groundY;
                player.vy = 0;
                player.isOnGround = true;
            } else {
                player.isOnGround = false;
            }

            // Giới hạn không đi ra ngoài cửa sổ (trái phải)
            if (player.x < 0) player.x = 0;
            if (player.x + player.width > SCREEN_WIDTH) player.x = SCREEN_WIDTH - player.width;

            // Giới hạn không bay quá cao
            if (player.y < 0) player.y = 0;

            // Cập nhật chướng ngại vật
            obstacleManager.update();

            // Kiểm tra va chạm
            if (obstacleManager.checkCollisionWithPlayer(
                player.x, player.y, player.width, player.height)) {
                gameOver = true;
                std::cout << "💀 GAME OVER! Press R to restart" << std::endl;
            }
        }

        // Vẽ lại khung hình
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // Nền màu trời
        SDL_RenderClear(renderer);

        // Vẽ mặt sàn
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Màu xanh lá
        SDL_Rect ground = { 0, player.groundY + (int)player.height, SCREEN_WIDTH,
                           SCREEN_HEIGHT - (player.groundY + (int)player.height) };
        SDL_RenderFillRect(renderer, &ground);

        // Vẽ chướng ngại vật
        obstacleManager.render(renderer);

        // Vẽ nhân vật
        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Màu xám khi chết
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Màu đỏ
        }
        SDL_Rect playerRect = { player.x, player.y, (int)player.width, (int)player.height };
        SDL_RenderFillRect(renderer, &playerRect);

        // Vẽ text GAME OVER (đơn giản bằng hình chữ nhật)
        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect gameOverBg = { SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 30, 200, 60 };
            SDL_RenderFillRect(renderer, &gameOverBg);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect gameOverBorder = { SCREEN_WIDTH/2 - 98, SCREEN_HEIGHT/2 - 28, 196, 56 };
            SDL_RenderDrawRect(renderer, &gameOverBorder);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

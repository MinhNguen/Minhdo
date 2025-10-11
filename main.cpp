#include <SDL2/SDL.h>
#include <iostream>
#include "player.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "❌ SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Platformer Game - SDL2",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_SHOWN);

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
    bool running = true;
    SDL_Event event;

    // Lưu trạng thái phím bấm liên tục
    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    while (running) {
        // Xử lý sự kiện
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                case SDLK_UP:
                    // Chỉ nhảy khi đang đứng trên mặt đất
                    if (player.isOnGround) {
                        player.vy = -12.0f;  // Lực nhảy
                        player.isOnGround = false;
                    }
                    break;
                case SDLK_ESCAPE:
                    running = false;
                    break;
                }
            }
        }

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
        if (player.x + player.width > 640) player.x = 640 - player.width;

        // Giới hạn không bay quá cao
        if (player.y < 0) player.y = 0;

        // Vẽ lại khung hình
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // Nền màu trời
        SDL_RenderClear(renderer);

        // Vẽ mặt sàn
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Màu xanh lá
        SDL_Rect ground = { 0, player.groundY + player.height, 640, 480 - (player.groundY + player.height) };
        SDL_RenderFillRect(renderer, &ground);

        // Vẽ nhân vật
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Màu đỏ
        SDL_Rect playerRect = { player.x, player.y, (int)player.width, (int)player.height };
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

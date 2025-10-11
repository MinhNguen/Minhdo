#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "❌ SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Di chuyen khoi vuong - SDL2",
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

    bool running = true;
    SDL_Event event;

    // Tọa độ và kích thước khối vuông
    SDL_Rect square = { 300, 220, 50, 50 };
    int speed = 5;

    while (running) {
        // Xử lý sự kiện bàn phím
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    square.y -= speed;
                    break;
                case SDLK_DOWN:
                    square.y += speed;
                    break;
                case SDLK_LEFT:
                    square.x -= speed;
                    break;
                case SDLK_RIGHT:
                    square.x += speed;
                    break;
                case SDLK_ESCAPE:
                    running = false;
                    break;
                }
            }
        }

        // Giới hạn để khối vuông không đi ra ngoài cửa sổ
        if (square.x < 0) square.x = 0;
        if (square.y < 0) square.y = 0;
        if (square.x + square.w > 640) square.x = 640 - square.w;
        if (square.y + square.h > 480) square.y = 480 - square.h;

        // Vẽ lại khung hình
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // nền trắng
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // khối vuông màu xanh
        SDL_RenderFillRect(renderer, &square);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

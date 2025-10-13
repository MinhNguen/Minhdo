#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include "player.h"
#include "obstacle.h"

enum class GameState { MENU, PLAYING, GAME_OVER };

// ===================== Helper to Render Text =====================
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        std::cout << "TTF_RenderText Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                        SDL_Color color, int y, int screenW) {
    SDL_Texture* tex = renderText(renderer, font, text, color);
    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { (screenW - w) / 2, y, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
}

// ===================== Main =====================
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cout << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 480;

    SDL_Window* window = SDL_CreateWindow("Dino Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load font
    TTF_Font* fontBig = TTF_OpenFont("NotoSans-Regular.ttf", 48);
    TTF_Font* fontSmall = TTF_OpenFont("NotoSans-Regular.ttf", 24);
    if (!fontBig || !fontSmall) {
        std::cout << "Failed to load font! Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Entities
    Player player;
    ObstacleManager obstacles(player.groundY, 6, SCREEN_WIDTH);

    GameState state = GameState::MENU;
    bool running = true;
    bool gameOver = false;
    SDL_Event e;

    SDL_Rect playBtn = { SCREEN_WIDTH / 2 - 150, 220, 300, 80 };
    SDL_Rect quitBtn = { SCREEN_WIDTH / 2 - 150, 320, 300, 80 };

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
            else if (state == GameState::MENU && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;
                if (mx >= playBtn.x && mx <= playBtn.x + playBtn.w &&
                    my >= playBtn.y && my <= playBtn.y + playBtn.h) {
                    player.x = 50;
                    player.y = player.groundY;
                    player.vy = 0;
                    player.isOnGround = true;
                    obstacles.clear();
                    state = GameState::PLAYING;
                } else if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                           my >= quitBtn.y && my <= quitBtn.y + quitBtn.h) {
                    running = false;
                }
            } else if (state == GameState::PLAYING && e.type == SDL_KEYDOWN) {
                if ((e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_UP) && player.isOnGround) {
                    player.vy = -12.0f;
                    player.isOnGround = false;
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::MENU;
                }
            } else if (state == GameState::GAME_OVER && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_r) {
                    obstacles.clear();
                    player.x = 50;
                    player.y = player.groundY;
                    player.vy = 0;
                    player.isOnGround = true;
                    gameOver = false;
                    state = GameState::PLAYING;
                } else if (e.key.keysym.sym == SDLK_m) {
                    state = GameState::MENU;
                }
            }
        }

        // Update
        if (state == GameState::PLAYING && !gameOver) {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_LEFT]) player.x -= player.vx;
            if (keys[SDL_SCANCODE_RIGHT]) player.x += player.vx;
            if (!player.isOnGround) player.vy += player.gravity;
            player.y += player.vy;
            if (player.y >= player.groundY) {
                player.y = player.groundY;
                player.vy = 0;
                player.isOnGround = true;
            }

            obstacles.update();
            if (obstacles.checkCollisionWithPlayer(player.x, player.y, player.width, player.height)) {
                gameOver = true;
                state = GameState::GAME_OVER;
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
        SDL_RenderClear(renderer);

        if (state == GameState::MENU) {
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Color black = { 0, 0, 0, 255 };

            renderCenteredText(renderer, fontBig, "DINO GAME", black, 100, SCREEN_WIDTH);

            // Play button
            int mx, my; SDL_GetMouseState(&mx, &my);
            bool hovPlay = (mx >= playBtn.x && mx <= playBtn.x + playBtn.w &&
                            my >= playBtn.y && my <= playBtn.y + playBtn.h);
            bool hovQuit = (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                            my >= quitBtn.y && my <= quitBtn.y + quitBtn.h);

            SDL_SetRenderDrawColor(renderer, hovPlay ? 80 : 50, hovPlay ? 160 : 100, 50, 255);
            SDL_RenderFillRect(renderer, &playBtn);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &playBtn);
            renderCenteredText(renderer, fontSmall, "PLAY", white, playBtn.y + 20, SCREEN_WIDTH);

            SDL_SetRenderDrawColor(renderer, hovQuit ? 160 : 100, 50, 50, 255);
            SDL_RenderFillRect(renderer, &quitBtn);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &quitBtn);
            renderCenteredText(renderer, fontSmall, "QUIT", white, quitBtn.y + 20, SCREEN_WIDTH);

            renderCenteredText(renderer, fontSmall,
                "CONTROLS: SPACE/UP=JUMP, ESC=MENU", black, SCREEN_HEIGHT - 60, SCREEN_WIDTH);
        }
        else if (state == GameState::PLAYING || state == GameState::GAME_OVER) {
            // Ground
            SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
            SDL_Rect ground = { 0, player.groundY, SCREEN_WIDTH, SCREEN_HEIGHT - player.groundY };
            SDL_RenderFillRect(renderer, &ground);

            // Obstacles & Player
            obstacles.render(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect rect = { player.x, player.y - player.height, player.width, player.height };
            SDL_RenderFillRect(renderer, &rect);

            if (state == GameState::GAME_OVER) {
                SDL_Color white = { 255, 255, 255, 255 };
                renderCenteredText(renderer, fontBig, "GAME OVER", white, 180, SCREEN_WIDTH);
                renderCenteredText(renderer, fontSmall, "PRESS R TO RESTART OR M FOR MENU",
                                   white, 260, SCREEN_WIDTH);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Cleanup
    TTF_CloseFont(fontBig);
    TTF_CloseFont(fontSmall);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

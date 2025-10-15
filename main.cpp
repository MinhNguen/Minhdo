#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include "player.h"
#include "obstacle.h"
#include "score.h"

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

void renderLeftText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                    SDL_Color color, int x, int y) {
    SDL_Texture* tex = renderText(renderer, font, text, color);
    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { x, y, w, h };
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

    SDL_Window* window = SDL_CreateWindow("Dino Game - With Score System",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load font
    TTF_Font* fontBig = TTF_OpenFont("NotoSans-Regular.ttf", 48);
    TTF_Font* fontMedium = TTF_OpenFont("NotoSans-Regular.ttf", 32);
    TTF_Font* fontSmall = TTF_OpenFont("NotoSans-Regular.ttf", 24);
    TTF_Font* fontTiny = TTF_OpenFont("NotoSans-Regular.ttf", 18);
    if (!fontBig || !fontSmall || !fontMedium || !fontTiny) {
        std::cout << "Failed to load font! Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_Texture* dino = IMG_LoadTexture(renderer, "image/dino.png");

    // Entities
    Player player;
    ObstacleManager obstacles(player.groundY, 6, SCREEN_WIDTH);
    ScoreManager scoreManager(player.groundY, 6, SCREEN_WIDTH);

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
                    scoreManager.reset();
                    gameOver = false;
                    state = GameState::PLAYING;
                } else if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                           my >= quitBtn.y && my <= quitBtn.y + quitBtn.h) {
                    running = false;
                }
            } else if (state == GameState::PLAYING && e.type == SDL_KEYDOWN) {
                if ((e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_UP) && player.isOnGround) {
                    player.vy = -12.0f;
                    player.isOnGround = false;
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    player.x += player.vx;
                    obstacles.update();
                    scoreManager.update(player.x, player.y, player.width, player.height);
                } else if (e.key.keysym.sym == SDLK_LEFT) {
                    player.x -= player.vx;
                    obstacles.update();
                    scoreManager.update(player.x, player.y, player.width, player.height);
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::MENU;
                }
            } else if (state == GameState::GAME_OVER && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_r) {
                    obstacles.clear();
                    scoreManager.reset();
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
            if (!player.isOnGround) player.vy += player.gravity;
            player.y += player.vy;
            if (player.y >= player.groundY) {
                player.y = player.groundY;
                player.vy = 0;
                player.isOnGround = true;
            }

            obstacles.update();
            scoreManager.update(player.x, player.y, player.width, player.height);

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
            SDL_Color gold = { 255, 215, 0, 255 };

            renderCenteredText(renderer, fontBig, "DINO GAME", black, 80, SCREEN_WIDTH);

            // Hiển thị High Score
            std::string highScoreText = "HIGH SCORE: " + std::to_string(scoreManager.getHighScore());
            renderCenteredText(renderer, fontSmall, highScoreText, gold, 150, SCREEN_WIDTH);

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

            renderCenteredText(renderer, fontTiny,
                "Collect coins for bonus points!", black, SCREEN_HEIGHT - 30, SCREEN_WIDTH);
        }
        else if (state == GameState::PLAYING || state == GameState::GAME_OVER) {
            // Ground
            SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
            SDL_Rect ground = { 0, player.groundY, SCREEN_WIDTH, SCREEN_HEIGHT - player.groundY };
            SDL_RenderFillRect(renderer, &ground);

            // Obstacles, Coins & Player
            obstacles.render(renderer);
            scoreManager.render(renderer);
            SDL_Rect rect = { player.x, player.y - player.height, player.width, player.height };
            SDL_RenderCopy(renderer, dino, nullptr,  &rect);

            // Hiển thị điểm số trong game
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Color yellow = { 255, 255, 0, 255 };
            SDL_Color black = { 0, 0, 0, 255 };

            // Tổng điểm (góc trên bên trái)
            std::string scoreText = "SCORE: " + std::to_string(scoreManager.getCurrentScore());
            renderLeftText(renderer, fontMedium, scoreText, black, 10, 10);

            // Chi tiết điểm (góc trên bên phải)
            std::string distanceText = "Distance: " + std::to_string(scoreManager.getDistanceScore());
            std::string coinText = "Coins: " + std::to_string(scoreManager.getCoinScore()) +
                                   " (" + std::to_string(scoreManager.getTotalCoins()) + ")";

            renderLeftText(renderer, fontTiny, distanceText, black, SCREEN_WIDTH - 150, 10);
            renderLeftText(renderer, fontTiny, coinText, yellow, SCREEN_WIDTH - 150, 35);

            if (state == GameState::GAME_OVER) {
                // Màn hình game over với thông tin chi tiết
                renderCenteredText(renderer, fontBig, "GAME OVER", white, 120, SCREEN_WIDTH);

                std::string finalScore = "FINAL SCORE: " + std::to_string(scoreManager.getCurrentScore());
                renderCenteredText(renderer, fontMedium, finalScore, yellow, 190, SCREEN_WIDTH);

                std::string breakdown = "Distance: " + std::to_string(scoreManager.getDistanceScore()) +
                                       " | Coins: " + std::to_string(scoreManager.getCoinScore());
                renderCenteredText(renderer, fontSmall, breakdown, white, 240, SCREEN_WIDTH);

                std::string coinsCollected = "Coins Collected: " + std::to_string(scoreManager.getTotalCoins());
                renderCenteredText(renderer, fontSmall, coinsCollected, yellow, 270, SCREEN_WIDTH);

                if (scoreManager.getCurrentScore() == scoreManager.getHighScore() &&
                    scoreManager.getCurrentScore() > 0) {
                    renderCenteredText(renderer, fontSmall, "NEW HIGH SCORE!",
                                     SDL_Color{255, 215, 0, 255}, 300, SCREEN_WIDTH);
                }

                renderCenteredText(renderer, fontSmall, "PRESS R TO RESTART OR M FOR MENU",
                                   white, 340, SCREEN_WIDTH);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Cleanup
    TTF_CloseFont(fontBig);
    TTF_CloseFont(fontMedium);
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontTiny);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

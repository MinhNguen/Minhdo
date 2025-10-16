#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include <fstream>
#include "player.h"
#include "obstacle.h"
#include "score.h"

enum class GameState { MENU, LEVEL_SELECT, PLAYING, GAME_OVER, LEVEL_COMPLETE };

// Cấu trúc thông tin màn chơi
struct LevelInfo {
    int levelNumber;
    std::string name;
    int obstacleSpeed;
    int spawnInterval;
    int meteorInterval;
    int targetScore;
    bool unlocked;
    int bestScore;
};

// Quản lý màn chơi
class LevelManager {
public:
    std::vector<LevelInfo> levels;
    int currentLevel;

    LevelManager() {
        currentLevel = 0;
        initializeLevels();
        loadProgress();
    }

    void initializeLevels() {
        levels = {
            {1, "Easy Valley", 6, 90, 250, 50, true, 0},
            {2, "Rocky Hills", 7, 80, 220, 100, false, 0},
            {3, "Desert Storm", 8, 70, 200, 150, false, 0},
            {4, "Thunder Plains", 9, 60, 180, 200, false, 0},
            {5, "Volcano Peak", 10, 50, 150, 300, false, 0}
        };
    }

    void saveProgress() {
        std::ofstream file("game_progress.dat");
        if (file.is_open()) {
            for (const auto& level : levels) {
                file << level.unlocked << " " << level.bestScore << "\n";
            }
            file.close();
        }
    }

    void loadProgress() {
        std::ifstream file("game_progress.dat");
        if (file.is_open()) {
            for (auto& level : levels) {
                file >> level.unlocked >> level.bestScore;
            }
            file.close();
        }
    }

    void unlockNextLevel() {
        if (currentLevel < levels.size() - 1) {
            levels[currentLevel + 1].unlocked = true;
            saveProgress();
        }
    }

    void updateBestScore(int score) {
        if (score > levels[currentLevel].bestScore) {
            levels[currentLevel].bestScore = score;
            saveProgress();
        }
    }

    bool isLevelComplete(int score) {
        return score >= levels[currentLevel].targetScore;
    }

    LevelInfo getCurrentLevel() {
        return levels[currentLevel];
    }

    void setCurrentLevel(int index) {
        if (index >= 0 && index < levels.size()) {
            currentLevel = index;
        }
    }
};

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

    SDL_Window* window = SDL_CreateWindow("Dino Game - Level System",
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
    ObstacleManager* obstacles = nullptr;
    ScoreManager* scoreManager = nullptr;
    LevelManager levelManager;

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
                    state = GameState::LEVEL_SELECT;
                } else if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                           my >= quitBtn.y && my <= quitBtn.y + quitBtn.h) {
                    running = false;
                }
            }
            else if (state == GameState::LEVEL_SELECT && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;

                // Kiểm tra click vào các màn chơi
                for (size_t i = 0; i < levelManager.levels.size(); i++) {
                    int btnX = 50 + (i % 3) * 250;
                    int btnY = 120 + (i / 3) * 100;
                    SDL_Rect levelBtn = { btnX, btnY, 200, 80 };

                    if (mx >= levelBtn.x && mx <= levelBtn.x + levelBtn.w &&
                        my >= levelBtn.y && my <= levelBtn.y + levelBtn.h &&
                        levelManager.levels[i].unlocked) {

                        levelManager.setCurrentLevel(i);
                        LevelInfo level = levelManager.getCurrentLevel();

                        // Khởi tạo game với thông số của màn chơi
                        if (obstacles) delete obstacles;
                        if (scoreManager) delete scoreManager;

                        obstacles = new ObstacleManager(player.groundY, level.obstacleSpeed, SCREEN_WIDTH);
                        obstacles->spawnInterval = level.spawnInterval;
                        obstacles->meteorInterval = level.meteorInterval;

                        scoreManager = new ScoreManager(player.groundY, level.obstacleSpeed, SCREEN_WIDTH);

                        player.x = 50;
                        player.y = player.groundY;
                        player.vy = 0;
                        player.isOnGround = true;
                        gameOver = false;
                        state = GameState::PLAYING;
                        break;
                    }
                }

                // Nút back
                SDL_Rect backBtn = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 70, 200, 50 };
                if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                    my >= backBtn.y && my <= backBtn.y + backBtn.h) {
                    state = GameState::MENU;
                }
            }
            else if (state == GameState::PLAYING && e.type == SDL_KEYDOWN) {
                if ((e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_UP) && player.isOnGround) {
                    player.vy = -12.0f;
                    player.isOnGround = false;
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    player.x += player.vx;
                    if (obstacles) obstacles->update();
                    if (scoreManager) scoreManager->update(player.x, player.y, player.width, player.height);
                } else if (e.key.keysym.sym == SDLK_LEFT) {
                    player.x -= player.vx;
                    if (obstacles) obstacles->update();
                    if (scoreManager) scoreManager->update(player.x, player.y, player.width, player.height);
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::LEVEL_SELECT;
                }
            }
            else if (state == GameState::GAME_OVER && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_r) {
                    LevelInfo level = levelManager.getCurrentLevel();
                    if (obstacles) {
                        obstacles->clear();
                        obstacles->spawnInterval = level.spawnInterval;
                        obstacles->meteorInterval = level.meteorInterval;
                    }
                    if (scoreManager) scoreManager->reset();
                    player.x = 50;
                    player.y = player.groundY;
                    player.vy = 0;
                    player.isOnGround = true;
                    gameOver = false;
                    state = GameState::PLAYING;
                } else if (e.key.keysym.sym == SDLK_m) {
                    state = GameState::LEVEL_SELECT;
                }
            }
            else if (state == GameState::LEVEL_COMPLETE && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_n) {
                    // Chơi màn tiếp theo
                    if (levelManager.currentLevel < levelManager.levels.size() - 1) {
                        levelManager.setCurrentLevel(levelManager.currentLevel + 1);
                        LevelInfo level = levelManager.getCurrentLevel();

                        if (obstacles) delete obstacles;
                        if (scoreManager) delete scoreManager;

                        obstacles = new ObstacleManager(player.groundY, level.obstacleSpeed, SCREEN_WIDTH);
                        obstacles->spawnInterval = level.spawnInterval;
                        obstacles->meteorInterval = level.meteorInterval;

                        scoreManager = new ScoreManager(player.groundY, level.obstacleSpeed, SCREEN_WIDTH);

                        player.x = 50;
                        player.y = player.groundY;
                        player.vy = 0;
                        player.isOnGround = true;
                        gameOver = false;
                        state = GameState::PLAYING;
                    }
                } else if (e.key.keysym.sym == SDLK_m) {
                    state = GameState::LEVEL_SELECT;
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

            if (obstacles) obstacles->update();
            if (scoreManager) {
                scoreManager->update(player.x, player.y, player.width, player.height);

                // Kiểm tra hoàn thành màn chơi
                if (levelManager.isLevelComplete(scoreManager->getCurrentScore())) {
                    levelManager.updateBestScore(scoreManager->getCurrentScore());
                    levelManager.unlockNextLevel();
                    state = GameState::LEVEL_COMPLETE;
                }
            }

            if (obstacles && obstacles->checkCollisionWithPlayer(player.x, player.y, player.width, player.height)) {
                gameOver = true;
                if (scoreManager) {
                    levelManager.updateBestScore(scoreManager->getCurrentScore());
                }
                state = GameState::GAME_OVER;
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
        SDL_RenderClear(renderer);

        if (state == GameState::MENU) {
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Color black = { 0, 0, 0, 255 };

            renderCenteredText(renderer, fontBig, "DINO ADVENTURE", black, 80, SCREEN_WIDTH);

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
        }
        else if (state == GameState::LEVEL_SELECT) {
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Color black = { 0, 0, 0, 255 };
            SDL_Color green = { 0, 200, 0, 255 };
            SDL_Color gray = { 128, 128, 128, 255 };

            renderCenteredText(renderer, fontBig, "SELECT LEVEL", black, 30, SCREEN_WIDTH);

            int mx, my;
            SDL_GetMouseState(&mx, &my);

            // Vẽ các nút màn chơi
            for (size_t i = 0; i < levelManager.levels.size(); i++) {
                LevelInfo& level = levelManager.levels[i];
                int btnX = 50 + (i % 3) * 250;
                int btnY = 120 + (i / 3) * 100;
                SDL_Rect levelBtn = { btnX, btnY, 200, 80 };

                bool hovered = (mx >= levelBtn.x && mx <= levelBtn.x + levelBtn.w &&
                               my >= levelBtn.y && my <= levelBtn.y + levelBtn.h);

                if (level.unlocked) {
                    SDL_SetRenderDrawColor(renderer,
                        hovered ? 100 : 50,
                        hovered ? 180 : 150,
                        50, 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
                }

                SDL_RenderFillRect(renderer, &levelBtn);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &levelBtn);

                std::string levelText = "Level " + std::to_string(level.levelNumber);
                renderLeftText(renderer, fontSmall, levelText,
                    level.unlocked ? black : gray, btnX + 10, btnY + 10);

                if (level.unlocked) {
                    renderLeftText(renderer, fontTiny, level.name, black, btnX + 10, btnY + 40);
                    std::string target = "Goal: " + std::to_string(level.targetScore);
                    renderLeftText(renderer, fontTiny, target, green, btnX + 10, btnY + 60);
                } else {
                    renderLeftText(renderer, fontTiny, "LOCKED", gray, btnX + 50, btnY + 45);
                }
            }

            // Nút back
            SDL_Rect backBtn = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 70, 200, 50 };
            bool hovBack = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                           my >= backBtn.y && my <= backBtn.y + backBtn.h);

            SDL_SetRenderDrawColor(renderer, hovBack ? 150 : 100, 50, 50, 255);
            SDL_RenderFillRect(renderer, &backBtn);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &backBtn);
            renderCenteredText(renderer, fontSmall, "BACK", white, backBtn.y + 10, SCREEN_WIDTH);
        }
        else if (state == GameState::PLAYING || state == GameState::GAME_OVER) {
            // Ground
            SDL_SetRenderDrawColor(renderer, 10, 139, 34, 255);
            SDL_Rect ground = { 0, player.groundY, SCREEN_WIDTH, SCREEN_HEIGHT - player.groundY };
            SDL_RenderFillRect(renderer, &ground);

            // Obstacles, Coins & Player
            if (obstacles) obstacles->render(renderer);
            if (scoreManager) scoreManager->render(renderer);
            SDL_Rect rect = { player.x, player.y - player.height, player.width, player.height };
            SDL_RenderCopy(renderer, dino, nullptr, &rect);

            // Hiển thị thông tin màn chơi
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Color yellow = { 255, 255, 0, 255 };
            SDL_Color black = { 0, 0, 0, 255 };

            LevelInfo currentLevel = levelManager.getCurrentLevel();
            std::string levelInfo = "Level " + std::to_string(currentLevel.levelNumber) + ": " + currentLevel.name;
            renderLeftText(renderer, fontSmall, levelInfo, black, 10, 10);

            if (scoreManager) {
                std::string scoreText = "Score: " + std::to_string(scoreManager->getCurrentScore()) +
                                       " / " + std::to_string(currentLevel.targetScore);
                renderLeftText(renderer, fontSmall, scoreText, black, 10, 40);

                std::string coinText = "Coins: " + std::to_string(scoreManager->getTotalCoins());
                renderLeftText(renderer, fontTiny, coinText, yellow, SCREEN_WIDTH - 120, 10);
            }

            if (state == GameState::GAME_OVER) {
                renderCenteredText(renderer, fontBig, "GAME OVER", white, 120, SCREEN_WIDTH);

                if (scoreManager) {
                    std::string finalScore = "Final Score: " + std::to_string(scoreManager->getCurrentScore());
                    renderCenteredText(renderer, fontMedium, finalScore, yellow, 190, SCREEN_WIDTH);
                }

                renderCenteredText(renderer, fontSmall, "PRESS R TO RETRY OR M FOR MENU",
                                   white, 260, SCREEN_WIDTH);
            }
        }
        else if (state == GameState::LEVEL_COMPLETE) {
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Color gold = { 255, 215, 0, 255 };
            SDL_Color green = { 0, 255, 0, 255 };

            renderCenteredText(renderer, fontBig, "LEVEL COMPLETE!", green, 100, SCREEN_WIDTH);

            LevelInfo currentLevel = levelManager.getCurrentLevel();
            std::string levelText = "Level " + std::to_string(currentLevel.levelNumber) + " - " + currentLevel.name;
            renderCenteredText(renderer, fontMedium, levelText, white, 170, SCREEN_WIDTH);

            if (scoreManager) {
                std::string scoreText = "Final Score: " + std::to_string(scoreManager->getCurrentScore());
                renderCenteredText(renderer, fontMedium, scoreText, gold, 220, SCREEN_WIDTH);
            }

            if (levelManager.currentLevel < levelManager.levels.size() - 1) {
                renderCenteredText(renderer, fontSmall, "Next level unlocked!", green, 270, SCREEN_WIDTH);
                renderCenteredText(renderer, fontSmall, "Press N for NEXT LEVEL or M for MENU",
                                   white, 320, SCREEN_WIDTH);
            } else {
                renderCenteredText(renderer, fontSmall, "Congratulations! All levels complete!",
                                   gold, 270, SCREEN_WIDTH);
                renderCenteredText(renderer, fontSmall, "Press M for MENU", white, 320, SCREEN_WIDTH);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Cleanup
    if (obstacles) delete obstacles;
    if (scoreManager) delete scoreManager;
    TTF_CloseFont(fontBig);
    TTF_CloseFont(fontMedium);
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontTiny);
    SDL_DestroyTexture(dino);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

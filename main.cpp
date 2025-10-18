#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <functional>
#include "player.h"
#include "obstacle.h"
#include "score.h"

// Forward declaration
class Shop;

enum class GameState { MENU, LEVEL_SELECT, PLAYING, GAME_OVER, LEVEL_COMPLETE, SHOP };

// ===================== Shop Item Structure =====================
struct ShopItem {
    int id;
    std::string name;
    std::string description;
    int price;
    bool isOwned;
    SDL_Texture* texture;
    SDL_Texture* previewTexture;

    ShopItem(int itemId, const std::string& itemName, const std::string& desc, int itemPrice)
        : id(itemId), name(itemName), description(desc), price(itemPrice),
          isOwned(false), texture(nullptr), previewTexture(nullptr) {}
};

// ===================== Shop Class =====================
class Shop {
public:
    std::vector<ShopItem> items;
    int selectedIndex;
    int scrollOffset;

    Shop() : selectedIndex(0), scrollOffset(0) {}

    SDL_Texture* createColoredTexture(SDL_Renderer* renderer, SDL_Color color, int w, int h) {
        SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_TARGET, w, h);
        if (tex) {
            SDL_SetRenderTarget(renderer, tex);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect body = {5, 10, 30, 35};
            SDL_RenderFillRect(renderer, &body);
            SDL_Rect head = {10, 5, 20, 15};
            SDL_RenderFillRect(renderer, &head);
            SDL_Rect leg1 = {10, 40, 8, 15};
            SDL_RenderFillRect(renderer, &leg1);
            SDL_Rect leg2 = {22, 40, 8, 15};
            SDL_RenderFillRect(renderer, &leg2);

            SDL_SetRenderTarget(renderer, nullptr);
        }
        return tex;
    }

    void initialize(SDL_Renderer* renderer) {
        items.clear();

        items.push_back(ShopItem(0, "Classic Dino", "The original dino", 0));
        items.push_back(ShopItem(1, "Red Dragon", "Fierce red dragon", 100));
        items.push_back(ShopItem(2, "Blue Raptor", "Fast blue raptor", 150));
        items.push_back(ShopItem(3, "Golden Rex", "Legendary golden T-Rex", 300));
        items.push_back(ShopItem(4, "Purple Ghost", "Mysterious ghost dino", 200));
        items.push_back(ShopItem(5, "Green Turtle", "Slow but steady", 80));
        items.push_back(ShopItem(6, "Rainbow Dino", "Colorful party dino", 500));

        items[0].isOwned = true;

        const char* skinFiles[] = {
            "image/dino.png",
            "image/dino_red.png",
            "image/dino_blue.png",
            "image/dino_gold.png",
            "image/dino_purple.png",
            "image/dino_green.png",
            "image/dino_rainbow.png"
        };

        SDL_Color skinColors[] = {
            {100, 200, 100, 255},
            {255, 50, 50, 255},
            {50, 100, 255, 255},
            {255, 215, 0, 255},
            {150, 50, 200, 255},
            {34, 139, 34, 255},
            {255, 100, 200, 255}
        };

        for (size_t i = 0; i < items.size() && i < 7; i++) {
            items[i].texture = IMG_LoadTexture(renderer, skinFiles[i]);

            if (!items[i].texture) {
                items[i].texture = createColoredTexture(renderer, skinColors[i], 40, 60);
            }

            items[i].previewTexture = items[i].texture;
        }
    }

    SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface) return nullptr;
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

    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall,
                TTF_Font* fontTiny, const Player& player) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color black = {0, 0, 0, 255};
        SDL_Color green = {0, 200, 0, 255};
        SDL_Color gray = {128, 128, 128, 255};
        SDL_Color yellow = {255, 215, 0, 255};
        SDL_Color orange = {255, 140, 0, 255};

        renderCenteredText(renderer, fontBig, "DINO SHOP", black, 20, 800);

        std::string coinText = "Your Coins: " + std::to_string(player.totalCoins);
        renderLeftText(renderer, fontSmall, coinText, orange, 20, 80);

        std::string levelText = "Level " + std::to_string(player.level) +
                               " (XP: " + std::to_string(player.xp) + "/" +
                               std::to_string(player.xpToNextLevel) + ")";
        renderLeftText(renderer, fontTiny, levelText, black, 20, 115);

        int mx, my;
        SDL_GetMouseState(&mx, &my);

        int startY = 150;
        int itemsPerRow = 3;
        int itemWidth = 220;
        int itemHeight = 140;
        int spacing = 20;
        int startX = 50;

        for (size_t i = 0; i < items.size(); i++) {
            int col = i % itemsPerRow;
            int row = i / itemsPerRow;

            int x = startX + col * (itemWidth + spacing);
            int y = startY + row * (itemHeight + spacing);

            SDL_Rect itemRect = {x, y, itemWidth, itemHeight};

            bool hovered = (mx >= itemRect.x && mx <= itemRect.x + itemRect.w &&
                          my >= itemRect.y && my <= itemRect.y + itemRect.h);

            if (items[i].isOwned) {
                if (player.equippedSkinIndex == items[i].id) {
                    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, hovered ? 100 : 80,
                                         hovered ? 220 : 200, 80, 255);
                }
            } else {
                SDL_SetRenderDrawColor(renderer, hovered ? 150 : 120,
                                     hovered ? 150 : 120,
                                     hovered ? 150 : 120, 255);
            }

            SDL_RenderFillRect(renderer, &itemRect);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &itemRect);

            if (items[i].previewTexture) {
                SDL_Rect previewRect = {x + itemWidth/2 - 20, y + 10, 40, 60};
                SDL_RenderCopy(renderer, items[i].previewTexture, nullptr, &previewRect);
            }

            renderLeftText(renderer, fontTiny, items[i].name, black, x + 10, y + 75);

            if (items[i].isOwned) {
                if (player.equippedSkinIndex == items[i].id) {
                    renderLeftText(renderer, fontTiny, "EQUIPPED", green, x + 10, y + 95);
                } else {
                    renderLeftText(renderer, fontTiny, "Click to Equip", green, x + 10, y + 95);
                }
            } else {
                std::string priceText = std::to_string(items[i].price) + " coins";
                SDL_Color priceColor = (player.totalCoins >= items[i].price) ? green : gray;
                renderLeftText(renderer, fontTiny, priceText, priceColor, x + 10, y + 95);

                if (player.totalCoins >= items[i].price) {
                    renderLeftText(renderer, fontTiny, "Click to Buy", black, x + 10, y + 115);
                } else {
                    renderLeftText(renderer, fontTiny, "Not enough coins", gray, x + 10, y + 115);
                }
            }
        }

        SDL_Rect backBtn = {300, 420, 200, 50};
        bool hovBack = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                       my >= backBtn.y && my <= backBtn.y + backBtn.h);

        SDL_SetRenderDrawColor(renderer, hovBack ? 180 : 150, 50, 50, 255);
        SDL_RenderFillRect(renderer, &backBtn);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &backBtn);
        renderCenteredText(renderer, fontSmall, "BACK TO MENU", white, backBtn.y + 12, 800);
    }

    bool handleInput(SDL_Event& e, Player& player, std::function<void()> saveCallback) {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x;
            int my = e.button.y;

            SDL_Rect backBtn = {300, 420, 200, 50};
            if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                my >= backBtn.y && my <= backBtn.y + backBtn.h) {
                return true;
            }

            int startY = 150;
            int itemsPerRow = 3;
            int itemWidth = 220;
            int itemHeight = 140;
            int spacing = 20;
            int startX = 50;

            for (size_t i = 0; i < items.size(); i++) {
                int col = i % itemsPerRow;
                int row = i / itemsPerRow;

                int x = startX + col * (itemWidth + spacing);
                int y = startY + row * (itemHeight + spacing);

                SDL_Rect itemRect = {x, y, itemWidth, itemHeight};

                if (mx >= itemRect.x && mx <= itemRect.x + itemRect.w &&
                    my >= itemRect.y && my <= itemRect.y + itemRect.h) {

                    if (items[i].isOwned) {
                        player.equippedSkinIndex = items[i].id;
                        if (saveCallback) saveCallback();
                    } else {
                        if (player.totalCoins >= items[i].price) {
                            player.totalCoins -= items[i].price;
                            items[i].isOwned = true;
                            player.equippedSkinIndex = items[i].id;
                            if (saveCallback) saveCallback();
                        }
                    }
                    break;
                }
            }
        }

        return false;
    }

    void cleanup() {
        for (auto& item : items) {
            if (item.texture) {
                SDL_DestroyTexture(item.texture);
                item.texture = nullptr;
            }
            item.previewTexture = nullptr;
        }
        items.clear();
    }
};

// ===================== Level Info Structure =====================
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

// ===================== Level Manager =====================
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

// ===================== Save/Load Progress =====================
void savePlayerProgress(const Player& player, const Shop& shop, const LevelManager& levelManager) {
    std::ofstream file("game_progress.dat", std::ios::trunc);
    if (file.is_open()) {
        for (const auto& level : levelManager.levels) {
            file << level.unlocked << " " << level.bestScore << "\n";
        }

        file << player.totalCoins << "\n";
        file << player.equippedSkinIndex << "\n";
        file << player.level << "\n";
        file << player.xp << "\n";
        file << player.xpToNextLevel << "\n";

        for (const auto& item : shop.items) {
            file << item.isOwned << " ";
        }
        file << "\n";

        file.close();
    }
}

void loadPlayerProgress(Player& player, Shop& shop, LevelManager& levelManager) {
    std::ifstream file("game_progress.dat");
    bool fileExists = file.good();

    if (fileExists) {
        levelManager.loadProgress();

        for (size_t i = 0; i < levelManager.levels.size(); ++i) {
            std::string line;
            std::getline(file, line);
        }

        if (!(file >> player.totalCoins)) player.totalCoins = 0;
        if (!(file >> player.equippedSkinIndex)) player.equippedSkinIndex = 0;
        if (!(file >> player.level)) player.level = 1;
        if (!(file >> player.xp)) player.xp = 0;
        if (!(file >> player.xpToNextLevel)) player.xpToNextLevel = 100;

        if (!shop.items.empty()) {
            for (auto& item : shop.items) {
                if (!(file >> item.isOwned)) {
                    item.isOwned = (item.id == 0);
                }
            }
            shop.items[0].isOwned = true;
        }

        file.close();
    } else {
        player.totalCoins = 0;
        player.equippedSkinIndex = 0;
        player.level = 1;
        player.xp = 0;
        player.xpToNextLevel = 100;
        if (!shop.items.empty()) {
            for(auto& item : shop.items) item.isOwned = (item.id == 0);
            shop.items[0].isOwned = true;
        }
        levelManager.initializeLevels();
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
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cout << "IMG_Init Error: " << IMG_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 480;

    SDL_Window* window = SDL_CreateWindow("Dino Game - Shop System",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* fontBig = TTF_OpenFont("NotoSans-Regular.ttf", 48);
    TTF_Font* fontMedium = TTF_OpenFont("NotoSans-Regular.ttf", 32);
    TTF_Font* fontSmall = TTF_OpenFont("NotoSans-Regular.ttf", 24);
    TTF_Font* fontTiny = TTF_OpenFont("NotoSans-Regular.ttf", 18);
    if (!fontBig || !fontSmall || !fontMedium || !fontTiny) {
        std::cout << "Failed to load font! Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Player player;
    ObstacleManager* obstacles = nullptr;
    ScoreManager* scoreManager = nullptr;
    LevelManager levelManager;
    Shop shop;

    shop.initialize(renderer);
    loadPlayerProgress(player, shop, levelManager);

    GameState state = GameState::MENU;
    bool running = true;
    bool gameOver = false;
    SDL_Event e;

    SDL_Rect playBtn = { SCREEN_WIDTH / 2 - 150, 180, 300, 70 };
    SDL_Rect shopBtn = { SCREEN_WIDTH / 2 - 150, 270, 300, 70 };
    SDL_Rect quitBtn = { SCREEN_WIDTH / 2 - 150, 360, 300, 70 };

    auto saveCallback = [&]() {
        savePlayerProgress(player, shop, levelManager);
    };

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
            else if (state == GameState::MENU && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;
                if (mx >= playBtn.x && mx <= playBtn.x + playBtn.w &&
                    my >= playBtn.y && my <= playBtn.y + playBtn.h) {
                    state = GameState::LEVEL_SELECT;
                } else if (mx >= shopBtn.x && mx <= shopBtn.x + shopBtn.w &&
                           my >= shopBtn.y && my <= shopBtn.y + shopBtn.h) {
                    state = GameState::SHOP;
                } else if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                           my >= quitBtn.y && my <= quitBtn.y + quitBtn.h) {
                    running = false;
                }
            }
            else if (state == GameState::SHOP) {
                if (shop.handleInput(e, player, saveCallback)) {
                    state = GameState::MENU;
                }
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::MENU;
                }
            }
            else if (state == GameState::LEVEL_SELECT && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;

                for (size_t i = 0; i < levelManager.levels.size(); i++) {
                    int btnX = 50 + (i % 3) * 250;
                    int btnY = 120 + (i / 3) * 100;
                    SDL_Rect levelBtn = { btnX, btnY, 200, 80 };

                    if (mx >= levelBtn.x && mx <= levelBtn.x + levelBtn.w &&
                        my >= levelBtn.y && my <= levelBtn.y + levelBtn.h &&
                        levelManager.levels[i].unlocked) {

                        levelManager.setCurrentLevel(i);
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
                        break;
                    }
                }

                SDL_Rect backBtnLS = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 70, 200, 50 };
                if (mx >= backBtnLS.x && mx <= backBtnLS.x + backBtnLS.w &&
                    my >= backBtnLS.y && my <= backBtnLS.y + backBtnLS.h) {
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
                    if (scoreManager) scoreManager->update(player);
                } else if (e.key.keysym.sym == SDLK_LEFT) {
                    player.x -= player.vx;
                    if (obstacles) obstacles->update();
                    if (scoreManager) scoreManager->update(player);
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    if (scoreManager) {
                        levelManager.updateBestScore(scoreManager->getCurrentScore());
                    }
                    saveCallback();
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
                    saveCallback();
                    state = GameState::LEVEL_SELECT;
                }
            }
            else if (state == GameState::LEVEL_COMPLETE && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_n) {
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
                    saveCallback();
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
                scoreManager->update(player);

                if (levelManager.isLevelComplete(scoreManager->getCurrentScore())) {
                    levelManager.updateBestScore(scoreManager->getCurrentScore());
                    levelManager.unlockNextLevel();
                    player.addXp(50);
                    saveCallback();
                    state = GameState::LEVEL_COMPLETE;
                }
            }
            if (obstacles && obstacles->checkCollisionWithPlayer(player.x, player.y, player.width, player.height)) {
                gameOver = true;
                if (scoreManager) {
                    levelManager.updateBestScore(scoreManager->getCurrentScore());
                }
                saveCallback();
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

            int mx, my; SDL_GetMouseState(&mx, &my);
            bool hovPlay = (mx >= playBtn.x && mx <= playBtn.x + playBtn.w &&
                            my >= playBtn.y && my <= playBtn.y + playBtn.h);
            bool hovShop = (mx >= shopBtn.x && mx <= shopBtn.x + shopBtn.w &&
                           my >= shopBtn.y && my <= shopBtn.y + shopBtn.h);
            bool hovQuit = (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                            my >= quitBtn.y && my <= quitBtn.y + quitBtn.h);

            SDL_SetRenderDrawColor(renderer, hovPlay ? 80 : 50, hovPlay ? 160 : 100, 50, 255);
            SDL_RenderFillRect(renderer, &playBtn);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &playBtn);
            renderCenteredText(renderer, fontSmall, "PLAY", white, playBtn.y + 20, SCREEN_WIDTH);

            SDL_SetRenderDrawColor(renderer, hovShop ? 160 : 100, hovShop ? 160 : 100, 50, 255);
            SDL_RenderFillRect(renderer, &shopBtn);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &shopBtn);
            renderCenteredText(renderer, fontSmall, "SHOP", white, shopBtn.y + 20, SCREEN_WIDTH);

            SDL_SetRenderDrawColor(renderer, hovQuit ? 160 : 100, 50, 50, 255);
            SDL_RenderFillRect(renderer, &quitBtn);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &quitBtn);
            renderCenteredText(renderer, fontSmall, "QUIT", white, quitBtn.y + 20, SCREEN_WIDTH);
        }
        else if (state == GameState::SHOP) {
            SDL_SetRenderDrawColor(renderer, 230, 230, 250, 255);
            SDL_RenderClear(renderer);
            shop.render(renderer, fontBig, fontSmall, fontTiny, player);
        }
        else if (state == GameState::LEVEL_SELECT) {
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Color black = { 0, 0, 0, 255 };
            SDL_Color green = { 0, 200, 0, 255 };
            SDL_Color gray = { 128, 128, 128, 255 };

            renderCenteredText(renderer, fontBig, "SELECT LEVEL", black, 30, SCREEN_WIDTH);

            int mx, my;
            SDL_GetMouseState(&mx, &my);

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

            SDL_Rect backBtnLS = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 70, 200, 50 };
            bool hovBackLS = (mx >= backBtnLS.x && mx <= backBtnLS.x + backBtnLS.w &&
                           my >= backBtnLS.y && my <= backBtnLS.y + backBtnLS.h);

            SDL_SetRenderDrawColor(renderer, hovBackLS ? 150 : 100, 50, 50, 255);
            SDL_RenderFillRect(renderer, &backBtnLS);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &backBtnLS);
            renderCenteredText(renderer, fontSmall, "BACK", white, backBtnLS.y + 10, SCREEN_WIDTH);
        }
        else if (state == GameState::PLAYING || state == GameState::GAME_OVER) {
            SDL_SetRenderDrawColor(renderer, 10, 139, 34, 255);
            SDL_Rect ground = { 0, player.groundY, SCREEN_WIDTH, SCREEN_HEIGHT - player.groundY };
            SDL_RenderFillRect(renderer, &ground);

            if (obstacles) obstacles->render(renderer);
            if (scoreManager) scoreManager->render(renderer);

            SDL_Texture* currentPlayerTexture = nullptr;
            if (player.equippedSkinIndex >= 0 && player.equippedSkinIndex < shop.items.size()) {
                currentPlayerTexture = shop.items[player.equippedSkinIndex].texture;
            } else {
                currentPlayerTexture = shop.items[0].texture;
            }

            if (currentPlayerTexture) {
                SDL_Rect rect = { player.x, player.y - (int)player.height, (int)player.width, (int)player.height };
                SDL_RenderCopy(renderer, currentPlayerTexture, nullptr, &rect);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect rect = { player.x, player.y - (int)player.height, (int)player.width, (int)player.height };
                SDL_RenderFillRect(renderer, &rect);
            }

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

                std::string coinText = "Coins: " + std::to_string(player.totalCoins);
                renderLeftText(renderer, fontTiny, coinText, yellow, SCREEN_WIDTH - 150, 10);

                std::string xpText = "Lvl " + std::to_string(player.level) + " (XP: " +
                                    std::to_string(player.xp) + "/" + std::to_string(player.xpToNextLevel) + ")";
                renderLeftText(renderer, fontTiny, xpText, white, SCREEN_WIDTH - 150, 30);
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
    savePlayerProgress(player, shop, levelManager);

    if (obstacles) delete obstacles;
    if (scoreManager) delete scoreManager;
    shop.cleanup();
    TTF_CloseFont(fontBig);
    TTF_CloseFont(fontMedium);
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontTiny);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}

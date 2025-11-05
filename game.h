#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>
#include <functional>

#include "player.h"
#include "obstacle.h"
#include "score.h"
#include "powerup.h"
#include "combo_achievement.h"
#include "quest_system.h"
#include "shop.h"
#include "quest_screen.h"
#include "achievement_screen.h"
#include "daily_reset_system.h"
#include "leaderboard.h"
#include "map_theme.h"
#include "ui_renderer.h"

enum class GameState {
    MENU,
    LEVEL_SELECT,
    PLAYING,
    GAME_OVER,
    LEVEL_COMPLETE,
    SHOP,
    QUEST,
    ACHIEVEMENT,
    LEADERBOARD
};

struct LevelInfo {
    int levelNumber;
    std::string name;
    int obstacleSpeed;
    int spawnInterval;
    int meteorInterval;
    int targetScore;
    bool unlocked;
    int bestScore;
    MapThemeType themeType;
};

class LevelManager {
public:
    std::vector<LevelInfo> levels;
    int currentLevel;

    LevelManager();
    void initializeLevels();
    void saveProgress();
    void loadProgress();
    void unlockNextLevel();
    void updateBestScore(int score);
    bool isLevelComplete(int score);
    LevelInfo& getCurrentLevelInfo();
    void setCurrentLevel(int index);
};

class Game {
private:
    // Window and rendering
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* fontBig;
    TTF_Font* fontMedium;
    TTF_Font* fontSmall;
    TTF_Font* fontTiny;
    Mix_Music* backgroundMusic;

    // Game systems
    Player player;
    UIRenderer uiRenderer;
    ObstacleManager obstacleManager;
    ScoreManager scoreManager;
    PowerUpManager powerUpManager;
    LevelManager levelManager;
    Shop shop;
    ComboSystem comboSystem;
    AchievementSystem achievementSystem;
    DifficultyManager difficultyManager;
    QuestSystem questSystem;
    QuestScreen questScreen;
    AchievementScreen achievementScreen;
    DailyResetSystem dailyResetSystem;
    Leaderboard leaderboard;

    // Map and environment
    MapTheme mapTheme;
    DayNightCycle dayNightCycle;

    // Game state
    GameState state;
    bool running;
    bool gameOver;
    bool musicPlaying;

    // Screen dimensions
    const int SCREEN_WIDTH;
    const int SCREEN_HEIGHT;
    const int GROUND_Y;

public:
    Game(int width = 800, int height = 600);
    ~Game();

    bool initialize();
    void run();
    void cleanup();

private:
    void handleEvents();
    void update();
    void render();

    void handleMenuInput(SDL_Event& e);
    void handleLevelSelectInput(SDL_Event& e);
    void handlePlayingInput(SDL_Event& e);
    void handleGameOverInput(SDL_Event& e);
    void handleLevelCompleteInput(SDL_Event& e);
    void handleShopInput(SDL_Event& e);
    void handleQuestInput(SDL_Event& e);
    void handleAchievementInput(SDL_Event& e);
    void handleLeaderboardInput(SDL_Event& e);

    void renderMenu();
    void renderLevelSelect();
    void renderPlaying();
    void renderGameOver();
    void renderLevelComplete();
    void renderShop();
    void renderQuest();
    void renderAchievement();
    void renderLeaderboard();

    void saveProgress();
    void loadProgress();

    // Helper functions
    SDL_Texture* renderText(TTF_Font* font, const std::string& text, SDL_Color color);
    void renderCenteredText(TTF_Font* font, const std::string& text, SDL_Color color, int y, int screenW);
    void renderLeftText(TTF_Font* font, const std::string& text, SDL_Color color, int x, int y);

    void resetGame();
    void startLevel(int levelIndex);
};

#endif // GAME_H_INCLUDED

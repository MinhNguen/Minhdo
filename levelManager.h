#ifndef LEVELMANAGER_H_INCLUDED
#define LEVELMANAGER_H_INCLUDED
#include <string>
#include "map_theme.h"
#include <vector>
#include <fstream>

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

#endif // LEVELMANAGER_H_INCLUDED

#include "levelManager.h"


LevelManager::LevelManager() {
    currentLevel = 0;
    initializeLevels();
    loadProgress();
}

void LevelManager::initializeLevels() {
    levels = {
        {1, "Easy Valley", 6, 90, 250, 50, true, 0, GRASSLAND},
        {2, "Rocky Hills", 7, 80, 220, 100, false, 0, DESERT},
        {3, "Desert Storm", 8, 70, 200, 150, false, 0, FOREST},
        {4, "Thunder Plains", 9, 60, 180, 200, false, 0, MOUNTAIN},
        {5, "Volcano Peak", 10, 50, 150, 300, false, 0, VOLCANO}
    };
}

void LevelManager::saveProgress() {
    std::ofstream file("game_progress.dat");
    if (file.is_open()) {
        for (const auto& level : levels) {
            file << level.unlocked << " " << level.bestScore << "\n";
        }
        file.close();
    }
}

void LevelManager::loadProgress() {
    std::ifstream file("game_progress.dat");
    if (file.is_open()) {
        for (auto& level : levels) {
            file >> level.unlocked >> level.bestScore;
        }
        file.close();
    }
}

void LevelManager::unlockNextLevel() {
    if (static_cast<size_t>(currentLevel) < levels.size() - 1) {
        levels[currentLevel + 1].unlocked = true;
        saveProgress();
    }
}

void LevelManager::updateBestScore(int score) {
    if (score > levels[currentLevel].bestScore) {
        levels[currentLevel].bestScore = score;
        saveProgress();
    }
}

bool LevelManager::isLevelComplete(int score) {
    return score >= levels[currentLevel].targetScore;
}

LevelInfo& LevelManager::getCurrentLevelInfo() {
    return levels[currentLevel];
}

void LevelManager::setCurrentLevel(int index) {
    if (index >= 0 && static_cast<size_t>(index) < levels.size()) {
        currentLevel = index;
    }
}

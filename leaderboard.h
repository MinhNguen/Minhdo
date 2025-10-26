#ifndef LEADERBOARD_H_INCLUDED
#define LEADERBOARD_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>
#include "player.h"
#include "ui_renderer.h"

struct LeaderboardEntry {
    std::string playerName;
    int score;
    int level;
    std::string date;
    LeaderboardEntry() : playerName("PLAYER"), score(100), level(100), date("1/1/2000") {}
    LeaderboardEntry(const std::string& name, int s, int lvl, const std::string& d)
        : playerName(name), score(s), level(lvl), date(d) {}
};

class Leaderboard {
private:
    std::vector<LeaderboardEntry> entries;
    std::string saveFile;
    const int MAX_ENTRIES = 10;

public:
    Leaderboard(const std::string& file = "leaderboard.dat") : saveFile(file) {
        loadEntries();
        ensureDefaultEntries();
    }

    void ensureDefaultEntries() {
        if (entries.empty()) {
            addEntry("Player", 250, 3);
            addEntry("ProGamer", 180, 2);
            addEntry("Newbie", 75, 1);
            addEntry("Champion", 320, 4);
            addEntry("Rookie", 50, 1);
        }
    }

    void addEntry(const std::string& name, int score, int level) {
        time_t now = time(nullptr);
        char dateStr[32];
        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&now));

        entries.emplace_back(name, score, level, dateStr);

        std::sort(entries.begin(), entries.end(),
            [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
                return a.score > b.score;
            });

        if (entries.size() > MAX_ENTRIES) {
            entries.resize(MAX_ENTRIES);
        }

        saveEntries();
    }

    bool isHighScore(int score) {
        if (entries.size() < MAX_ENTRIES) return true;
        return score > entries.back().score;
    }

    int getRank(int score) {
        int rank = 1;
        for (const auto& entry : entries) {
            if (score > entry.score) break;
            rank++;
        }
        return rank;
    }

    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall,
                TTF_Font* fontTiny, UIRenderer& uiRenderer) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gold = {255, 215, 0, 255};
        SDL_Color silver = {192, 192, 192, 255};
        SDL_Color bronze = {205, 127, 50, 255};

        // Title panel
        SDL_FRect titlePanel = {200, 10, 400, 70};
        uiRenderer.drawEnhancedGlassPanel(titlePanel, {200, 150, 100, 180});
        uiRenderer.renderTextCentered("LEADERBOARD", 400, 45, fontBig, white);

        // Header
        SDL_FRect headerPanel = {100, 90, 600, 40};
        uiRenderer.drawEnhancedGlassPanel(headerPanel, {80, 80, 150, 200});
        uiRenderer.renderTextLeft("Rank", 120, 105, fontSmall, white);
        uiRenderer.renderTextLeft("Player", 220, 105, fontSmall, white);
        uiRenderer.renderTextLeft("Score", 420, 105, fontSmall, white);
        uiRenderer.renderTextLeft("Level", 520, 105, fontSmall, white);
        uiRenderer.renderTextLeft("Date", 600, 105, fontSmall, white);

        // Entries
        int startY = 140;
        for (size_t i = 0; i < entries.size(); i++) {
            int y = startY + i * 45;
            SDL_FRect entryPanel = {100, (float)y, 600, 40};

            SDL_Color bgColor;
            SDL_Color rankColor = white;

            if (i == 0) {
                bgColor = {255, 215, 0, 150}; // Gold
                rankColor = {255, 215, 0, 255};
            } else if (i == 1) {
                bgColor = {192, 192, 192, 150}; // Silver
                rankColor = silver;
            } else if (i == 2) {
                bgColor = {205, 127, 50, 150}; // Bronze
                rankColor = bronze;
            } else {
                bgColor = {100, 100, 150, 120};
            }

            uiRenderer.drawEnhancedGlassPanel(entryPanel, bgColor);

            // Rank với medal cho top 3
            std::string rankStr = "#" + std::to_string(i + 1);
            uiRenderer.renderTextLeft(rankStr, 130, y + 10, fontSmall, rankColor);

            // Player info
            uiRenderer.renderTextLeft(entries[i].playerName, 220, y + 10, fontTiny, white);
            uiRenderer.renderTextLeft(std::to_string(entries[i].score), 440, y + 10, fontTiny, white);
            uiRenderer.renderTextLeft("Lvl " + std::to_string(entries[i].level), 530, y + 10, fontTiny, white);
            uiRenderer.renderTextLeft(entries[i].date, 600, y + 10, fontTiny, {200, 200, 200, 255});
        }

        // Empty state
        if (entries.empty()) {
            uiRenderer.renderTextCentered("No entries yet. Be the first!", 400, 300,
                                         fontSmall, {200, 200, 200, 255});
        }

        // Back button
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        SDL_FRect backBtn = {300, 520, 200, 50};
        bool hovBack = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                        my >= backBtn.y && my <= backBtn.y + backBtn.h);
        uiRenderer.renderEnhancedButton(backBtn, hovBack, "BACK", fontSmall, {180, 50, 50, 255});
    }

    bool handleInput(SDL_Event& e) {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            SDL_Rect backBtn = {300, 520, 200, 50};
            if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                my >= backBtn.y && my <= backBtn.y + backBtn.h) {
                return true;
            }
        } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            return true;
        }
        return false;
    }

    void saveEntries() {
        std::ofstream file(saveFile);
        if (file.is_open()) {
            file << entries.size() << "\n";
            for (const auto& entry : entries) {
                file << entry.playerName << "\n"
                     << entry.score << " " << entry.level << "\n"
                     << entry.date << "\n";
            }
            file.close();
        }
    }

    void loadEntries() {
        std::ifstream file(saveFile);
        if (file.is_open()) {
            size_t count;
            file >> count;
            file.ignore();

            entries.clear();
            for (size_t i = 0; i < count; i++) {
                std::string name, date;
                int score, level;

                std::getline(file, name);
                file >> score >> level;
                file.ignore();
                std::getline(file, date);

                entries.emplace_back(name, score, level, date);
            }
            file.close();
        }
    }

    const std::vector<LeaderboardEntry>& getEntries() const { return entries; }
};

#endif // LEADERBOARD_H_INCLUDED

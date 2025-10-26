#ifndef DAILY_RESET_SYSTEM_H_INCLUDED
#define DAILY_RESET_SYSTEM_H_INCLUDED

#include <ctime>
#include <fstream>
#include <string>

class DailyResetSystem {
private:
    time_t lastResetTime;
    std::string saveFile;

public:
    DailyResetSystem(const std::string& file = "daily_reset.dat") : saveFile(file) {
        loadLastResetTime();
    }

    bool shouldResetDaily() {
        time_t now = time(nullptr);
        tm* nowTm = localtime(&now);
        tm* lastTm = localtime(&lastResetTime);

        // Kiểm tra nếu đã qua ngày mới
        if (nowTm->tm_year != lastTm->tm_year ||
            nowTm->tm_yday != lastTm->tm_yday) {
            return true;
        }
        return false;
    }

    void markReset() {
        lastResetTime = time(nullptr);
        saveLastResetTime();
    }

    void saveLastResetTime() {
        std::ofstream file(saveFile);
        if (file.is_open()) {
            file << lastResetTime;
            file.close();
        }
    }

    void loadLastResetTime() {
        std::ifstream file(saveFile);
        if (file.is_open()) {
            file >> lastResetTime;
            file.close();
        } else {
            lastResetTime = time(nullptr);
            saveLastResetTime();
        }
    }

    std::string getTimeUntilReset() {
        time_t now = time(nullptr);
        tm* nowTm = localtime(&now);

        // Tính thời gian đến 00:00 ngày mai
        tm tomorrow = *nowTm;
        tomorrow.tm_mday += 1;
        tomorrow.tm_hour = 0;
        tomorrow.tm_min = 0;
        tomorrow.tm_sec = 0;

        time_t tomorrowTime = mktime(&tomorrow);
        int secondsLeft = static_cast<int>(difftime(tomorrowTime, now));

        int hours = secondsLeft / 3600;
        int minutes = (secondsLeft % 3600) / 60;

        return std::to_string(hours) + "h " + std::to_string(minutes) + "m";
    }
};

#endif // DAILY_RESET_SYSTEM_H_INCLUDED

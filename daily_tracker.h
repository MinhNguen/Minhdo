#ifndef DAILY_TRACKER_H_INCLUDED
#define DAILY_TRACKER_H_INCLUDED

#include <ctime>
#include <string>

class DailyTracker {
private:
    time_t lastCheckTime;
    std::string trackerFile;

public:
    DailyTracker(const std::string& file = "daily_tracker.dat") : trackerFile(file) {
        loadLastCheckTime();
    }

    bool isNewDay() {
        time_t now = time(nullptr);
        tm* nowTm = localtime(&now);
        tm* lastTm = localtime(&lastCheckTime);

        return (nowTm->tm_year != lastTm->tm_year ||
                nowTm->tm_yday != lastTm->tm_yday);
    }

    void updateCheckTime() {
        lastCheckTime = time(nullptr);
        saveLastCheckTime();
    }

    std::string getDaysSinceLastPlay() {
        time_t now = time(nullptr);
        double seconds = difftime(now, lastCheckTime);
        int days = static_cast<int>(seconds / (60 * 60 * 24));

        if (days == 0) return "Today";
        else if (days == 1) return "1 day ago";
        else return std::to_string(days) + " days ago";
    }

private:
    void saveLastCheckTime() {
        std::ofstream file(trackerFile);
        if (file.is_open()) {
            file << lastCheckTime;
            file.close();
        }
    }

    void loadLastCheckTime() {
        std::ifstream file(trackerFile);
        if (file.is_open()) {
            file >> lastCheckTime;
            file.close();
        } else {
            lastCheckTime = time(nullptr);
            saveLastCheckTime();
        }
    }
};

#endif // DAILY_TRACKER_H_INCLUDED

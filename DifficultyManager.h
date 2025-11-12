#ifndef DIFFICULTYMANAGER_H_INCLUDED
#define DIFFICULTYMANAGER_H_INCLUDED
#pragma one

class DifficultyManager {
public:
    int survivalTime;
    float baseSpeed, currentSpeed, speedIncrement;
    int speedIncreaseInterval, nextSpeedIncrease;

    DifficultyManager() : baseSpeed(6.0f), speedIncrement(0.5f), speedIncreaseInterval(600) { reset(); }

    void update() {
        survivalTime++;
        if (survivalTime >= nextSpeedIncrease) {
            currentSpeed += speedIncrement;
            nextSpeedIncrease += speedIncreaseInterval;
        }
    }
    void reset() {
        survivalTime = 0;
        currentSpeed = baseSpeed;
        nextSpeedIncrease = speedIncreaseInterval;
    }
    float getSpeed() const { return currentSpeed; }
};



#endif // DIFFICULTYMANAGER_H_INCLUDED

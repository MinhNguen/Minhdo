#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

struct Player {
    int x, y;
    float width, height;
    int vx;           // Vận tốc ngang
    float vy;         // Vận tốc dọc (cho nhảy)
    float gravity;    // Gia tốc trọng lực
    bool isOnGround;  // Kiểm tra có đang đứng trên sàn không
    int groundY;      // Vị trí mặt sàn

    // Thuộc tính mới
    int level;
    int xp;
    int xpToNextLevel;
    int totalCoins;
    int equippedSkinIndex;

    Player() : x(50), y(380), width(40), height(60),
               vx(5), vy(0), gravity(0.5f),
               isOnGround(true), groundY(380),
               level(1), xp(0), xpToNextLevel(100),
               totalCoins(0),
               equippedSkinIndex(0)
               {}

    void addXp(int amount) {
        xp += amount;
        while (xp >= xpToNextLevel) {
            levelUp();
        }
    }

    void levelUp() {
        level++;
        xp -= xpToNextLevel;
        xpToNextLevel = static_cast<int>(xpToNextLevel * 1.5f);
    }
};

#endif // PLAYER_H_INCLUDED

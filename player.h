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

    Player() : x(50), y(380), width(30), height(40),
               vx(5), vy(0), gravity(0.5f),
               isOnGround(true), groundY(380) {}
};

#endif // PLAYER_H_INCLUDED

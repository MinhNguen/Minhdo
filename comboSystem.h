#ifndef COMBOSYSTEM_H_INCLUDED
#define COMBOSYSTEM_H_INCLUDED
#pragma one

class ComboSystem {
public:
    int currentCombo, maxCombo, comboTimer, comboTimeout, comboPopTimer;
    float comboMultiplier, comboScale;

    ComboSystem() { reset(); comboTimeout = 120; }

    void reset() {
        currentCombo = 0; maxCombo = 0; comboTimer = 0;
        comboMultiplier = 1.0f; comboPopTimer = 0; comboScale = 1.0f;
    }

    void addCombo() {
        currentCombo++;
        comboTimer = comboTimeout;
        if (currentCombo > maxCombo) maxCombo = currentCombo;
        comboMultiplier = 1.0f + (currentCombo / 5) * 0.5f;
        comboPopTimer = 30; comboScale = 1.5f;
    }

    void update() {
        if (currentCombo > 0 && --comboTimer <= 0) {
            currentCombo = 0; comboMultiplier = 1.0f;
        }
        if (comboPopTimer > 0) {
            comboPopTimer--;
            comboScale = 1.5f - (30 - comboPopTimer) * 0.017f;
            if (comboScale < 1.0f) comboScale = 1.0f;
        }
    }

    void render(SDL_Renderer* renderer, TTF_Font* font, int screenWidth) {
        if (currentCombo < 3) return;
        SDL_Color color = (currentCombo < 10) ? SDL_Color{255, 255, 0, 255} : (currentCombo < 20) ? SDL_Color{255, 140, 0, 255} : SDL_Color{255, 50, 50, 255};
        std::string comboText = "COMBO x" + std::to_string(currentCombo);
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, comboText.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            int w = (int)(surface->w * comboScale), h = (int)(surface->h * comboScale);
            SDL_Rect dst = {screenWidth / 2 - w / 2, 100, w, h};
            SDL_RenderCopy(renderer, texture, nullptr, &dst);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
        }
    }

    int getMaxCombo() const { return maxCombo; }
};

#endif // COMBOSYSTEM_H_INCLUDED

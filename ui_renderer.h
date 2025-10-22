#ifndef UI_RENDERER_H_INCLUDED
#define UI_RENDERER_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <algorithm>
#include <string>

class UIRenderer {
private:
    SDL_Renderer* renderer;
    float animTimer;

public:
    UIRenderer(SDL_Renderer* r) : renderer(r), animTimer(0.0f) {}

    void update() { animTimer += 0.016f; }
    float getAnimTimer() const { return animTimer; }
    SDL_Renderer* getRenderer() {
        return renderer;
    }

    // === HSV to RGB Conversion ===
    SDL_Color hsvToRgb(float h, float s, float v) {
        s = std::max(0.0f, std::min(1.0f, s));
        v = std::max(0.0f, std::min(1.0f, v));
        h = fmod(h, 360.0f);
        if (h < 0) h += 360.0f;

        float c = v * s;
        float x = c * (1.0f - abs(fmod(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;

        float r_prime = 0.0f, g_prime = 0.0f, b_prime = 0.0f;

        if (h >= 0 && h < 60) {
            r_prime = c; g_prime = x;
        } else if (h >= 60 && h < 120) {
            r_prime = x; g_prime = c;
        } else if (h >= 120 && h < 180) {
            g_prime = c; b_prime = x;
        } else if (h >= 180 && h < 240) {
            g_prime = x; b_prime = c;
        } else if (h >= 240 && h < 300) {
            r_prime = x; b_prime = c;
        } else if (h >= 300 && h < 360) {
            r_prime = c; b_prime = x;
        }

        return {
            static_cast<Uint8>((r_prime + m) * 255.0f),
            static_cast<Uint8>((g_prime + m) * 255.0f),
            static_cast<Uint8>((b_prime + m) * 255.0f),
            255
        };
    }

    // === Draw Filled Circle Quarter ===
    void fillCircleQuarter(float cx, float cy, float radius, const SDL_Color& color, int quarter) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (int dy = 0; dy <= radius; ++dy) {
            int dx = static_cast<int>(round(sqrt(radius * radius - dy * dy)));
            switch(quarter) {
                case 1: SDL_RenderDrawLineF(renderer, cx, cy - dy, cx + dx, cy - dy); break;
                case 2: SDL_RenderDrawLineF(renderer, cx - dx, cy - dy, cx, cy - dy); break;
                case 3: SDL_RenderDrawLineF(renderer, cx - dx, cy + dy, cx, cy + dy); break;
                case 4: SDL_RenderDrawLineF(renderer, cx, cy + dy, cx + dx, cy + dy); break;
            }
        }
    }

    // === Draw Circle Quarter Border ===
    void drawCircleQuarterBorder(float cx, float cy, float radius, const SDL_Color& color, int quarter, float thickness) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (float r = radius - thickness; r <= radius; r += 0.5f) {
            for (int angle = 0; angle <= 90; angle += 1) {
                float rad = angle * M_PI / 180.0f;
                float x = r * cos(rad);
                float y = r * sin(rad);
                switch(quarter) {
                    case 1: SDL_RenderDrawPointF(renderer, cx + x, cy - y); break;
                    case 2: SDL_RenderDrawPointF(renderer, cx - x, cy - y); break;
                    case 3: SDL_RenderDrawPointF(renderer, cx - x, cy + y); break;
                    case 4: SDL_RenderDrawPointF(renderer, cx + x, cy + y); break;
                }
            }
        }
    }

    // === Draw Rounded Rectangle (Filled) ===
    void drawRoundedRect(const SDL_FRect& rect, float radius, const SDL_Color& color) {
        radius = std::min(radius, rect.w / 2.0f);
        radius = std::min(radius, rect.h / 2.0f);

        if (radius <= 0.0f) {
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRectF(renderer, &rect);
            return;
        }

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

        const SDL_FRect topRect = { rect.x + radius, rect.y, rect.w - radius * 2, radius };
        const SDL_FRect midRect = { rect.x, rect.y + radius, rect.w, rect.h - radius * 2 };
        const SDL_FRect botRect = { rect.x + radius, rect.y + rect.h - radius, rect.w - radius * 2, radius };

        if (topRect.w > 0) SDL_RenderFillRectF(renderer, &topRect);
        if (midRect.h > 0) SDL_RenderFillRectF(renderer, &midRect);
        if (botRect.w > 0) SDL_RenderFillRectF(renderer, &botRect);

        fillCircleQuarter(rect.x + radius, rect.y + radius, radius, color, 2);
        fillCircleQuarter(rect.x + rect.w - radius, rect.y + radius, radius, color, 1);
        fillCircleQuarter(rect.x + radius, rect.y + rect.h - radius, radius, color, 3);
        fillCircleQuarter(rect.x + rect.w - radius, rect.y + rect.h - radius, radius, color, 4);
    }

    // === Draw Rounded Rectangle Border ===
    void drawRoundedRectBorder(const SDL_FRect& rect, float radius, const SDL_Color& color, float thickness) {
        radius = std::min(radius, rect.w / 2.0f);
        radius = std::min(radius, rect.h / 2.0f);

        if (radius < 0.0f) radius = 0.0f;
        if (thickness <= 0.0f) return;

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

        SDL_FRect topBorder = { rect.x + radius, rect.y, rect.w - radius * 2, thickness };
        SDL_FRect bottomBorder = { rect.x + radius, rect.y + rect.h - thickness, rect.w - radius * 2, thickness };
        SDL_FRect leftBorder = { rect.x, rect.y + radius, thickness, rect.h - radius * 2 };
        SDL_FRect rightBorder = { rect.x + rect.w - thickness, rect.y + radius, thickness, rect.h - radius * 2 };

        if (topBorder.w > 0) SDL_RenderFillRectF(renderer, &topBorder);
        if (bottomBorder.w > 0) SDL_RenderFillRectF(renderer, &bottomBorder);
        if (leftBorder.h > 0) SDL_RenderFillRectF(renderer, &leftBorder);
        if (rightBorder.h > 0) SDL_RenderFillRectF(renderer, &rightBorder);

        drawCircleQuarterBorder(rect.x + radius, rect.y + radius, radius, color, 2, thickness);
        drawCircleQuarterBorder(rect.x + rect.w - radius, rect.y + radius, radius, color, 1, thickness);
        drawCircleQuarterBorder(rect.x + radius, rect.y + rect.h - radius, radius, color, 3, thickness);
        drawCircleQuarterBorder(rect.x + rect.w - radius, rect.y + rect.h - radius, radius, color, 4, thickness);
    }

    // === Enhanced Button with Rainbow Glow ===
    void renderEnhancedButton(const SDL_FRect& rect, bool isHovered, const std::string& text,
                            TTF_Font* font, const SDL_Color& customColor = {0,0,0,0}) {
        const float cornerRadius = rect.h * 0.25f;

        // 1. Glow effect
        for (int layer = 0; layer < 3; ++layer) {
            float offset = layer * 2.0f;
            SDL_FRect layerRect = {
                rect.x - offset, rect.y - offset,
                rect.w + offset * 2, rect.h + offset * 2
            };

            float hue = fmod(animTimer * 50.0f + layer * 40.0f, 360.0f);
            SDL_Color rainbowColor = hsvToRgb(hue, 0.7f, isHovered ? 0.9f : 0.6f);
            rainbowColor.a = isHovered ? (180 - layer * 40) : (120 - layer * 40);

            drawRoundedRect(layerRect, cornerRadius + offset, rainbowColor);
        }

        // 2. Base color
        SDL_Color mainColor;
        if (customColor.a != 0) {
            mainColor = customColor;
            if(isHovered) {
                mainColor.r = std::min(255, mainColor.r + 30);
                mainColor.g = std::min(255, mainColor.g + 30);
                mainColor.b = std::min(255, mainColor.b + 30);
            }
        } else {
            float mainHue = fmod(animTimer * 30.0f, 360.0f);
            mainColor = hsvToRgb(mainHue, 0.5f, isHovered ? 0.8f : 0.5f);
        }
        mainColor.a = 220;
        drawRoundedRect(rect, cornerRadius, mainColor);

        // 3. Shine effect
        SDL_FRect shineRect = {
            rect.x + cornerRadius, rect.y + 2,
            rect.w - cornerRadius * 2, rect.h * 0.3f
        };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
        for (float y = 0; y < shineRect.h; y += 1.0f) {
            float alpha = (1.0f - y / shineRect.h) * (isHovered ? 60 : 30);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(alpha));
            SDL_RenderDrawLineF(renderer, shineRect.x, shineRect.y + y,
                              shineRect.x + shineRect.w, shineRect.y + y);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // 4. Border
        float mainHue = fmod(animTimer * 30.0f, 360.0f);
        SDL_Color borderColor = hsvToRgb(mainHue + 30.0f, 0.8f, 1.0f);
        borderColor.a = isHovered ? 255 : 180;
        drawRoundedRectBorder(rect, cornerRadius, borderColor, 2.0f);

        // 5. Text with shadow
        if (!text.empty() && font) {
            const float centerX = rect.x + rect.w / 2;
            const float centerY = rect.y + rect.h / 2;

            // Shadow
            renderTextCentered(text, centerX + 2, centerY + 2, font, {0, 0, 0, 150});

            // Main text
            SDL_Color textColor = isHovered ? SDL_Color{255, 255, 255, 255} : SDL_Color{240, 240, 240, 255};
            renderTextCentered(text, centerX, centerY, font, textColor);
        }
    }

    // === Enhanced Glass Panel ===
    void drawEnhancedGlassPanel(const SDL_FRect& rect, const SDL_Color& baseColor) {
        const float CORNER_RADIUS = 20.0f;

        // 1. Gradient background
        SDL_Color topColor = baseColor;
        SDL_Color bottomColor = {
            static_cast<Uint8>(baseColor.r * 0.6f),
            static_cast<Uint8>(baseColor.g * 0.6f),
            static_cast<Uint8>(baseColor.b * 0.6f),
            baseColor.a
        };

        float pulseAlpha = baseColor.a + 20 * sin(animTimer * 2.5f);
        topColor.a = static_cast<Uint8>(pulseAlpha);
        bottomColor.a = static_cast<Uint8>(pulseAlpha);

        for (int y = 0; y < static_cast<int>(rect.h); ++y) {
            float t = static_cast<float>(y) / rect.h;
            SDL_Color currentColor;
            currentColor.r = static_cast<Uint8>(topColor.r * (1.0f - t) + bottomColor.r * t);
            currentColor.g = static_cast<Uint8>(topColor.g * (1.0f - t) + bottomColor.g * t);
            currentColor.b = static_cast<Uint8>(topColor.b * (1.0f - t) + bottomColor.b * t);
            currentColor.a = static_cast<Uint8>(topColor.a * (1.0f - t) + bottomColor.a * t);

            SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
            SDL_RenderDrawLineF(renderer, rect.x, rect.y + y, rect.x + rect.w, rect.y + y);
        }

        // 2. Noise effect
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
        for (int i = 0; i < 100; ++i) {
            float randX = rect.x + (rand() % static_cast<int>(rect.w));
            float randY = rect.y + (rand() % static_cast<int>(rect.h));
            int alpha = 10 + (rand() % 15);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
            SDL_RenderDrawPointF(renderer, randX, randY);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // 3. Inner highlight
        SDL_Color highlightColor = {255, 255, 255, 60};
        drawRoundedRectBorder(rect, CORNER_RADIUS, highlightColor, 1.5f);

        // 4. Outer glow
        float hue = fmod(animTimer * 30.0f, 360.0f);
        SDL_Color glowColor = hsvToRgb(hue, 0.5f, 0.8f);
        glowColor.a = static_cast<Uint8>(40 + 20 * sin(animTimer * 4.0f));
        for (int i = 0; i < 3; ++i) {
            float offset = i * 2.0f;
            SDL_FRect glowRect = {
                rect.x - offset, rect.y - offset,
                rect.w + offset * 2, rect.h + offset * 2
            };
            drawRoundedRectBorder(glowRect, CORNER_RADIUS + offset, glowColor, 2.0f);
        }
    }

    // === Text Rendering Helpers ===
    void renderTextCentered(const std::string& text, float x, float y, TTF_Font* font, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface) return;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int w = surface->w, h = surface->h;
        SDL_FreeSurface(surface);
        if (!texture) return;
        SDL_Rect dst = { static_cast<int>(x - w/2), static_cast<int>(y - h/2), w, h };
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }

    void renderTextLeft(const std::string& text, float x, float y, TTF_Font* font, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface) return;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int w = surface->w, h = surface->h;
        SDL_FreeSurface(surface);
        if (!texture) return;
        SDL_Rect dst = { static_cast<int>(x), static_cast<int>(y), w, h };
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }
};

#endif // UI_RENDERER_H_INCLUDED

#ifndef SHOP_H_INCLUDED
#define SHOP_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include "player.h"
#include "ui_renderer.h"

struct ShopItem {
    int id;
    std::string name;
    std::string description;
    int price;
    bool isOwned;
    SDL_Texture* texture;
    SDL_Texture* previewTexture;

    ShopItem(int itemId, const std::string& itemName, const std::string& desc, int itemPrice)
        : id(itemId), name(itemName), description(desc), price(itemPrice),
          isOwned(false), texture(nullptr), previewTexture(nullptr) {}
};

class Shop {
public:
    std::vector<ShopItem> items;
    int selectedIndex;
    int scrollOffset;

    Shop() : selectedIndex(0), scrollOffset(0) {}

    void initialize(SDL_Renderer* renderer) {
        items.clear();
        items.emplace_back(0, "Classic Dino", "The original dino", 0);
        items.emplace_back(1, "Red Dragon", "Fierce red dragon", 100);
        items.emplace_back(2, "Blue Raptor", "Fast blue raptor", 150);
        items.emplace_back(3, "Golden Rex", "Legendary golden T-Rex", 300);
        items.emplace_back(4, "Purple Ghost", "Mysterious ghost dino", 200);
        items.emplace_back(5, "Green Turtle", "Slow but steady", 80);
        items.emplace_back(6, "Rainbow Dino", "Colorful party dino", 500);
        items[0].isOwned = true;
        loadTextures(renderer);
    }

    void loadTextures(SDL_Renderer* renderer) {
        const char* skinFiles[] = { "image/dino.png", "image/dino_red.png", "image/dino_blue.png", "image/dino_gold.png", "image/dino_purple.png", "image/dino_green.png", "image/dino_rainbow.png"};
        SDL_Color skinColors[] = { {100,200,100,255}, {255,50,50,255}, {50,100,255,255}, {255,215,0,255}, {150,50,200,255}, {34,139,34,255}, {255,100,200,255} };

        for (size_t i = 0; i < items.size(); i++) {
            items[i].texture = IMG_LoadTexture(renderer, skinFiles[i]);
            if (!items[i].texture) {
                std::cerr << "Warning: Could not load " << skinFiles[i] << ". Creating fallback texture." << std::endl;
                items[i].texture = createColoredTexture(renderer, skinColors[i], 40, 60);
            }
            items[i].previewTexture = items[i].texture;
        }
    }

    SDL_Texture* createColoredTexture(SDL_Renderer* renderer, SDL_Color color, int w, int h) {
        SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        if (tex) {
            SDL_SetRenderTarget(renderer, tex);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderClear(renderer);
            SDL_SetRenderTarget(renderer, nullptr);
        }
        return tex;
    }

    // Helper function moved to main.cpp, but can be kept here if preferred
    SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface) return nullptr;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return texture;
    }

    void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int y, int screenW) {
        SDL_Texture* tex = renderText(renderer, font, text, color);
        if (tex) { int w, h; SDL_QueryTexture(tex, NULL, NULL, &w, &h); SDL_Rect dst = { (screenW - w) / 2, y, w, h }; SDL_RenderCopy(renderer, tex, NULL, &dst); SDL_DestroyTexture(tex); }
    }

    void renderLeftText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
        SDL_Texture* tex = renderText(renderer, font, text, color);
        if (tex) { int w, h; SDL_QueryTexture(tex, NULL, NULL, &w, &h); SDL_Rect dst = { x, y, w, h }; SDL_RenderCopy(renderer, tex, NULL, &dst); SDL_DestroyTexture(tex); }
    }

    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall, TTF_Font* fontTiny,
            const Player& player, UIRenderer& uiRenderer) {
        SDL_Color white={255,255,255,255}, black={0,0,0,255}, orange={255,140,0,255};

        // Title với glass panel
        SDL_FRect titlePanel = {200, 10, 400, 80};
        uiRenderer.drawEnhancedGlassPanel(titlePanel, {50, 100, 200, 180});
        uiRenderer.renderTextCentered("DINO SHOP", 400, 50, fontBig, white);

        // Player info panel
        SDL_FRect infoPanel = {20, 100, 760, 60};
        uiRenderer.drawEnhancedGlassPanel(infoPanel, {100, 50, 150, 160});

        uiRenderer.renderTextLeft("Coins: " + std::to_string(player.totalCoins), 40, 115, fontSmall, orange);
        uiRenderer.renderTextLeft("Level " + std::to_string(player.level) + " (XP: " +
                                 std::to_string(player.xp) + "/" + std::to_string(player.xpToNextLevel) + ")",
                                 40, 145, fontTiny, white);

        int mx, my;
        SDL_GetMouseState(&mx, &my);
        int startY = 180, itemsPerRow = 3, itemWidth = 220, itemHeight = 180, spacing = 25, startX = 50;

        for (size_t i = 0; i < items.size(); i++) {
            int col = i % itemsPerRow, row = i / itemsPerRow;
            float x = startX + col * (itemWidth + spacing), y = startY + row * (itemHeight + spacing);
            SDL_FRect itemRect = {x, y, (float)itemWidth, (float)itemHeight};

            bool hovered = (mx >= itemRect.x && mx <= itemRect.x + itemRect.w &&
                           my >= itemRect.y && my <= itemRect.y + itemRect.h);

            // Item background with glow
            SDL_Color bgColor;
            if (player.equippedSkinIndex == (int)i) {
                bgColor = {255, 215, 0, 200}; // Gold for equipped
            } else if (items[i].isOwned) {
                bgColor = {80, 200, 80, 180}; // Green for owned
            } else {
                bgColor = {120, 120, 120, 180}; // Gray for locked
            }

            // Enhanced panel with glow
            uiRenderer.drawEnhancedGlassPanel(itemRect, bgColor);

            // Preview texture
            if (items[i].previewTexture) {
                SDL_Rect pv = {(int)(x + itemWidth/2 - 30), (int)(y + 20), 60, 90};
                SDL_RenderCopy(renderer, items[i].previewTexture, nullptr, &pv);
            }

            // Item name
            uiRenderer.renderTextCentered(items[i].name, x + itemWidth/2, y + 125, fontTiny, white);

            // Status/Price button
            SDL_FRect statusBtn = {x + 10, y + itemHeight - 40, itemWidth - 20, 30};
            SDL_Color btnColor;
            std::string btnText;

            if (items[i].isOwned) {
                if (player.equippedSkinIndex == (int)i) {
                    btnColor = {255, 215, 0, 255};
                    btnText = "EQUIPPED";
                } else {
                    btnColor = {100, 200, 100, 255};
                    btnText = "EQUIP";
                }
            } else {
                if (player.totalCoins >= items[i].price) {
                    btnColor = {50, 150, 255, 255};
                } else {
                    btnColor = {150, 50, 50, 255};
                }
                btnText = "BUY: " + std::to_string(items[i].price);
            }

            uiRenderer.renderEnhancedButton(statusBtn, hovered, btnText, fontTiny, btnColor);
        }

        // Back button
        SDL_FRect backBtn = {300, 100, 200, 50};
        bool hovBack = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                        my >= backBtn.y && my <= backBtn.y + backBtn.h);
        uiRenderer.renderEnhancedButton(backBtn, hovBack, "BACK TO MENU", fontSmall, {200, 50, 50, 255});
    }

    bool handleInput(SDL_Event& e, Player& player, int& equippedSkinIndex, std::function<void()> saveCallback) {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            SDL_Rect backBtn = {300, 100, 200, 50};
            if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w && my >= backBtn.y && my <= backBtn.y + backBtn.h) return true;

            int startY = 150, itemsPerRow = 3, itemWidth = 220, itemHeight = 140, spacing = 20, startX = 50;
            for (size_t i = 0; i < items.size(); i++) {
                int col = i % itemsPerRow, row = i / itemsPerRow;
                int x = startX + col * (itemWidth + spacing), y = startY + row * (itemHeight + spacing);
                SDL_Rect itemRect = {x, y, itemWidth, itemHeight};
                if (mx >= itemRect.x && mx <= itemRect.x + itemRect.w && my >= itemRect.y && my <= itemRect.y + itemRect.h) {
                    if (items[i].isOwned) {
                        player.equippedSkinIndex = i;
                        equippedSkinIndex = i;
                    } else if (player.totalCoins >= items[i].price) {
                        player.totalCoins -= items[i].price;
                        items[i].isOwned = true;
                        player.equippedSkinIndex = i;
                        equippedSkinIndex = i;
                    }
                    saveCallback();
                    break;
                }
            }
        }
        return false;
    }

    void cleanup() {
        for (auto& item : items) {
            if (item.texture) { SDL_DestroyTexture(item.texture); item.texture = nullptr; }
            item.previewTexture = nullptr;
        }
        items.clear();
    }
};

#endif // SHOP_H_INCLUDED

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

    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall, TTF_Font* fontTiny, const Player& player) {
        SDL_Color white={255,255,255,255}, black={0,0,0,255}, green={0,200,0,255}, gray={128,128,128,255}, orange={255,140,0,255};
        renderCenteredText(renderer, fontBig, "DINO SHOP", black, 20, 800);
        renderLeftText(renderer, fontSmall, "Your Coins: " + std::to_string(player.totalCoins), orange, 20, 80);
        renderLeftText(renderer, fontTiny, "Level " + std::to_string(player.level) + " (XP: " + std::to_string(player.xp) + "/" + std::to_string(player.xpToNextLevel) + ")", black, 20, 115);

        int mx, my; SDL_GetMouseState(&mx, &my);
        int startY = 150, itemsPerRow = 3, itemWidth = 220, itemHeight = 140, spacing = 20, startX = 50;

        for (size_t i = 0; i < items.size(); i++) {
            int col = i % itemsPerRow, row = i / itemsPerRow;
            int x = startX + col * (itemWidth + spacing), y = startY + row * (itemHeight + spacing);
            SDL_Rect itemRect = {x, y, itemWidth, itemHeight};
            bool hovered = (mx >= itemRect.x && mx <= itemRect.x + itemRect.w && my >= itemRect.y && my <= itemRect.y + itemRect.h);

            if (player.equippedSkinIndex == (int)i) SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
            else if (items[i].isOwned) SDL_SetRenderDrawColor(renderer, hovered?100:80, hovered?220:200, 80, 255);
            else SDL_SetRenderDrawColor(renderer, hovered?150:120, hovered?150:120, hovered?150:120, 255);
            SDL_RenderFillRect(renderer, &itemRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderDrawRect(renderer, &itemRect);

            if (items[i].previewTexture) { SDL_Rect pv = {x + itemWidth/2 - 20, y + 10, 40, 60}; SDL_RenderCopy(renderer, items[i].previewTexture, nullptr, &pv); }
            renderLeftText(renderer, fontTiny, items[i].name, black, x + 10, y + 75);

            if (items[i].isOwned) {
                renderLeftText(renderer, fontTiny, player.equippedSkinIndex == (int)i ? "EQUIPPED" : "OWNED", green, x + 10, y + 95);
            } else {
                renderLeftText(renderer, fontTiny, "Price: " + std::to_string(items[i].price), (player.totalCoins >= items[i].price)?green:gray, x + 10, y + 95);
            }
        }
        SDL_Rect backBtn = {300, 420, 200, 50};
        bool hovBack = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w && my >= backBtn.y && my <= backBtn.y + backBtn.h);
        SDL_SetRenderDrawColor(renderer, hovBack ? 180 : 150, 50, 50, 255); SDL_RenderFillRect(renderer, &backBtn);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderDrawRect(renderer, &backBtn);
        renderCenteredText(renderer, fontSmall, "BACK TO MENU", white, backBtn.y + 12, 800);
    }

    bool handleInput(SDL_Event& e, Player& player, int& equippedSkinIndex, std::function<void()> saveCallback) {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            SDL_Rect backBtn = {300, 420, 200, 50};
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

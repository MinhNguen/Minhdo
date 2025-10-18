#ifndef SHOP_H_INCLUDED
#define SHOP_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <string>
#include <functional>
#include "player.h"

// Cấu trúc cho mỗi item trong shop
struct ShopItem {
    int id;
    std::string name;
    std::string description;
    int price;
    bool isOwned;
    SDL_Texture* texture;
    SDL_Texture* previewTexture; // Texture để hiển thị trong shop

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

    // Khởi tạo shop với các skin
    void initialize(SDL_Renderer* renderer) {
        items.clear();

        // Skin mặc định (miễn phí)
        items.push_back(ShopItem(0, "Classic Dino", "The original dino", 0));

        // Các skin có thể mua
        items.push_back(ShopItem(1, "Red Dragon", "Fierce red dragon", 100));
        items.push_back(ShopItem(2, "Blue Raptor", "Fast blue raptor", 150));
        items.push_back(ShopItem(3, "Golden Rex", "Legendary golden T-Rex", 300));
        items.push_back(ShopItem(4, "Purple Ghost", "Mysterious ghost dino", 200));
        items.push_back(ShopItem(5, "Green Turtle", "Slow but steady", 80));
        items.push_back(ShopItem(6, "Rainbow Dino", "Colorful party dino", 500));

        // Skin mặc định luôn được sở hữu
        items[0].isOwned = true;

        // Load textures cho các skin
        loadTextures(renderer);
    }

    // Load textures cho các skin
    void loadTextures(SDL_Renderer* renderer) {
        // Thử load từ file, nếu không có thì tạo texture màu sắc
        const char* skinFiles[] = {
            "image/dino.png",
            "image/dino_red.png",
            "image/dino_blue.png",
            "image/dino_gold.png",
            "image/dino_purple.png",
            "image/dino_green.png",
            "image/dino_rainbow.png"
        };

        SDL_Color skinColors[] = {
            {100, 200, 100, 255},  // Classic - xanh lá
            {255, 50, 50, 255},    // Red
            {50, 100, 255, 255},   // Blue
            {255, 215, 0, 255},    // Gold
            {150, 50, 200, 255},   // Purple
            {34, 139, 34, 255},    // Green
            {255, 100, 200, 255}   // Rainbow
        };

        for (size_t i = 0; i < items.size() && i < 7; i++) {
            // Thử load từ file
            items[i].texture = IMG_LoadTexture(renderer, skinFiles[i]);

            // Nếu không load được, tạo texture màu đơn giản
            if (!items[i].texture) {
                items[i].texture = createColoredTexture(renderer, skinColors[i], 40, 60);
            }

            // Tạo preview texture (có thể dùng chung với texture chính)
            items[i].previewTexture = items[i].texture;
        }
    }

    // Tạo texture màu đơn giản
    SDL_Texture* createColoredTexture(SDL_Renderer* renderer, SDL_Color color, int w, int h) {
        SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_TARGET, w, h);
        if (tex) {
            SDL_SetRenderTarget(renderer, tex);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderClear(renderer);

            // Vẽ hình dáng đơn giản của dino
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect body = {5, 10, 30, 35};
            SDL_RenderDrawRect(renderer, &body);
            SDL_Rect head = {10, 5, 20, 15};
            SDL_RenderDrawRect(renderer, &head);
            SDL_Rect leg1 = {10, 40, 8, 15};
            SDL_RenderDrawRect(renderer, &leg1);
            SDL_Rect leg2 = {22, 40, 8, 15};
            SDL_RenderDrawRect(renderer, &leg2);

            SDL_SetRenderTarget(renderer, nullptr);
        }
        return tex;
    }

    // Render helper function
    SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface) return nullptr;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return texture;
    }

    void renderCenteredText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                            SDL_Color color, int y, int screenW) {
        SDL_Texture* tex = renderText(renderer, font, text, color);
        if (tex) {
            int w, h;
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
            SDL_Rect dst = { (screenW - w) / 2, y, w, h };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
    }

    void renderLeftText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                        SDL_Color color, int x, int y) {
        SDL_Texture* tex = renderText(renderer, font, text, color);
        if (tex) {
            int w, h;
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
            SDL_Rect dst = { x, y, w, h };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
    }

    // Render shop
    void render(SDL_Renderer* renderer, TTF_Font* fontBig, TTF_Font* fontSmall,
                TTF_Font* fontTiny, const Player& player) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color black = {0, 0, 0, 255};
        SDL_Color green = {0, 200, 0, 255};
        SDL_Color gray = {128, 128, 128, 255};
        SDL_Color yellow = {255, 215, 0, 255};
        SDL_Color orange = {255, 140, 0, 255};

        // Title
        renderCenteredText(renderer, fontBig, "DINO SHOP", black, 20, 800);

        // Hiển thị số coins của player
        std::string coinText = "Your Coins: " + std::to_string(player.totalCoins);
        renderLeftText(renderer, fontSmall, coinText, orange, 20, 80);

        // Hiển thị level và XP
        std::string levelText = "Level " + std::to_string(player.level) +
                               " (XP: " + std::to_string(player.xp) + "/" +
                               std::to_string(player.xpToNextLevel) + ")";
        renderLeftText(renderer, fontTiny, levelText, black, 20, 115);

        int mx, my;
        SDL_GetMouseState(&mx, &my);

        // Vẽ grid các items
        int startY = 150;
        int itemsPerRow = 3;
        int itemWidth = 220;
        int itemHeight = 140;
        int spacing = 20;
        int startX = 50;

        for (size_t i = 0; i < items.size(); i++) {
            int col = i % itemsPerRow;
            int row = i / itemsPerRow;

            int x = startX + col * (itemWidth + spacing);
            int y = startY + row * (itemHeight + spacing);

            SDL_Rect itemRect = {x, y, itemWidth, itemHeight};

            // Kiểm tra hover
            bool hovered = (mx >= itemRect.x && mx <= itemRect.x + itemRect.w &&
                          my >= itemRect.y && my <= itemRect.y + itemRect.h);

            // Màu nền
            if (items[i].isOwned) {
                if (player.equippedSkinIndex == items[i].id) {
                    // Đang sử dụng - viền vàng
                    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                } else {
                    // Đã sở hữu - xanh lá
                    SDL_SetRenderDrawColor(renderer, hovered ? 100 : 80,
                                         hovered ? 220 : 200, 80, 255);
                }
            } else {
                // Chưa sở hữu - xám
                SDL_SetRenderDrawColor(renderer, hovered ? 150 : 120,
                                     hovered ? 150 : 120,
                                     hovered ? 150 : 120, 255);
            }

            SDL_RenderFillRect(renderer, &itemRect);

            // Viền
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &itemRect);

            // Preview skin
            if (items[i].previewTexture) {
                SDL_Rect previewRect = {x + itemWidth/2 - 20, y + 10, 40, 60};
                SDL_RenderCopy(renderer, items[i].previewTexture, nullptr, &previewRect);
            }

            // Tên item
            renderLeftText(renderer, fontTiny, items[i].name, black, x + 10, y + 75);

            // Giá hoặc trạng thái
            if (items[i].isOwned) {
                if (player.equippedSkinIndex == items[i].id) {
                    renderLeftText(renderer, fontTiny, "EQUIPPED", green, x + 10, y + 95);
                } else {
                    renderLeftText(renderer, fontTiny, "OWNED - Click to Equip", green, x + 10, y + 95);
                }
            } else {
                std::string priceText = "Price: " + std::to_string(items[i].price) + " coins";
                SDL_Color priceColor = (player.totalCoins >= items[i].price) ? green : gray;
                renderLeftText(renderer, fontTiny, priceText, priceColor, x + 10, y + 95);

                if (player.totalCoins >= items[i].price) {
                    renderLeftText(renderer, fontTiny, "Click to Buy", black, x + 10, y + 115);
                } else {
                    renderLeftText(renderer, fontTiny, "Not enough coins", gray, x + 10, y + 115);
                }
            }
        }

        // Nút Back
        SDL_Rect backBtn = {300, 420, 200, 50};
        bool hovBack = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                       my >= backBtn.y && my <= backBtn.y + backBtn.h);

        SDL_SetRenderDrawColor(renderer, hovBack ? 180 : 150, 50, 50, 255);
        SDL_RenderFillRect(renderer, &backBtn);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &backBtn);
        renderCenteredText(renderer, fontSmall, "BACK TO MENU", white, backBtn.y + 12, 800);
    }

    // Xử lý input
    bool handleInput(SDL_Event& e, Player& player, int& equippedSkinIndex,
                     std::function<void()> saveCallback) {
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x;
            int my = e.button.y;

            // Kiểm tra nút Back
            SDL_Rect backBtn = {300, 420, 200, 50};
            if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                my >= backBtn.y && my <= backBtn.y + backBtn.h) {
                return true; // Quay về menu
            }

            // Kiểm tra click vào items
            int startY = 150;
            int itemsPerRow = 3;
            int itemWidth = 220;
            int itemHeight = 140;
            int spacing = 20;
            int startX = 50;

            for (size_t i = 0; i < items.size(); i++) {
                int col = i % itemsPerRow;
                int row = i / itemsPerRow;

                int x = startX + col * (itemWidth + spacing);
                int y = startY + row * (itemHeight + spacing);

                SDL_Rect itemRect = {x, y, itemWidth, itemHeight};

                if (mx >= itemRect.x && mx <= itemRect.x + itemRect.w &&
                    my >= itemRect.y && my <= itemRect.y + itemRect.h) {

                    if (items[i].isOwned) {
                        // Trang bị skin
                        equippedSkinIndex = items[i].id;
                        player.equippedSkinIndex = items[i].id;
                        if (saveCallback) saveCallback();
                    } else {
                        // Mua skin
                        if (player.totalCoins >= items[i].price) {
                            player.totalCoins -= items[i].price;
                            items[i].isOwned = true;
                            equippedSkinIndex = items[i].id;
                            player.equippedSkinIndex = items[i].id;
                            if (saveCallback) saveCallback();
                        }
                    }
                    break;
                }
            }
        }

        return false;
    }

    // Cleanup
    void cleanup() {
        for (auto& item : items) {
            if (item.texture) {
                SDL_DestroyTexture(item.texture);
                item.texture = nullptr;
            }
            // Không destroy previewTexture vì nó cùng con trỏ với texture
            item.previewTexture = nullptr;
        }
        items.clear();
    }
};

#endif // SHOP_H_INCLUDED

#pragma once
#include <SDL.h>
#include <cmath>
#include <unordered_map>

namespace th01_native {

struct TouchInputState {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool shot = false;
    bool strike = false;
    bool bomb = false;
    bool pause = false;
};

class TouchControls {
public:
    // Logical coordinate space is 640x400 (matching SDL_RenderSetLogicalSize).
    struct Rect {
        float x, y, w, h;
        bool contains(float px, float py) const {
            return px >= x && px <= x + w && py >= y && py <= y + h;
        }
    };

    void handle_event(const SDL_Event& event) {
        if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION) {
            fingers_[event.tfinger.fingerId] = {
                event.tfinger.x * 640.0f,
                event.tfinger.y * 400.0f
            };
        } else if (event.type == SDL_FINGERUP) {
            fingers_.erase(event.tfinger.fingerId);
        }
    }

    TouchInputState state() const {
        TouchInputState s;
        for (const auto& [id, pos] : fingers_) {
            float x = pos.first;
            float y = pos.second;

            // Virtual D-Pad area (bottom-left)
            if (dpad_area_.contains(x, y)) {
                float dx = x - dpad_cx_;
                float dy = y - dpad_cy_;
                const float deadzone = 14.0f;
                if (dx < -deadzone) s.left = true;
                if (dx > deadzone) s.right = true;
                if (dy < -deadzone) s.up = true;
                if (dy > deadzone) s.down = true;
            }
            // Action buttons (bottom-right)
            if (btn_shot_.contains(x, y)) s.shot = true;
            if (btn_strike_.contains(x, y)) s.strike = true;
            if (btn_bomb_.contains(x, y)) {
                s.bomb = true;
                s.shot = true;
                s.strike = true;
            }
            // Pause / Start (top-right)
            if (btn_pause_.contains(x, y)) s.pause = true;
        }
        return s;
    }

    void render(SDL_Renderer* renderer) const {
        const auto s = state();
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // D-Pad arms with directional arrows
        Rect r_up{dpad_cx_ - 23, dpad_cy_ - 69, 46, 46};
        Rect r_down{dpad_cx_ - 23, dpad_cy_ + 23, 46, 46};
        Rect r_left{dpad_cx_ - 69, dpad_cy_ - 23, 46, 46};
        Rect r_right{dpad_cx_ + 23, dpad_cy_ - 23, 46, 46};

        draw_button(renderer, r_up, s.up, 255, 255, 255);
        draw_arrow(renderer, r_up, 0, -1, s.up);
        draw_button(renderer, r_down, s.down, 255, 255, 255);
        draw_arrow(renderer, r_down, 0, 1, s.down);
        draw_button(renderer, r_left, s.left, 255, 255, 255);
        draw_arrow(renderer, r_left, -1, 0, s.left);
        draw_button(renderer, r_right, s.right, 255, 255, 255);
        draw_arrow(renderer, r_right, 1, 0, s.right);

        // Action buttons: Z (Shot - red), X (Strike - blue), B (Bomb - amber), || (Pause - gray)
        draw_button(renderer, btn_shot_, s.shot && !s.bomb, 255, 90, 110);
        draw_glyph_Z(renderer, btn_shot_, s.shot && !s.bomb);

        draw_button(renderer, btn_strike_, s.strike && !s.bomb, 90, 180, 255);
        draw_glyph_X(renderer, btn_strike_, s.strike && !s.bomb);

        draw_button(renderer, btn_bomb_, s.bomb, 255, 200, 60);
        draw_glyph_B(renderer, btn_bomb_, s.bomb);

        draw_button(renderer, btn_pause_, s.pause, 200, 200, 200);
        draw_glyph_pause(renderer, btn_pause_, s.pause);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

private:
    static void draw_button(SDL_Renderer* renderer, Rect r, bool active, Uint8 cr, Uint8 cg, Uint8 cb) {
        SDL_Rect sr{
            static_cast<int>(r.x),
            static_cast<int>(r.y),
            static_cast<int>(r.w),
            static_cast<int>(r.h)
        };
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, active ? 140 : 60);
        SDL_RenderFillRect(renderer, &sr);
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, active ? 235 : 140);
        SDL_RenderDrawRect(renderer, &sr);
    }

    static void draw_arrow(SDL_Renderer* renderer, Rect r, int dx, int dy, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 245 : 170);
        if (dy < 0) {
            SDL_RenderDrawLine(renderer, cx, cy - 6, cx - 6, cy + 4);
            SDL_RenderDrawLine(renderer, cx, cy - 6, cx + 6, cy + 4);
        } else if (dy > 0) {
            SDL_RenderDrawLine(renderer, cx, cy + 6, cx - 6, cy - 4);
            SDL_RenderDrawLine(renderer, cx, cy + 6, cx + 6, cy - 4);
        } else if (dx < 0) {
            SDL_RenderDrawLine(renderer, cx - 6, cy, cx + 4, cy - 6);
            SDL_RenderDrawLine(renderer, cx - 6, cy, cx + 4, cy + 6);
        } else if (dx > 0) {
            SDL_RenderDrawLine(renderer, cx + 6, cy, cx - 4, cy - 6);
            SDL_RenderDrawLine(renderer, cx + 6, cy, cx - 4, cy + 6);
        }
    }

    static void draw_glyph_Z(SDL_Renderer* renderer, Rect r, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 255 : 185);
        SDL_RenderDrawLine(renderer, cx - 7, cy - 8, cx + 7, cy - 8);
        SDL_RenderDrawLine(renderer, cx + 7, cy - 8, cx - 7, cy + 8);
        SDL_RenderDrawLine(renderer, cx - 7, cy + 8, cx + 7, cy + 8);
    }

    static void draw_glyph_X(SDL_Renderer* renderer, Rect r, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 255 : 185);
        SDL_RenderDrawLine(renderer, cx - 7, cy - 8, cx + 7, cy + 8);
        SDL_RenderDrawLine(renderer, cx + 7, cy - 8, cx - 7, cy + 8);
    }

    static void draw_glyph_B(SDL_Renderer* renderer, Rect r, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 255 : 185);
        SDL_RenderDrawLine(renderer, cx - 6, cy - 8, cx - 6, cy + 8);
        SDL_RenderDrawLine(renderer, cx - 6, cy - 8, cx + 4, cy - 8);
        SDL_RenderDrawLine(renderer, cx + 4, cy - 8, cx + 6, cy - 4);
        SDL_RenderDrawLine(renderer, cx + 6, cy - 4, cx + 4, cy);
        SDL_RenderDrawLine(renderer, cx - 6, cy, cx + 5, cy);
        SDL_RenderDrawLine(renderer, cx + 5, cy, cx + 7, cy + 4);
        SDL_RenderDrawLine(renderer, cx + 7, cy + 4, cx + 4, cy + 8);
        SDL_RenderDrawLine(renderer, cx - 6, cy + 8, cx + 4, cy + 8);
    }

    static void draw_glyph_pause(SDL_Renderer* renderer, Rect r, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 255 : 185);
        SDL_Rect bar1{cx - 6, cy - 6, 4, 12};
        SDL_Rect bar2{cx + 2, cy - 6, 4, 12};
        SDL_RenderFillRect(renderer, &bar1);
        SDL_RenderFillRect(renderer, &bar2);
    }

    std::unordered_map<SDL_FingerID, std::pair<float, float>> fingers_;

    const float dpad_cx_ = 88.0f;
    const float dpad_cy_ = 312.0f;
    const Rect dpad_area_{4.0f, 224.0f, 176.0f, 176.0f};

    const Rect btn_shot_{478.0f, 316.0f, 68.0f, 68.0f};
    const Rect btn_strike_{558.0f, 276.0f, 68.0f, 68.0f};
    const Rect btn_bomb_{480.0f, 244.0f, 64.0f, 58.0f};
    const Rect btn_pause_{564.0f, 12.0f, 62.0f, 38.0f};
};

} // namespace th01_native

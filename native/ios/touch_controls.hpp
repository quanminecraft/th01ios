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

        // D-Pad arms
        draw_button(renderer, {dpad_cx_ - 18, dpad_cy_ - 54, 36, 36}, s.up, 255, 255, 255);
        draw_button(renderer, {dpad_cx_ - 18, dpad_cy_ + 18, 36, 36}, s.down, 255, 255, 255);
        draw_button(renderer, {dpad_cx_ - 54, dpad_cy_ - 18, 36, 36}, s.left, 255, 255, 255);
        draw_button(renderer, {dpad_cx_ + 18, dpad_cy_ - 18, 36, 36}, s.right, 255, 255, 255);

        // Action buttons: Z (red/pink), X (blue/cyan), Bomb (amber), Pause (gray)
        draw_button(renderer, btn_shot_, s.shot && !s.bomb, 255, 90, 110);
        draw_button(renderer, btn_strike_, s.strike && !s.bomb, 90, 180, 255);
        draw_button(renderer, btn_bomb_, s.bomb, 255, 200, 60);
        draw_button(renderer, btn_pause_, s.pause, 200, 200, 200);

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
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, active ? 135 : 55);
        SDL_RenderFillRect(renderer, &sr);
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, active ? 220 : 120);
        SDL_RenderDrawRect(renderer, &sr);
    }

    std::unordered_map<SDL_FingerID, std::pair<float, float>> fingers_;

    const float dpad_cx_ = 74.0f;
    const float dpad_cy_ = 326.0f;
    const Rect dpad_area_{10.0f, 260.0f, 132.0f, 132.0f};

    const Rect btn_shot_{506.0f, 326.0f, 54.0f, 54.0f};
    const Rect btn_strike_{572.0f, 294.0f, 54.0f, 54.0f};
    const Rect btn_bomb_{506.0f, 260.0f, 54.0f, 50.0f};
    const Rect btn_pause_{576.0f, 14.0f, 50.0f, 28.0f};
};

} // namespace th01_native

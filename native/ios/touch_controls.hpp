#pragma once
#include <SDL.h>
#include <cmath>
#include <optional>
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

    void clear() {
        fingers_.clear();
        joy_finger_.reset();
    }

    void handle_event(const SDL_Event& event) {
        if (event.type == SDL_FINGERDOWN) {
            float x = event.tfinger.x * 640.0f;
            float y = event.tfinger.y * 400.0f;
            fingers_[event.tfinger.fingerId] = {x, y};
            if (!joy_finger_.has_value() && joy_zone_.contains(x, y)) {
                joy_finger_ = event.tfinger.fingerId;
            }
        } else if (event.type == SDL_FINGERMOTION) {
            fingers_[event.tfinger.fingerId] = {
                event.tfinger.x * 640.0f,
                event.tfinger.y * 400.0f
            };
        } else if (event.type == SDL_FINGERUP) {
            fingers_.erase(event.tfinger.fingerId);
            if (joy_finger_.has_value() && *joy_finger_ == event.tfinger.fingerId) {
                joy_finger_.reset();
            }
        }
    }

    TouchInputState state() const {
        TouchInputState s;
        if (joy_finger_.has_value()) {
            auto it = fingers_.find(*joy_finger_);
            if (it != fingers_.end()) {
                float dx = it->second.first - joy_cx_;
                float dy = it->second.second - joy_cy_;
                const float deadzone = 12.0f;
                if (dx < -deadzone) s.left = true;
                if (dx > deadzone) s.right = true;
                if (dy < -deadzone) s.up = true;
                if (dy > deadzone) s.down = true;
            }
        }

        for (const auto& [id, pos] : fingers_) {
            if (joy_finger_.has_value() && id == *joy_finger_) continue;
            float x = pos.first;
            float y = pos.second;
            if (btn_shot_.contains(x, y)) s.shot = true;
            if (btn_strike_.contains(x, y)) s.strike = true;
            if (btn_bomb_.contains(x, y)) {
                s.bomb = true;
                s.shot = true;
                s.strike = true;
            }
            if (btn_pause_.contains(x, y)) s.pause = true;
        }
        return s;
    }

    void render(SDL_Renderer* renderer) const {
        const auto s = state();
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // Virtual Joystick (bottom-left)
        bool joy_active = joy_finger_.has_value();
        float knob_x = joy_cx_;
        float knob_y = joy_cy_;
        if (joy_active) {
            auto it = fingers_.find(*joy_finger_);
            if (it != fingers_.end()) {
                float dx = it->second.first - joy_cx_;
                float dy = it->second.second - joy_cy_;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist > joy_radius_) {
                    dx = dx * (joy_radius_ / dist);
                    dy = dy * (joy_radius_ / dist);
                }
                knob_x = joy_cx_ + dx;
                knob_y = joy_cy_ + dy;
            }
        }

        // Joystick base circle + outline
        draw_filled_circle(renderer, static_cast<int>(joy_cx_), static_cast<int>(joy_cy_),
                           static_cast<int>(joy_radius_), 255, 255, 255, joy_active ? 55 : 35);
        draw_circle_outline(renderer, static_cast<int>(joy_cx_), static_cast<int>(joy_cy_),
                            static_cast<int>(joy_radius_), 255, 255, 255, joy_active ? 180 : 110);
        // Joystick knob
        draw_filled_circle(renderer, static_cast<int>(knob_x), static_cast<int>(knob_y),
                           static_cast<int>(joy_knob_radius_), 255, 255, 255, joy_active ? 160 : 95);
        draw_circle_outline(renderer, static_cast<int>(knob_x), static_cast<int>(knob_y),
                            static_cast<int>(joy_knob_radius_), 255, 255, 255, joy_active ? 245 : 165);

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
    static void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int r,
                                   Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca) {
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, ca);
        for (int dy = -r; dy <= r; ++dy) {
            int dx = static_cast<int>(std::sqrt(static_cast<float>(r * r - dy * dy)));
            SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
        }
    }

    static void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int r,
                                    Uint8 cr, Uint8 cg, Uint8 cb, Uint8 ca) {
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, ca);
        int x = r, y = 0, err = 1 - r;
        while (x >= y) {
            SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            SDL_RenderDrawPoint(renderer, cx + y, cy + x);
            SDL_RenderDrawPoint(renderer, cx - y, cy + x);
            SDL_RenderDrawPoint(renderer, cx - x, cy + y);
            SDL_RenderDrawPoint(renderer, cx - x, cy - y);
            SDL_RenderDrawPoint(renderer, cx - y, cy - x);
            SDL_RenderDrawPoint(renderer, cx + y, cy - x);
            SDL_RenderDrawPoint(renderer, cx + x, cy - y);
            ++y;
            if (err < 0) {
                err += 2 * y + 1;
            } else {
                --x;
                err += 2 * (y - x + 1);
            }
        }
    }

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

    static void draw_glyph_Z(SDL_Renderer* renderer, Rect r, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 255 : 185);
        SDL_RenderDrawLine(renderer, cx - 8, cy - 9, cx + 8, cy - 9);
        SDL_RenderDrawLine(renderer, cx + 8, cy - 9, cx - 8, cy + 9);
        SDL_RenderDrawLine(renderer, cx - 8, cy + 9, cx + 8, cy + 9);
    }

    static void draw_glyph_X(SDL_Renderer* renderer, Rect r, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 255 : 185);
        SDL_RenderDrawLine(renderer, cx - 8, cy - 9, cx + 8, cy + 9);
        SDL_RenderDrawLine(renderer, cx + 8, cy - 9, cx - 8, cy + 9);
    }

    static void draw_glyph_B(SDL_Renderer* renderer, Rect r, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 255 : 185);
        SDL_RenderDrawLine(renderer, cx - 7, cy - 9, cx - 7, cy + 9);
        SDL_RenderDrawLine(renderer, cx - 7, cy - 9, cx + 5, cy - 9);
        SDL_RenderDrawLine(renderer, cx + 5, cy - 9, cx + 7, cy - 5);
        SDL_RenderDrawLine(renderer, cx + 7, cy - 5, cx + 5, cy);
        SDL_RenderDrawLine(renderer, cx - 7, cy, cx + 6, cy);
        SDL_RenderDrawLine(renderer, cx + 6, cy, cx + 8, cy + 5);
        SDL_RenderDrawLine(renderer, cx + 8, cy + 5, cx + 5, cy + 9);
        SDL_RenderDrawLine(renderer, cx - 7, cy + 9, cx + 5, cy + 9);
    }

    static void draw_glyph_pause(SDL_Renderer* renderer, Rect r, bool active) {
        int cx = static_cast<int>(r.x + r.w * 0.5f);
        int cy = static_cast<int>(r.y + r.h * 0.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 255 : 185);
        SDL_Rect bar1{cx - 7, cy - 7, 5, 14};
        SDL_Rect bar2{cx + 2, cy - 7, 5, 14};
        SDL_RenderFillRect(renderer, &bar1);
        SDL_RenderFillRect(renderer, &bar2);
    }

    std::unordered_map<SDL_FingerID, std::pair<float, float>> fingers_;
    std::optional<SDL_FingerID> joy_finger_;

    const float joy_cx_ = 96.0f;
    const float joy_cy_ = 304.0f;
    const float joy_radius_ = 66.0f;
    const float joy_knob_radius_ = 28.0f;
    const Rect joy_zone_{0.0f, 180.0f, 240.0f, 220.0f};

    const Rect btn_shot_{466.0f, 310.0f, 76.0f, 76.0f};
    const Rect btn_strike_{552.0f, 268.0f, 76.0f, 76.0f};
    const Rect btn_bomb_{470.0f, 234.0f, 70.0f, 62.0f};
    const Rect btn_pause_{560.0f, 12.0f, 66.0f, 40.0f};
};

} // namespace th01_native

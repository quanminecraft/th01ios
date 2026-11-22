#pragma once
#include "framebuffer.hpp"
#include "assets.hpp"
#include <functional>
namespace th01_native {
struct KonngaraEntrance {
    Framebuffer& framebuffer;
    std::filesystem::path directory;
    // Required so the live session performs original boss-stage object reset.
    std::function<void()> reset_stage_objects;
    std::function<void(unsigned)> wait;
    std::function<void(const std::string&)> music_load;
    std::function<void()> music_play;
    Palette stage_palette{};
    std::array<SpriteSet,7> effects;
    std::array<SpriteSet,3> entities;
    std::vector<unsigned> sound_commands;
    bool pellet_interlace=false;
    unsigned selected_page=0;
    void run();
    std::function<void()> release_boss,music_stop,music_fade;
    std::uint32_t score=0;
    bool game_cleared=false;
    void run_defeat();
};
}

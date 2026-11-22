#pragma once
#include "framebuffer.hpp"
#include "assets.hpp"
#include <functional>
namespace th01_native {
struct SarielEntrance{
    Framebuffer& framebuffer;std::filesystem::path directory;
    std::function<void()> reset_stage_objects;
    std::function<void(unsigned)> wait,delay_ms;
    std::function<void(const std::string&)> music_load,se_load;
    std::function<void()> music_play;
    std::uint32_t frame_seed=1;bool pellet_interlace=false;unsigned selected_page=0;
    void run();
};
}
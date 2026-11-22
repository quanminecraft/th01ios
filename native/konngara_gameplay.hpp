#pragma once
#include "framebuffer.hpp"
#include <filesystem>
#include <functional>
#include <memory>
namespace th01_native {
struct KonngaraGameplayState {int hp,phase,frame,pattern,patterns_done;bool cleared;};
class KonngaraGameplay {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    KonngaraGameplay(Framebuffer& fb,const std::filesystem::path& assets,std::function<void(unsigned)> wait,std::function<void()> fade,std::function<void(const std::string&,const std::string&)> audio={});
    ~KonngaraGameplay();
    void load();void update();void show_palette();
    KonngaraGameplayState state() const;
    std::vector<unsigned> take_sound_commands();
};
}

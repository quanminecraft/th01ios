#pragma once
#include "assets.hpp"
#include "framebuffer.hpp"
#include <functional>
#include <memory>
namespace th01_native {
struct GameAudioCommand {std::string operation,asset;};
struct SarielGameplayState {int hp,phase,frame,pattern,patterns_done;bool cleared;};
class SarielGameplay {
 struct Impl;std::unique_ptr<Impl> impl;
public:
 SarielGameplay(Framebuffer&,const std::vector<Entry>&,const std::filesystem::path&,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay,std::function<void()> reset_stage,std::function<void()> hud,std::function<void(const std::string&,const std::string&)> audio);
 ~SarielGameplay();
 void load();void update();void show_palette();SarielGameplayState state()const;
};
}

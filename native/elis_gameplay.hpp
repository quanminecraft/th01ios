#pragma once
#include "assets.hpp"
#include "framebuffer.hpp"
#include <functional>
#include <memory>
namespace th01_native {
struct ElisGameplayState{int hp,phase,frame,pattern;bool bat,defeated;};
class ElisGameplay{struct Impl;std::unique_ptr<Impl> impl;public:
    ElisGameplay(Framebuffer&,const std::vector<Entry>&,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay,std::function<void()> fade,std::function<void()> defeat);
    ~ElisGameplay();ElisGameplay(const ElisGameplay&)=delete;ElisGameplay& operator=(const ElisGameplay&)=delete;
    void load();void update();void show_palette();ElisGameplayState state()const;
};
}
#pragma once
#include "assets.hpp"
#include "framebuffer.hpp"
#include <functional>
#include <memory>
namespace th01_native {
struct YuugenmaganGameplayState{int hp,phase,frame,subphase;bool defeated;};
class YuugenmaganGameplay{
    struct Impl;std::unique_ptr<Impl> impl;
public:
    YuugenmaganGameplay(Framebuffer&,const std::vector<Entry>&,std::function<void()> fade,std::function<void()> defeat);
    ~YuugenmaganGameplay();
    YuugenmaganGameplay(const YuugenmaganGameplay&)=delete;
    YuugenmaganGameplay& operator=(const YuugenmaganGameplay&)=delete;
    void load();void update();void show_palette();
    YuugenmaganGameplayState state()const;
};
}
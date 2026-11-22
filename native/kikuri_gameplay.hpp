#pragma once
#include "assets.hpp"
#include "framebuffer.hpp"
#include <functional>
#include <memory>
namespace th01_native {
struct KikuriGameplayState {int hp,phase,frame,subphase;bool defeated;};
class KikuriGameplay {
    struct Impl;
    std::unique_ptr<Impl> impl;
public:
    KikuriGameplay(Framebuffer& fb,const std::vector<Entry>& archive,std::function<void(unsigned)> wait,std::function<void()> fade,std::function<void()> defeat);
    ~KikuriGameplay();
    KikuriGameplay(const KikuriGameplay&)=delete;
    KikuriGameplay& operator=(const KikuriGameplay&)=delete;
    void load();void update();void show_palette();
    KikuriGameplayState state() const;
};
}
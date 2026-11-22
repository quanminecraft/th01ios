#pragma once
#include "assets.hpp"
#include "framebuffer.hpp"
#include <functional>
#include <memory>
namespace th01_native {
struct MimaGameplayState {int hp,phase,frame,pattern;bool defeated;};
// Keeps Mima/Missile bindings alive inside the session without exporting
// hardware compatibility macros into StageSession's other components.
class MimaGameplay {
    struct Impl;
    std::unique_ptr<Impl> impl;
public:
    MimaGameplay(Framebuffer& framebuffer,const std::vector<Entry>& archive,std::uint32_t& frame,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay_ms,std::function<void()> fade,std::function<void()> defeat);
    ~MimaGameplay();
    MimaGameplay(const MimaGameplay&)=delete;
    MimaGameplay& operator=(const MimaGameplay&)=delete;
    void load();
    void update();
    MimaGameplayState state() const;
};
}
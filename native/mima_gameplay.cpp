#include "mima_backend.hpp"
#include "mima_gameplay.hpp"
namespace th01_native {
struct MimaGameplay::Impl {
    MissileBackend missiles;
    MissileBinding missile_binding;
    MimaBackend backend;
    MimaBinding binding;
    bool loaded=false;
    Impl(Framebuffer& fb,const std::vector<Entry>& archive,std::uint32_t& frame,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay,std::function<void()> fade,std::function<void()> defeat):missiles{archive},missile_binding(missiles),backend{fb,missiles,frame,std::move(wait),std::move(delay),std::move(fade),std::move(defeat),{}},binding(backend) {}
    ~Impl() {if(loaded) mima_original::mima_free();}
};
MimaGameplay::MimaGameplay(Framebuffer& fb,const std::vector<Entry>& archive,std::uint32_t& frame,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay,std::function<void()> fade,std::function<void()> defeat):impl(std::make_unique<Impl>(fb,archive,frame,std::move(wait),std::move(delay),std::move(fade),std::move(defeat))) {}
MimaGameplay::~MimaGameplay()=default;
void MimaGameplay::load() {
    if(impl->loaded) {mima_original::mima_free();impl->loaded=false;}
    try {mima_original::mima_load();impl->loaded=true;}catch(...) {mima_original::mima_free();throw;}
}
void MimaGameplay::update() {if(!impl->loaded) throw std::logic_error("Mima gameplay is not loaded");mima_original::mima_main();}
MimaGameplayState MimaGameplay::state() const {return {mima_original::boss_hp,mima_original::boss_phase,mima_original::boss_phase_frame,impl->backend.pattern,impl->backend.defeated};}
}
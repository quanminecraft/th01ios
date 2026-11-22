#include "elis_backend.hpp"
#include "elis_gameplay.hpp"
namespace th01_native {
struct ElisGameplay::Impl{
    MissileBackend missiles;MissileBinding missile_binding;ElisBackend backend;ElisBinding binding;bool loaded=false;
    Impl(Framebuffer& fb,const std::vector<Entry>& archive,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay,std::function<void()> fade,std::function<void()> defeat):missiles{archive},missile_binding(missiles),backend{fb,archive,missiles,std::move(wait),std::move(delay),std::move(fade),std::move(defeat)},binding(backend){}
    ~Impl(){if(loaded)elis_original::elis_free();}
};
ElisGameplay::ElisGameplay(Framebuffer& fb,const std::vector<Entry>& archive,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay,std::function<void()> fade,std::function<void()> defeat):impl(std::make_unique<Impl>(fb,archive,std::move(wait),std::move(delay),std::move(fade),std::move(defeat))){}
ElisGameplay::~ElisGameplay()=default;
void ElisGameplay::load(){if(impl->loaded){elis_original::elis_free();impl->loaded=false;}elis_original::elis_load();impl->loaded=true;elis_original::boss_palette_snap();}
void ElisGameplay::update(){if(!impl->loaded)throw std::logic_error("Elis gameplay not loaded");elis_original::elis_main();}
void ElisGameplay::show_palette(){elis_original::boss_palette_show();}
ElisGameplayState ElisGameplay::state()const{auto& b=impl->backend;return{elis_original::boss_hp,elis_original::boss_phase,elis_original::boss_phase_frame,b.pattern,b.bat,b.defeated};}
}
#include "yuugenmagan_backend.hpp"
#include "yuugenmagan_gameplay.hpp"
namespace th01_native {
struct YuugenmaganGameplay::Impl{
    MissileBackend missiles;MissileBinding missile_binding;
    YuugenmaganBackend backend;YuugenmaganBinding binding;bool loaded=false;
    Impl(Framebuffer& fb,const std::vector<Entry>& archive,std::function<void()> fade,std::function<void()> defeat):missiles{archive},missile_binding(missiles),backend{fb,missiles,std::move(fade),std::move(defeat)},binding(backend){}
    ~Impl(){if(loaded)yuugenmagan_original::yuugenmagan_free();}
};
YuugenmaganGameplay::YuugenmaganGameplay(Framebuffer& fb,const std::vector<Entry>& archive,std::function<void()> fade,std::function<void()> defeat):impl(std::make_unique<Impl>(fb,archive,std::move(fade),std::move(defeat))){}
YuugenmaganGameplay::~YuugenmaganGameplay()=default;
void YuugenmaganGameplay::load(){
    if(impl->loaded){yuugenmagan_original::yuugenmagan_free();impl->loaded=false;}
    yuugenmagan_original::yuugenmagan_load();impl->loaded=true;yuugenmagan_original::boss_palette_snap();
}
void YuugenmaganGameplay::update(){if(!impl->loaded)throw std::logic_error("YuugenMagan gameplay is not loaded");yuugenmagan_original::yuugenmagan_main();}
void YuugenmaganGameplay::show_palette(){yuugenmagan_original::boss_palette_show();}
YuugenmaganGameplayState YuugenmaganGameplay::state()const{return{yuugenmagan_original::boss_hp,yuugenmagan_original::boss_phase,yuugenmagan_original::boss_phase_frame,impl->backend.subphase,impl->backend.defeated};}
}
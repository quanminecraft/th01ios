#include "kikuri_backend.hpp"
#include "kikuri_gameplay.hpp"
namespace th01_native {
struct KikuriGameplay::Impl {
    SpriteSet sprites;
    KikuriRippleBackend ripple;
    KikuriRippleBinding ripple_binding;
    KikuriBackend backend;
    KikuriBinding binding;
    bool loaded=false;
    Impl(Framebuffer& fb,const std::vector<Entry>& archive,std::function<void(unsigned)> wait,std::function<void()> fade,std::function<void()> defeat):sprites(read_sprites(asset(archive,"TAMAYEN.PTN"))),ripple{fb,sprites},ripple_binding(ripple),backend{fb,archive,sprites,std::move(wait),std::move(fade),std::move(defeat)},binding(backend) {}
    ~Impl() {if(loaded) kikuri_original::kikuri_free();}
};
KikuriGameplay::KikuriGameplay(Framebuffer& fb,const std::vector<Entry>& archive,std::function<void(unsigned)> wait,std::function<void()> fade,std::function<void()> defeat):impl(std::make_unique<Impl>(fb,archive,std::move(wait),std::move(fade),std::move(defeat))) {}
KikuriGameplay::~KikuriGameplay()=default;
void KikuriGameplay::load() {
    if(impl->loaded) {kikuri_original::kikuri_free();impl->loaded=false;}
    try {kikuri_original::kikuri_load();impl->loaded=true;kikuri_original::boss_palette_snap();}catch(...) {kikuri_original::kikuri_free();throw;}
}
void KikuriGameplay::update() {if(!impl->loaded) throw std::logic_error("Kikuri gameplay is not loaded");kikuri_original::kikuri_main();}
void KikuriGameplay::show_palette() {kikuri_original::boss_palette_show();}
KikuriGameplayState KikuriGameplay::state() const {return {kikuri_original::boss_hp,kikuri_original::kikuri_phase,kikuri_original::boss_phase_frame,impl->backend.subphase,impl->backend.defeated};}
}
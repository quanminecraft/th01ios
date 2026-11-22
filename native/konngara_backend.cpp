#include "konngara_backend.hpp"
namespace {
th01_native::KonngaraBackend* active=nullptr;
th01_native::KonngaraBackend& backend() {if(!active)throw std::logic_error("Konngara backend is not bound");return *active;}
Palette4 unpack(const th01_native::Palette& colors) {Palette4 result;for(unsigned i=0;i<16;++i) {auto v=colors[i];result[i].set(((v>>16)&255)/17,((v>>8)&255)/17,(v&255)/17);}return result;}
}
namespace th01_native {
KonngaraBinding::KonngaraBinding(KonngaraBackend& value):previous(active) {
    auto& g=value.graphics;
    if(&g.framebuffer!=&graphics_framebuffer())throw std::invalid_argument("Konngara framebuffer mismatch");
    if(!g.reset_stage_objects || !g.wait || !g.music_load || !g.music_play || !g.release_boss || !g.music_stop || !g.music_fade)throw std::invalid_argument("Konngara lifecycle callbacks required");
    active=&value;
}
KonngaraBinding::~KonngaraBinding() {active=previous;}
}
namespace konngara_original {
konngara_int_t boss_hp=0,boss_phase_frame=0;std::int8_t boss_phase=0;
void native_konngara_intro() {
    auto& b=backend();b.pattern=99;b.patterns_done=0;b.graphics.game_cleared=false;
    b.graphics.selected_page=th01_native::current_render_page();b.graphics.run();
    pellet_interlace=b.graphics.pellet_interlace;
    const_cast<Palette4&>(native_bomb_stage_palette())=unpack(b.graphics.stage_palette);
    z_palette_set_all_show(unpack(b.graphics.framebuffer.palette));page_access(b.graphics.selected_page);
}
void boss_palette_snap() {backend().palette=native_bomb_palette();}
void boss_palette_show() {z_palette_set_all_show(backend().palette);}
void grx_put(unsigned slot) {auto& g=backend().graphics;g.framebuffer.blit(th01_native::current_render_page(),0,0,g.effects.at(slot).images.at(0),true);}
void grx_free(unsigned slot) {backend().graphics.effects.at(slot)={};}
bool native_konngara_before_frame() {return !backend().graphics.game_cleared;}
void native_konngara_observe(int pattern,int done) {backend().pattern=pattern;backend().patterns_done=done;}
void native_konngara_defeat() {
    auto& g=backend().graphics;g.score=score;g.selected_page=th01_native::current_render_page();
    g.run_defeat();score=g.score;z_palette_set_all_show(unpack(g.framebuffer.palette));page_access(g.selected_page);
}
}

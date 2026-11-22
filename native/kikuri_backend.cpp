#include "kikuri_backend.hpp"
#include <cctype>
namespace {
th01_native::KikuriBackend* active=nullptr;
th01_native::KikuriBackend& backend() {if(!active) throw std::runtime_error("Kikuri backend is not bound");return *active;}
}
namespace th01_native {
KikuriBinding::KikuriBinding(KikuriBackend& value):previous(active) {
    if(!value.wait || !value.music_fade || !value.defeat) throw std::invalid_argument("Kikuri host callbacks are required");
    if(&value.framebuffer!=&graphics_framebuffer()) throw std::invalid_argument("Kikuri framebuffer mismatch");active=&value;
}
KikuriBinding::~KikuriBinding() {active=previous;}
}
namespace kikuri_original {
kikuri_int_t boss_hp=0,boss_phase_frame=0;Palette4 boss_palette;
void native_kikuri_reset_backend() {auto& b=backend();b.defeated=false;b.subphase=b.patterns_done=0;native_kikuri_ripple_sloppy()=false;}
bool native_kikuri_before_frame() {return !backend().defeated;}
void native_kikuri_observe(int subphase,int patterns) {auto& b=backend();b.subphase=subphase;b.patterns_done=patterns;}
void ptn_load(main_ptn_slot_t slot,const char* name) {
    if(slot!=PTN_SLOT_BOSS_1) throw std::invalid_argument("Kikuri ripple slot");std::string key=name;for(auto& c:key) c=char(std::toupper(static_cast<unsigned char>(c)));
    backend().ripple_sprites=th01_native::read_sprites(th01_native::asset(backend().archive,key));
}
void ptn_free(main_ptn_slot_t slot) {if(slot!=PTN_SLOT_BOSS_1) throw std::invalid_argument("Kikuri ripple slot");backend().ripple_sprites={};}
void native_kikuri_copy_line(screen_y_t y,std::uint16_t mask) {
    if(y<0 || y>=400) return;auto& fb=backend().framebuffer;
    for(unsigned plane=0;plane<4;++plane) for(unsigned x=0;x<80;++x) fb.pages[0][plane][y*80+x]=fb.pages[1][plane][y*80+x]&std::uint8_t(mask>>((x&1)*8));
    page_access(0);
}
void frame_delay(unsigned frames) {backend().wait(frames);}
void mdrv2_bgm_fade_out_nonblock() {backend().music_fade();}
void boss_defeat_animate() {auto& b=backend();if(!b.defeated) {b.defeated=true;b.defeat();}}
void boss_palette_snap() {boss_palette=native_bomb_palette();}
void boss_palette_show() {z_palette_set_all_show(boss_palette);}
Palette4& native_kikuri_stage_palette() {return const_cast<Palette4&>(native_bomb_stage_palette());}
void stage_palette_set(const Palette4& value) {native_kikuri_stage_palette()=value;}
}
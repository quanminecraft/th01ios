#include "sariel_body.hpp"
#include "boss_entity_backend.hpp"
#include "hp_backend.hpp"
namespace {
constexpr int RES_X=640,RES_Y=400;
std::int16_t boss_hp=0,boss_phase_frame=0;std::int8_t boss_phase=0;
#include "th01/main/boss/sariel_coords.inc"
#include "th01/main/boss/sariel_entities.inc"
#include "th01/main/boss/sariel_setup.inc"

constexpr int PTN_SLOT_WAND_LOWERED=0;
std::array<th01_native::Bytes,12> wand_background;
void ptn_snap_rect_from_1_8(int left,int top,int w,int h,int,int& image,int& x,int& y){
    auto& fb=th01_native::graphics_framebuffer();page_access(1);
    for(y=0;y<h/32;++y)for(x=0;x<w/32;++x){auto& tile=wand_background.at(image++);tile.resize(1024);for(int py=0;py<32;++py)for(int px=0;px<32;++px)tile[py*32+px]=fb.pixel(1,(left/8)*8+x*32+px,top+y*32+py);}page_access(0);
}
void wand_tiles_put(int left,int top,int w,int h,int,int& image,int& x,int& y){
    auto& fb=th01_native::graphics_framebuffer();auto page=th01_native::current_render_page();
    for(y=0;y<h/32;++y)for(x=0;x<w/32;++x){const auto& tile=wand_background.at(image++);if(tile.size()!=1024)throw std::logic_error("Sariel wand background has not been captured");for(int py=0;py<32;++py)for(int px=0;px<32;++px)fb.put(page,(left/8)*8+x*32+px,top+y*32+py,tile[py*32+px]);}
}
#define ptn_put_rect_noalpha_8(...) wand_tiles_put(__VA_ARGS__);
#include "th01/main/boss/sariel_wand_background.inc"
#undef ptn_put_rect_noalpha_8
#include "th01/main/boss/sariel_wand_lower.inc"

#include "th01/main/boss/sariel_shield.inc"
#include "th01/main/boss/sariel_wand_raise.inc"
#include "th01/main/boss/sariel_dress.inc"
bool active=false;
}
namespace th01_native {
struct SarielBody::Impl{
    Impl(){if(active)throw std::logic_error("Sariel body supports one instance");active=true;boss_entity_0={};boss_anims[0]={};boss_anims[1]={};try{sariel_ent_load();sariel_setup();for(auto& tile:wand_background)tile.clear();wand_render_raise_both(true);}catch(...){sariel_ent_free();active=false;throw;}}
    ~Impl(){sariel_ent_free();active=false;}
};
SarielBody::SarielBody():impl(std::make_unique<Impl>()){}SarielBody::~SarielBody()=default;
void SarielBody::render(int frame){if(frame<0 || frame>32767)throw std::invalid_argument("Sariel body frame");boss_phase_frame=frame;shield_render_both();dress_render_both();}
bool SarielBody::raise_wand(bool restart){return wand_render_raise_both(restart);}
void SarielBody::capture_wand_background(){wand_lowered_snap();}
void SarielBody::lower_wand(){wand_lower_both();}
SarielBodyState SarielBody::state()const{return{boss_hp,boss_phase,boss_phase_frame,ent_shield.image(),anm_dress.bos_image,anm_wand.bos_image};}
}
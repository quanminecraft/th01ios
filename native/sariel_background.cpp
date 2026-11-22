#include "sariel_background.hpp"
#include "grp.hpp"
#include "shot_backend.hpp"
#include "physics_types.hpp"
namespace {
constexpr int RES_X=640,RES_Y=400,ROW_SIZE=80,PLANE_SIZE=32000,COL_AIR=12,GPF_PALETTE_SHOW=1;
using vram_y_t=std::int16_t;using vram_offset_t=std::int16_t;using vc2=std::uint8_t;using dots16_t=std::uint16_t;using vram_word_amount_t=std::int16_t;using vram_byte_amount_t=std::int16_t;
struct Runtime {std::filesystem::path assets;unsigned target;int hp,frame=0;bool done=false;th01_native::SarielBackgroundHooks hooks;unsigned color=0;};Runtime* active=nullptr;
Runtime& current(){if(!active)throw std::logic_error("Sariel background is not bound");return *active;}
int boss_phase_frame=0,boss_hp=0;
const char* BG_IMAGES[]={"BOSS6_A1.GRP","BOSS6_A2.GRP","BOSS6_A3.GRP","BOSS6_A4.GRP"};
void grcg_setcolor_tcr(unsigned color){current().color=color;}
void grcg_setcolor_rmw(unsigned color){current().color=color;}
void grcg_off_func(){}
struct Chunk {
 int offset;
 operator dots16_t()const{auto& fb=th01_native::graphics_framebuffer();unsigned result=0;for(int bit=0;bit<16;++bit){int linear=offset*8+bit;if(linear>=0 && linear<640*400 && fb.pixel(th01_native::current_render_page(),linear%640,linear/640)==current().color)result|=(0x80u>>(bit&7))<<((bit/8)*8);}return result;}
 void operator=(dots16_t dots)const{auto& fb=th01_native::graphics_framebuffer();for(int bit=0;bit<16;++bit)if(dots&((0x80u>>(bit&7))<<((bit/8)*8))){int linear=offset*8+bit;if(linear>=0 && linear<640*400)fb.put(th01_native::current_render_page(),linear%640,linear/640,current().color);}}
};
Chunk grcg_chunk(int offset,int){return{offset};}
vram_offset_t vram_offset_shift(int x,int y){return std::int16_t(y*80+(x>>3));}
void z_vsync_wait_and_scrollup(int y){current().hooks.vsync();th01_native::graphics_framebuffer().display_scroll_y=(y%400+400)%400;}
void grp_put(const char* name,int){auto image=th01_native::read_grp(th01_native::read_file(current().assets/name));auto& fb=th01_native::graphics_framebuffer();fb.blit(th01_native::current_render_page(),0,0,image.images.at(0),false);fb.palette=image.palette;}
void graph_copy_page_to_other(unsigned page){th01_native::graphics_framebuffer().pages.at(page^1)=th01_native::graphics_framebuffer().pages.at(page);}
#define z_Palettes th01_native::graphics_framebuffer().palette
void stage_palette_set(const th01_native::Palette& palette){current().hooks.stage_palette(palette);}
void hud_rerender(){current().hooks.hud();}
void hud_hp_rerender(int hp){current().hooks.hp(hp);}
#include "th01/main/boss/sariel_background.inc"
}
namespace th01_native {
struct SarielBackground::Impl {
 Runtime runtime;
 Impl(const std::filesystem::path& assets,unsigned target,int hp,SarielBackgroundHooks hooks):runtime{assets,target,hp,0,false,std::move(hooks)}{if(active)throw std::logic_error("Sariel background supports one instance");if(target>3 || !runtime.hooks.vsync || !runtime.hooks.hud || !runtime.hooks.hp || !runtime.hooks.stage_palette)throw std::invalid_argument("Sariel background target/hooks");active=&runtime;}
 ~Impl(){active=nullptr;}
};
SarielBackground::SarielBackground(const std::filesystem::path& assets,unsigned target,int hp,SarielBackgroundHooks hooks):impl(std::make_unique<Impl>(assets,target,hp,std::move(hooks))){}
SarielBackground::~SarielBackground()=default;
bool SarielBackground::advance(){auto& r=impl->runtime;if(r.done)return true;boss_phase_frame=++r.frame;boss_hp=r.hp;bg_transition(r.target);r.frame=boss_phase_frame;r.done=r.frame==0;return r.done;}
int SarielBackground::frame()const{return impl->runtime.frame;}
}

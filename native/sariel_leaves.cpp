#include "sariel_leaves.hpp"
#include "pellet_backend.hpp"
#include "th01/main/playfld.hpp"
#include "th01/sprites/leaf.hpp"
#include <functional>
namespace {
using sariel_frame_t=int;
constexpr int RES_X=640,RES_Y=400,LEAFSPLASH_CELS=3,GRC_SLOT_LEAFSPLASH=0,ROW_SIZE=80,PLANE_SIZE=32000,BYTE_DOTS=8;
using vram_offset_t=std::int16_t;using dots8_t=std::uint8_t;using dots16_t=std::uint16_t;using vc2=std::uint8_t;
struct NativeLeafSprite {dots8_t row[8];};
th01_native::SpriteSet* sprites=nullptr;
std::function<th01_native::SarielLeafState(unsigned)> inspect;
template<class Flag>void native_leaf_bind(Flag* flag,DecimalSubpixel* left,DecimalSubpixel* top,DecimalSubpixel* vx,DecimalSubpixel* vy){if(!inspect)inspect=[=](unsigned i){return th01_native::SarielLeafState{int(flag[i]),left[i].v,top[i].v,vx[i].v,vy[i].v};};}
vram_offset_t vram_offset_divmul(int left,int top){return std::int16_t(top*80+left/8);}
unsigned mono_color=0;
void grcg_setcolor_rmw(unsigned color){mono_color=color;}
void grcg_off_func(){}
void grcg_put(vram_offset_t offset,dots16_t dots,int){auto& fb=th01_native::graphics_framebuffer();for(int bit=0;bit<16;++bit)if(dots&((0x80u>>(bit&7))<<((bit/8)*8))){int linear=int(offset)*8+bit;if(linear>=0 && linear<640*400)fb.put(th01_native::current_render_page(),linear%640,linear/640,mono_color);}}
#include "th01/hardware/grcg8x8m_body.inc"
void grc_put_8(int left,int top,int,int cel,unsigned color){th01_native::graphics_framebuffer().blit(th01_native::current_render_page(),left,top,sprites->images.at(cel),true,color);}
#define grc_sloppy_unput(left,top) egc_copy_rect_1_to_0_16(left,top,48,32);
#include "th01/main/boss/sariel_coords.inc"
#include "th01/main/boss/sariel_leaves.inc"
}
namespace th01_native {
struct SarielLeaves::Impl {
 SpriteSet images;
 Impl(const std::vector<Entry>& archive,std::uint32_t seed):images(read_sprites(asset(archive,"BOSS6GR4.GRC"))){if(sprites)throw std::logic_error("Sariel leaves support one instance");sprites=&images;inspect={};native_random_seed=seed;int frame=0;pattern_swaying_leaves(frame,999);}
 ~Impl(){int frame=0;pattern_swaying_leaves(frame,999);inspect={};sprites=nullptr;}
};
SarielLeaves::SarielLeaves(const std::vector<Entry>& archive,std::uint32_t seed):impl(std::make_unique<Impl>(archive,seed)){}
SarielLeaves::~SarielLeaves()=default;
void SarielLeaves::reset(){int frame=0;pattern_swaying_leaves(frame,999);}
void SarielLeaves::update(int frame,int interval){if(frame<0 || frame>32767 || interval<1)throw std::invalid_argument("Sariel leaf frame/interval");pattern_swaying_leaves(frame,interval);}
SarielLeafState SarielLeaves::state(unsigned index)const{if(index>=30)throw std::out_of_range("Sariel leaf slot");return inspect(index);}
}

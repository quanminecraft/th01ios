#include "sariel_particles.hpp"
#include "pellet_backend.hpp"
#include <functional>
#include <cmath>
namespace {
using std::abs;
constexpr int RES_X=640,RES_Y=400,ROW_SIZE=80,BYTE_DOTS=8,BYTE_MASK=7,COL_AIR=12,COL_PARTICLE2X2=4;
using svc_t=std::int8_t;using dots8_t=std::uint8_t;using vram_offset_t=std::int16_t;using vram_y_t=std::int16_t;
template<class T,int H>struct DotRect{T row[H];T& operator[](int i){return row[i];}};
bool active=false,native_particle_reset=false;unsigned color=0;
int boss_phase_frame=0;
std::array<std::function<th01_native::SarielParticleState(unsigned)>,3> inspectors;
template<class C,class P,class V>void native_particle_bind(unsigned kind,C* col,P* left,P* top,V* velocity,const int* age){if(!inspectors[kind])inspectors[kind]=[=](unsigned i){return th01_native::SarielParticleState{col[i]!=0,double(left[i]),double(top[i]),double(velocity[i]),age ? age[i]:0};};}
void grcg_setcolor_tcr(unsigned c){color=c;}void grcg_setcolor_rmw(unsigned c){color=c;}void grcg_off_func(){}
dots8_t grcg_chunk(int offset,int){auto& fb=th01_native::graphics_framebuffer();unsigned result=0;for(int bit=0;bit<8;++bit){int linear=offset*8+bit;if(linear>=0 && linear<640*400 && fb.pixel(th01_native::current_render_page(),linear%640,linear/640)==color)result|=128>>bit;}return result;}
dots8_t grcg_chunk_8(int offset){return grcg_chunk(offset,8);}
void grcg_put_emptyopt(int offset,dots8_t dots,int){if(!dots)return;auto& fb=th01_native::graphics_framebuffer();for(int bit=0;bit<8;++bit)if(dots&(128>>bit)){int linear=offset*8+bit;if(linear>=0 && linear<640*400)fb.put(th01_native::current_render_page(),linear%640,linear/640,color);}}
template<class T>vram_offset_t vram_offset_divmul(T x,T y){return static_cast<vram_offset_t>((x/8)+(y*80));}
vram_offset_t vram_offset_shift(int x,int y){return std::int16_t((x>>3)+y*80);}
#include "th01/main/boss/sariel_particle_macros.inc"
#include "th01/main/boss/sariel_particles_vertical.inc"
screen_x_t polar_y(int center,int radius,int angle){return ::polar_y(pixel_t(center),pixel_t(radius),angle);}
#include "th01/main/boss/sariel_particles_wavy.inc"
#include "th01/main/boss/sariel_particles_horizontal.inc"
void reset_all(){native_particle_reset=true;particles2x2_vertical_unput_update_render(false);particles2x2_wavy_unput_update_render();particles2x2_horizontal_unput_update_render(0);native_particle_reset=false;}
}
namespace th01_native {
struct SarielParticles::Impl{Impl(std::uint32_t seed){if(active)throw std::logic_error("Sariel particles support one instance");active=true;inspectors={};reset_all();native_random_seed=seed;}~Impl(){reset_all();inspectors={};active=false;}};
SarielParticles::SarielParticles(std::uint32_t seed):impl(std::make_unique<Impl>(seed)){}SarielParticles::~SarielParticles()=default;
void SarielParticles::reset(){reset_all();}
void SarielParticles::update(SarielParticleKind kind,int frame,bool from_bottom){if(frame<0 || frame>32767)throw std::invalid_argument("Sariel particle frame");boss_phase_frame=frame;switch(kind){case SarielParticleKind::Vertical:particles2x2_vertical_unput_update_render(from_bottom);break;case SarielParticleKind::Wavy:particles2x2_wavy_unput_update_render();break;case SarielParticleKind::Horizontal:particles2x2_horizontal_unput_update_render(frame);break;}}
SarielParticleState SarielParticles::state(SarielParticleKind kind,unsigned slot)const{if(slot>=30)throw std::out_of_range("Sariel particle slot");return inspectors.at(unsigned(kind))(slot);}
}

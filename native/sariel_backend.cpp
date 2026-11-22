#include "sariel_backend.hpp"
#include "grp.hpp"
#include <cctype>
namespace {
th01_native::SarielBackend* active=nullptr;
th01_native::SarielBackend& backend(){if(!active)throw std::logic_error("Sariel backend is not bound");return *active;}
std::string upper(const char* name){std::string key=name;for(auto& c:key)c=char(std::toupper(static_cast<unsigned char>(c)));return key;}
int floor8(int x){return x>=0 ? x/8*8:-((-x+7)/8)*8;}
}
namespace th01_native {
SarielBinding::SarielBinding(SarielBackend& b):previous(active){if(active)throw std::logic_error("Sariel supports one active instance");if(!b.wait || !b.delay_ms || !b.stage_reset || !b.hud || !b.audio)throw std::invalid_argument("Sariel host callbacks required");if(&b.framebuffer!=&graphics_framebuffer())throw std::invalid_argument("Sariel framebuffer mismatch");active=&b;boss_entity_0={};boss_anims[0]={};boss_anims[1]={};sariel_original::native_sariel_reset();}
SarielBinding::~SarielBinding(){active=previous;}
}
namespace sariel_original {
sariel_frame_t boss_hp=0,boss_phase_frame=0;std::int8_t boss_phase=0;bool game_cleared=false,native_particle_reset=false,native_actor_reset=false;
void native_sariel_observe(int pattern,int done,int invincibility,bool invincible){auto& b=backend();b.pattern=pattern;b.patterns_done=done;b.invincibility=invincibility;b.invincible=invincible;}
void native_sariel_bind_particles(unsigned kind,std::function<std::array<double,5>(unsigned)> read){auto& slot=backend().particles.at(kind);if(!slot)slot=std::move(read);}
void native_sariel_bind_leaves(std::function<std::array<int,5>(unsigned)> read){if(!backend().leaves)backend().leaves=std::move(read);}
void frame_delay(unsigned frames){backend().wait(frames);}void delay(unsigned ms){backend().delay_ms(ms);}
void boss_palette_snap(){backend().palette=native_bomb_palette();}void boss_palette_show(){::z_palette_set_all_show(backend().palette);}
void stage_palette_set(const Palette4& p){const_cast<Palette4&>(native_bomb_stage_palette())=p;}
LoadedPalette& native_sariel_grp_palette(){return backend().loaded;}
void native_sariel_black_out_step(){auto& p=native_bomb_palette();auto& fb=backend().framebuffer;for(unsigned col=0;col<16;++col){unsigned argb=0xff000000;for(unsigned c=0;c<3;++c){auto& v=p[col].component(c);v=std::max<std::int16_t>(0,v-1);argb|=unsigned(v*17)<<(16-c*8);}fb.palette[col]=argb;}}
void text_fillca(char,int){backend().framebuffer.text_overlay.clear();}
void stageobjs_init_and_render(int){backend().stage_reset();}
void mdrv2_bgm_load(const char* name){backend().audio("bgm_load",name);}void mdrv2_bgm_play(){backend().audio("bgm_play","");}void mdrv2_bgm_stop(){backend().audio("bgm_stop","");}void mdrv2_bgm_fade_out_nonblock(){backend().audio("bgm_fade","");}void mdrv2_se_load(const char* name){backend().audio("se_load",name);}
void grp_put(const char* name,int flags){auto& b=backend();auto key=upper(name);auto path=b.assets/key;if(!std::filesystem::exists(path)){b.missing_images.push_back(key);return;}auto image=th01_native::read_grp(th01_native::read_file(path));b.framebuffer.blit(th01_native::current_render_page(),0,0,image.images.at(0),false);Palette4 p;for(unsigned col=0;col<16;++col)for(unsigned c=0;c<3;++c)b.loaded[col].v[c]=p[col].component(c)=((image.palette[col]>>(16-c*8))&255)/17;if(flags&GPF_PALETTE_SHOW)::z_palette_set_all_show(p);}
void hud_rerender(){backend().hud();}void hud_hp_rerender(int hp){::hud_hp_rerender(hp);}
void EGCCopy::rect_interpage(int x,int y,int w,int h,unsigned source){backend().framebuffer.restore(source,source^1,x,y,w,h);page_access(source^1);}
void grc_load(int slot,const char* name){backend().grc.at(slot)=th01_native::read_sprites(th01_native::asset(backend().archive,upper(name)));}void grc_free(int slot){backend().grc.at(slot)={};}
void native_sariel_grc_put(screen_x_t left,screen_y_t top,int slot,int cel,unsigned color){auto& sprite=backend().grc.at(slot).images.at(cel);auto& fb=backend().framebuffer;if(left< -sprite.width || left>=640 || top< -sprite.height || top>=400)return;int row=top*80+floor8(left)/8;for(int y=0;y<sprite.height;++y){if(row>=0)for(int byte=0;byte<sprite.width/8;++byte){int off=row+byte;if(!((left>=0 && off/80==row/80)||(left<0 && off/80!=row/80)))continue;for(int bit=0;bit<8;++bit)if(sprite.opaque.at(y*sprite.width+byte*8+bit)){int linear=off*8+bit;if(linear>=0 && linear<640*400)fb.put(th01_native::current_render_page(),linear%640,linear/640,color);}}row+=80;if(row>=32000)break;}}
void ptn_new(int slot,int count){backend().tiles.at(slot).assign(count,th01_native::Bytes(1024,0));}void ptn_free(int slot){backend().tiles.at(slot).clear();}
void ptn_snap_rect_from_1_8(screen_x_t left,screen_y_t top,int w,int h,int slot,int& image,int& x,int& y){auto& fb=backend().framebuffer;page_access(1);for(y=0;y<h/32;++y)for(x=0;x<w/32;++x){auto& tile=backend().tiles.at(slot).at(image++);for(int py=0;py<32;++py)for(int px=0;px<32;++px){int linear=(top+y*32+py)*640+floor8(left)+x*32+px;tile[py*32+px]=linear>=0 && linear<640*400 ? fb.pixel(1,linear%640,linear/640):0;}}page_access(0);}
void native_sariel_tiles_put(screen_x_t left,screen_y_t top,int w,int h,int slot,int& image,int& x,int& y){auto& fb=backend().framebuffer;for(y=0;y<h/32;++y)for(x=0;x<w/32;++x){auto& tile=backend().tiles.at(slot).at(image++);for(int py=0;py<32;++py)for(int px=0;px<32;++px){int linear=(top+y*32+py)*640+floor8(left)+x*32+px;if(linear>=0 && linear<640*400)fb.put(th01_native::current_render_page(),linear%640,linear/640,tile[py*32+px]);}}}
void grcg_setcolor_tcr(unsigned c){backend().color=c;}void grcg_setcolor_rmw(unsigned c){backend().color=c;}void grcg_off_func(){}
GrcgChunk grcg_chunk(int offset,int width){return{offset,width};}
GrcgChunk::operator std::uint16_t()const{unsigned result=0;auto& b=backend();for(int bit=0;bit<width;++bit){int linear=offset*8+bit;if(linear>=0 && linear<640*400 && b.framebuffer.pixel(th01_native::current_render_page(),linear%640,linear/640)==b.color)result|=(0x80u>>(bit&7))<<((bit/8)*8);}return result;}
void GrcgChunk::operator=(std::uint16_t dots)const{auto& b=backend();for(int bit=0;bit<width;++bit)if(dots&((0x80u>>(bit&7))<<((bit/8)*8))){int linear=offset*8+bit;if(linear>=0 && linear<640*400)b.framebuffer.put(th01_native::current_render_page(),linear%640,linear/640,b.color);}}
dots8_t grcg_chunk_8(int offset){return grcg_chunk(offset,8);}void grcg_put_emptyopt(int offset,dots8_t dots,int width){if(dots)grcg_chunk(offset,width)=dots;}
void grcg_put(vram_offset_t offset,dots16_t dots,int width){grcg_chunk(offset,width)=dots;}
#include "th01/hardware/grcg8x8m_body.inc"
}
namespace diagonal_shared {
constexpr int RES_X=640,RES_Y=400,INTERLEAVE_W=8,INTERLEAVE_H=8;
using svc2=std::int16_t;using Palette4=sariel_original::LoadedPalette;
void graph_interleave_pages_8x8_8(int left,int top,int mask_id){auto& fb=backend().framebuffer;for(int y=0;y<8;++y){int at=(top+y)*80+left/8;if(at<0 || at>=32000)continue;unsigned mask=((y+(mask_id==2))%2)==0 ? 0x55:0xaa;for(unsigned p=0;p<4;++p)fb.pages[0][p][at]=(fb.pages[1][p][at]&~mask)|(fb.pages[0][p][at]&mask);}page_access(0);}
void z_palette_set_all_show(const Palette4& p){::Palette4 converted;for(unsigned col=0;col<16;++col)for(unsigned c=0;c<3;++c)converted[col].component(c)=p.colors[col].v[c];::z_palette_set_all_show(converted);}
void delay(unsigned ms){sariel_original::delay(ms);}
#define palette_foreach(col,comp,body) for(col=0;col<16;++col)for(comp=0;comp<3;++comp){body}
#include "th01/hardware/pgtrans_diagonal.inc"
#undef palette_foreach
}
namespace sariel_original {
void pagetrans_diagonal_8x8_with_palette(unsigned ms,::Palette4& current,const LoadedPalette& target){LoadedPalette p;for(unsigned col=0;col<16;++col)for(unsigned c=0;c<3;++c)p[col].v[c]=current[col].component(c);diagonal_shared::pagetrans_diagonal_8x8_with_palette(ms,p,target);for(unsigned col=0;col<16;++col)for(unsigned c=0;c<3;++c)current[col].component(c)=p[col].v[c];}
void pagetrans_diagonal_8x8(unsigned ms){LoadedPalette target;auto& current=native_bomb_palette();for(unsigned col=0;col<16;++col)for(unsigned c=0;c<3;++c)target[col].v[c]=current[col].component(c);pagetrans_diagonal_8x8_with_palette(ms,current,target);}
}

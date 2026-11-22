#include "elis_backend.hpp"
#include <cctype>
namespace {
th01_native::ElisBackend* active=nullptr;
th01_native::ElisBackend& backend(){if(!active)throw std::logic_error("Elis backend is not bound");return *active;}
int floor8(int x) {return x>=0 ? x/8*8 : -((-x+7)/8)*8;}
th01_native::Bytes& tile(int id) {return backend().tiles.at(id/64).at(id%64);}
void tile_put(screen_x_t left,screen_y_t top,int id,int quarter=-1) {
    auto& fb=backend().framebuffer;auto& pixels=tile(id);int width=quarter<0 ? 32 : 16;
    int sx=quarter<0 ? 0 : (quarter&1)*16,sy=quarter<0 ? 0 : (quarter>>1)*16;
    int aligned=floor8(left);auto page=th01_native::current_render_page();
    for(int y=0;y<width;++y) for(int x=0;x<width;++x) {int linear=(top+y)*640+aligned+x;if(linear>=0 && linear<640*400) fb.put(page,linear%640,linear/640,pixels.at((sy+y)*32+sx+x));}
}
void tile_snap(screen_x_t left,screen_y_t top,int id) {
    auto& fb=backend().framebuffer;auto& pixels=tile(id);int aligned=floor8(left);
    for(int y=0;y<32;++y) for(int x=0;x<32;++x) {int linear=(top+y)*640+aligned+x;pixels[y*32+x]=linear>=0 && linear<640*400 ? fb.pixel(1,linear%640,linear/640) : 0;}
}
}
namespace th01_native {
ElisBinding::ElisBinding(ElisBackend& value):previous(active){if(active)throw std::logic_error("Elis supports one active instance");if(!value.wait || !value.delay_ms || !value.music_fade || !value.defeat)throw std::invalid_argument("Elis callbacks required");if(&value.framebuffer!=&graphics_framebuffer())throw std::invalid_argument("Elis framebuffer mismatch");active=&value;}
ElisBinding::~ElisBinding(){active=previous;}
}
namespace elis_original {
elis_int_t boss_hp=0,boss_phase_frame=0;std::int8_t boss_phase=0;
void native_elis_reset(){auto& b=backend();b.defeated=false;b.reset_pending=true;b.bat=false;b.pattern=b.invincibility=b.bat_pattern=b.girl_pattern=b.subphase=0;}
bool native_elis_before_frame(){backend().missiles.frame=std::int16_t(native_boss_frame());return !backend().defeated;}
bool native_elis_take_reset(){bool reset=backend().reset_pending;backend().reset_pending=false;return reset;}
void native_elis_observe(bool bat,int pattern,int invincibility){auto& b=backend();b.bat=bat;b.pattern=pattern;b.invincibility=invincibility;}
void native_elis_observe_girl(int pattern){backend().girl_pattern=pattern;}
void native_elis_observe_final(int subphase,int pattern){backend().subphase=subphase;backend().bat_pattern=pattern;}
void ptn_new(int slot,int count) {backend().tiles.at(slot).assign(count,th01_native::Bytes(1024,0));}
void ptn_free(int slot) {auto& b=backend();b.tiles.at(slot).clear();if(b.missiles.slot==slot) {b.missiles.sprites={};b.missiles.slot=-1;}}
void ptn_snap_rect_from_1_8(screen_x_t left,screen_y_t top,int w,int h,int slot,int& image,int& x,int& y) {
    page_access(1);for(y=0;y<h/32;++y) for(x=0;x<w/32;++x) {tile_snap(left+x*32,top+y*32,PTN_ID(slot,image));++image;}page_access(0);
}
void ptn_put_row_noalpha_8(screen_x_t left,screen_y_t top,int w,int slot,int& image,int& x) {for(x=0;x<w/32;++x) {tile_put(left+x*32,top,PTN_ID(slot,image));++image;}}
void ptn_put_rect_noalpha_8(screen_x_t left,screen_y_t top,int w,int h,int slot,int& image,int& x,int& y) {for(y=0;y<h/32;++y) ptn_put_row_noalpha_8(left,top+y*32,w,slot,image,x);}
void native_elis_grc_put(screen_x_t left,screen_y_t top,int slot,int image,unsigned color){
    if(slot!=GRC_SLOT_BOSS_1)throw std::invalid_argument("Elis GRC slot");
    auto& sprite=backend().grc.images.at(image);auto& fb=th01_native::graphics_framebuffer();
    if(left< -sprite.width || left>=640 || top< -sprite.height || top>=400)return;
    int row=top*80+floor8(left)/8;
    for(int y=0;y<sprite.height;++y){
        if(row>=0)for(int byte=0;byte<sprite.width/8;++byte){int off=row+byte;if(!((left>=0 && off/80==row/80)||(left<0 && off/80!=row/80)))continue;
            for(int bit=0;bit<8;++bit)if(sprite.opaque.at(y*sprite.width+byte*8+bit)){int linear=off*8+bit;if(linear>=0 && linear<640*400)fb.put(th01_native::current_render_page(),linear%640,linear/640,color);}}
        row+=80;if(row>=32000)break;
    }
}
void grc_load(int slot,const char* name){if(slot!=GRC_SLOT_BOSS_1)throw std::invalid_argument("Elis GRC slot");std::string key=name;for(auto& c:key)c=char(std::toupper(static_cast<unsigned char>(c)));backend().grc=th01_native::read_sprites(th01_native::asset(backend().archive,key));}
void grc_free(int slot){if(slot!=GRC_SLOT_BOSS_1)throw std::invalid_argument("Elis GRC slot");backend().grc={};}
void frame_delay(unsigned n){backend().wait(n);}void delay(unsigned ms){backend().delay_ms(ms);}
void mdrv2_bgm_fade_out_nonblock(){backend().music_fade();}
void boss_defeat_animate(){auto& b=backend();if(!b.defeated){b.defeated=true;b.defeat();}}
void boss_palette_snap(){backend().palette=native_bomb_palette();}void boss_palette_show(){z_palette_set_all_show(backend().palette);}
}
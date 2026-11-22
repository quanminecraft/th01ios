#include "konngara_entrance.hpp"
#include "grp.hpp"
#include "grz.hpp"
#include <cstdint>
namespace {
using namespace th01_native;
using svc2=std::int8_t;
constexpr int RES_X=640,RES_Y=400,BOSS_STAGE=-1,TX_BLACK=0,TX_REVERSE=1,TX_WHITE=7;
constexpr int GPF_PALETTE_SHOW=1,GPF_COLORKEY=2;
constexpr const char* SCROLL_BG_FN="boss7_d1.grp";
constexpr const char* GRZ_FN="boss8.grz";
constexpr const char* SE_FN="zigoku.mde";
struct RGB4 {static constexpr int Range=16;};
struct Color {struct {int r=0,g=0,b=0;} c;};
using Colors=std::array<Color,16>;
struct Context {KonngaraEntrance& state;Colors current{},grp{};};
Context* active=nullptr;
Context& ctx() {if(!active) throw std::logic_error("Konngara entrance is not bound");return *active;}
Colors unpack(const Palette& palette) {Colors result;for(unsigned i=0;i<16;++i) {auto v=palette[i];result[i].c={int((v>>16)&255)/17,int((v>>8)&255)/17,int(v&255)/17};}return result;}
Palette pack(const Colors& colors) {Palette result;for(unsigned i=0;i<16;++i) {auto c=colors[i].c;result[i]=0xff000000u|(unsigned(c.r)*17<<16)|(unsigned(c.g)*17<<8)|unsigned(c.b)*17;}return result;}
void z_palette_set_all_show(const Colors& colors) {ctx().current=colors;ctx().state.framebuffer.palette=pack(colors);}
void z_palette_set_show(unsigned i,int r,int g,int b) {ctx().current.at(i).c={r,g,b};ctx().state.framebuffer.palette=pack(ctx().current);}
void black() {z_palette_set_all_show(Colors{});}
void ramp() {auto& c=ctx();for(unsigned i=0;i<16;++i) {auto& v=c.current[i].c;auto t=c.grp[i].c;v.r+=v.r<t.r;v.g+=v.g<t.g;v.b+=v.b<t.b;}z_palette_set_all_show(c.current);}
void text_fillca(char,int) {ctx().state.framebuffer.text_overlay.clear();}
void page_access(unsigned page) {if(page>1) throw std::out_of_range("Konngara page");ctx().state.selected_page=page;}
std::filesystem::path path(const char* name) {std::string upper=name;for(auto& c:upper) if(c>='a' && c<='z') c-=32;return ctx().state.directory/upper;}
void grp_put(const char* name,int flags) {
    auto image=read_grp(read_file(path(name)));ctx().grp=unpack(image.palette);
    if(flags&GPF_COLORKEY) {auto& sprite=image.images.at(0);for(unsigned i=0;i<sprite.color.size();++i) sprite.opaque[i]=sprite.color[i]!=15;}
    ctx().state.framebuffer.blit(ctx().state.selected_page,0,0,image.images.at(0),flags&GPF_COLORKEY);
    if(flags&GPF_PALETTE_SHOW) z_palette_set_all_show(ctx().grp);
}
void stage_palette_set(const Colors& colors) {ctx().state.stage_palette=pack(colors);}
void stageobjs_init_and_render(int) {ctx().state.reset_stage_objects();}
void mdrv2_bgm_load(const char* name) {ctx().state.music_load(name);}
void mdrv2_bgm_play() {ctx().state.music_play();}
void mdrv2_se_load(const char*) {} // Reloading the same bank has no visual effect.
void mdrv2_se_play(unsigned id) {ctx().state.sound_commands.push_back(id);}
void konngara_ent_load() {
    auto& entities=ctx().state.entities;auto archive=load_archive(ctx().state.directory);
    const char* names[]={"BOSS8_1.BOS","BOSS8_E1.BOS","BOSS8_E2.BOS"};
    for(unsigned i=0;i<3;++i) entities[i]=read_sprites(asset(archive,names[i]));
}
void grz_load_single(unsigned slot,const char* name,unsigned image) {ctx().state.effects.at(slot)=read_grz_image(read_file(path(name)),image);}
void z_vsync_wait_and_scrollup(int y) {ctx().state.framebuffer.display_scroll_y=((y%400)+400)%400;}
void frame_delay(unsigned n) {ctx().state.wait(n);}
void graph_copy_page_to_other(unsigned source) {auto& fb=ctx().state.framebuffer;fb.pages.at(1-source)=fb.pages.at(source);}
struct EGCCopy {
    void rect_interpage(int left,int top,int width,int height,unsigned source) {
        auto& fb=ctx().state.framebuffer;
        // The original first pan rectangle begins at y=400, outside the
        // displayed planes. Discard those offscreen writes, as other backends do.
        for(int y=std::max(0,top);y<std::min(400,top+height);++y)
            for(int x=std::max(0,left);x<std::min(640,left+width);++x) fb.put(1-source,x,y,fb.pixel(source,x,y));
        page_access(1-source);
    }
};
void siddham_col_white() {z_palette_set_show(9,15,15,15);}
void siddham_fade() {auto& c=ctx().current[9].c;if(c.r>0)--c.r;if(c.g>9)--c.g;if(c.b>10)--c.b;z_palette_set_all_show(ctx().current);}
void fade_out_step() {for(auto& col:ctx().current) {auto& c=col.c;c.r-=c.r>0;c.g-=c.g>0;c.b-=c.b>0;}z_palette_set_all_show(ctx().current);}
void z_graph_clear() {ctx().state.framebuffer.clear(ctx().state.selected_page,0);}
void grx_put(unsigned slot) {ctx().state.framebuffer.blit(ctx().state.selected_page,0,0,ctx().state.effects.at(slot).images.at(0),true);}
void grx_free(unsigned slot) {ctx().state.effects.at(slot)={};}
void konngara_free() {auto& s=ctx().state;s.release_boss();for(auto& bank:s.entities)bank={};for(auto& bank:s.effects)bank={};}
void mdrv2_bgm_stop() {ctx().state.music_stop();}
void mdrv2_bgm_fade_out_nonblock() {ctx().state.music_fade();}
void text_command(const char*) {ctx().state.framebuffer.text_overlay.clear();}
#define pellet_interlace ctx().state.pellet_interlace
#define z_Palettes ctx().current
#define grp_palette ctx().grp
#define z_palette_set_black(a,b) black()
#define z_palette_black_in_step_to(a,b,c) ramp();
#define siddham_col_white_in_step() siddham_fade()
#include "th01/main/boss/konngara_entrance.inc"
#include "th01/main/boss/konngara_kuji.inc"
#define printf text_command
#define text_fill_black(a,b) text_command("")
#define text_clear_sloppy(a,b) text_command("")
#define z_palette_black_out_step(a,b) fade_out_step()
#define z_palette_black_out_step_2(a,b) fade_out_step()
#define score ctx().state.score
#define game_cleared ctx().state.game_cleared
constexpr int V_WHITE=7;
void defeat_run() {
    int i=0,j=0,comp=0,scroll_frame=0;svc2 col=0;std::int16_t boss_phase_frame=0;
#include "th01/main/boss/konngara_defeat.inc"
}
#undef printf
#undef text_fill_black
#undef text_clear_sloppy
#undef z_palette_black_out_step
#undef z_palette_black_out_step_2
#undef score
#undef game_cleared
#undef pellet_interlace
#undef z_Palettes
#undef grp_palette
#undef z_palette_set_black
#undef z_palette_black_in_step_to
#undef siddham_col_white_in_step
}
namespace th01_native {
void KonngaraEntrance::run() {
    if(!reset_stage_objects || !wait || !music_load || !music_play) throw std::invalid_argument("Konngara entrance host callbacks required");
    Context local{*this};auto previous=active;active=&local;
    try {konngara_load_and_entrance(0);active=previous;}catch(...) {active=previous;throw;}
}
void KonngaraEntrance::run_defeat() {
    if(game_cleared)return;
    if(!wait || !release_boss || !music_stop || !music_fade)throw std::invalid_argument("Konngara defeat callbacks required");
    Context local{*this,unpack(framebuffer.palette)};auto previous=active;active=&local;
    try {defeat_run();active=previous;}catch(...) {active=previous;throw;}
}
}

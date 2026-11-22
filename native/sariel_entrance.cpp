#include "sariel_entrance.hpp"
#include "physics_types.hpp"
#include "grp.hpp"
#include <cctype>
namespace {
using namespace th01_native;
constexpr int RES_X=640,RES_Y=400,COLOR_COUNT=16,COMPONENT_COUNT=3,INTERLEAVE_W=8,INTERLEAVE_H=8;
constexpr int TX_BLACK=0,TX_REVERSE=1,TX_WHITE=7,GPF_PALETTE_SHOW=1,BOSS_STAGE=-1;
constexpr const char* SE_FN="zigoku.mde";
const char* BG_IMAGES[]={"BOSS6_A1.GRP","BOSS6_A2.GRP","BOSS6_A3.GRP","BOSS6_A4.GRP"};
struct Color{std::int16_t v[3]{};};struct Palette4{std::array<Color,16> colors{};Color& operator[](unsigned i){return colors.at(i);}const Color& operator[](unsigned i)const{return colors.at(i);}};
using svc2=std::int16_t;using vram_y_t=std::int16_t;
struct Context{SarielEntrance& state;Palette4 palette;};Context* active=nullptr;
Context& ctx(){if(!active)throw std::logic_error("Sariel entrance is not bound");return *active;}
void page_access(unsigned p){if(p>1)throw std::out_of_range("Sariel page");ctx().state.selected_page=p;}
void z_palette_set_all_show(const Palette4& pal){ctx().palette=pal;for(unsigned i=0;i<16;++i)ctx().state.framebuffer.palette[i]=0xff000000u|(unsigned(pal[i].v[0])*17<<16)|(unsigned(pal[i].v[1])*17<<8)|unsigned(pal[i].v[2])*17;}
void grp_put(const char* name,int flags){std::string upper=name;for(auto& c:upper)c=char(std::toupper(static_cast<unsigned char>(c)));auto image=read_grp(read_file(ctx().state.directory/upper));ctx().state.framebuffer.blit(ctx().state.selected_page,0,0,image.images.at(0),false);if(flags&GPF_PALETTE_SHOW){Palette4 p;for(unsigned i=0;i<16;++i)for(unsigned c=0;c<3;++c)p[i].v[c]=(image.palette[i]>>(16-c*8)&255)/17;z_palette_set_all_show(p);}}
void text_fillca(char,int){ctx().state.framebuffer.text_overlay.clear();}
void stageobjs_init_and_render(int){ctx().state.reset_stage_objects();}
void mdrv2_bgm_load(const char* name){ctx().state.music_load(name);}void mdrv2_se_load(const char* name){ctx().state.se_load(name);}void mdrv2_bgm_play(){ctx().state.music_play();}
void frame_delay(unsigned n){ctx().state.wait(n);}void delay(unsigned n){ctx().state.delay_ms(n);}
void z_vsync_wait_and_scrollup(int y){ctx().state.wait(1);ctx().state.framebuffer.display_scroll_y=(y%400+400)%400;}
struct EGCCopy{void rect_interpage(int x,int y,int w,int h,unsigned source){ctx().state.framebuffer.restore(source,source^1,x,y,w,h);page_access(source^1);}};
void graph_interleave_pages_8x8_8(int left,int top,int mask_id){
    // Generated source BMP masks 1/2: 55/AA alternating rows and its inverse.
    if(mask_id!=1 && mask_id!=2)throw std::invalid_argument("Sariel diagonal mask");auto& fb=ctx().state.framebuffer;int byte=left/8;
    for(int y=0;y<8;++y){int at=(top+y)*80+byte;if(at<0 || at>=32000)continue;unsigned mask=((y+(mask_id==2))%2)==0 ? 0x55:0xaa;for(unsigned p=0;p<4;++p)fb.pages[0][p][at]=(fb.pages[1][p][at]&~mask)|(fb.pages[0][p][at]&mask);}
    page_access(0);
}
#define palette_foreach(col,comp,body) for(col=0;col<16;++col)for(comp=0;comp<3;++comp){body}
#include "th01/hardware/pgtrans_diagonal.inc"
void pagetrans_diagonal_8x8(unsigned ms){pagetrans_diagonal_8x8_with_palette(ms,ctx().palette,ctx().palette);}
#define pellet_interlace ctx().state.pellet_interlace
#define frame_rand ctx().state.frame_seed
#define random_seed native_random_seed
#include "th01/main/boss/sariel_entrance.inc"
}
namespace th01_native {
void SarielEntrance::run(){if(active)throw std::logic_error("Sariel entrance already running");if(!reset_stage_objects || !wait || !delay_ms || !music_load || !se_load || !music_play)throw std::invalid_argument("Sariel entrance callbacks required");Context context{*this,{}};active=&context;try{sariel_entrance(0);}catch(...){active=nullptr;throw;}active=nullptr;}
}
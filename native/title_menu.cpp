#include "title_menu.hpp"
#include "game/input.hpp"
#include "practice_menu.hpp"
#include "th01/math/clamp.hpp"
#include <any>
#include <cstdio>
#include <fstream>
#include <map>
#include <utility>

namespace th01_native {
namespace title_literals {
using pixel_t=int;
constexpr int GLYPH_FULL_W=16;
#include "title_strings.inc"
}
using namespace title_literals;
struct TitleMenu::Impl {
 using bool16=std::int16_t;
 using vc_t=unsigned;
 using vc2=unsigned;
 using screen_x_t=int;
 using screen_y_t=int;
 using pixel_t=int;
 using vram_offset_t=int;
 using vram_dword_amount_t=int;
 using dots32_t=std::uint32_t;
 using shiftjis_t=char;
 struct REGS {struct {unsigned char ah=0,al=0;} h;};
 static constexpr int RES_X=640,RES_Y=400,ROW_SIZE=80,GLYPH_H=16,GLYPH_FULL_W=16;
 static constexpr int FX_WEIGHT_BLACK=48,FX_WEIGHT_BOLD=32;
 static constexpr int RANK_COUNT=4,RANK_LUNATIC=3,BGM_MODE_COUNT=2,CFG_CREDIT_LIVES_EXTRA_MAX=5;
 static constexpr int GPF_PALETTE_SHOW=1,GPF_PALETTE_KEEP=0,GPF_COLORKEY=2;
 Framebuffer fb;
 const FontRom& font;
 std::filesystem::path profile;
 TitleHost host;
 GameOptions opts;
 InputKeys keys;
 std::map<std::string,std::any> locals;
 unsigned page=0,frame_rand=0,rng=0;
 bool input_ok=false,input_shot=false,hit_key=true;
 bool release_after_hit_key=false;
 TitleAction action=TitleAction::menu;
 template<class T> T& local(const char* function,const char* name,T initial) {
  auto key=std::string(function)+":"+name;
  auto [it,inserted]=locals.emplace(key,std::move(initial));
  return std::any_cast<T&>(it->second);
 }
 template<std::size_t N> static constexpr int shiftjis_w(const char (&)[N]) {return (N-1)*8;}
 void frame_delay(unsigned frames) {host.present_wait(fb,frames);}
 void page_access(unsigned value) {page=value;}
 void int86(int,REGS* in,REGS* out) {
  unsigned mask=0;
  switch(in->h.al) {
   case 0:mask=keys.escape?K0_ESC:0;break;
   case 3:mask=keys.enter?K3_RETURN:0;break;
   case 5:mask=keys.shot?K5_Z:0;break;
   case 7:mask=(keys.up?K7_ARROW_UP:0)|(keys.down?K7_ARROW_DOWN:0)|(keys.left?K7_ARROW_LEFT:0)|(keys.right?K7_ARROW_RIGHT:0);break;
   case 8:mask=(keys.num_up?K8_NUM_8:0)|(keys.num_left?K8_NUM_4:0);break;
   case 9:mask=(keys.num_down?K9_NUM_2:0)|(keys.num_right?K9_NUM_6:0);break;
  }
  out->h.ah=mask;
 }
 unsigned irand() {rng=rng*0x015a4e35u+1;return rng>>16&0x7fff;}
 void graph_copy_page_to_other(unsigned source) {fb.pages[!source]=fb.pages[source];page=!source;}
 static int vram_offset_shift(int x,int y) {return y*80+x/8;}
 void grcg_setcolor_rmw(unsigned) {}
 void grcg_off_func() {}
 void grcg_put(int offset,std::uint32_t bits,int) {
  for(unsigned p=0;p<4;++p)for(unsigned b=0;b<4;++b)fb.pages[page][p].at(offset+b)|=bits>>(b*8);
 }
 void egc_copy_rect_1_to_0_16(int x,int y,int w,int h) {
  int left=x/16*16,right=(x+w+15)/16*16;fb.restore(1,0,left,y,right-left,h);page=0;
 }
 void egc_copy_rect_1_to_0_16_word_w(int x,int y,int w,int h) {egc_copy_rect_1_to_0_16(x,y,(w/16)*16+16,h);}
 void graph_putsa_fx(int x,int y,int fx,const char* text) {font.shift_jis(fb,page,x,y,fx,text);}
 template<class... Args> void graph_printf_fx(int x,int y,int fx,const char* fmt,Args... args) {
  char text[512]{};int length=std::snprintf(text,sizeof(text),fmt,args...);
  if(length<0||length>=int(sizeof(text)))throw std::runtime_error("Title text overflow");
  graph_putsa_fx(x,y,fx,text);
 }
 void grp_put(const char* name,int flags) {
  auto image=host.load(name);
  if(image.images.size()!=1||image.images[0].width!=640||image.images[0].height!=400)throw std::runtime_error("Invalid title GRP");
  if(flags==GPF_COLORKEY) {
   auto sprite=image.images[0];for(unsigned i=0;i<sprite.color.size();++i)sprite.opaque[i]=sprite.color[i]!=15;
   fb.blit(page,0,0,sprite,true);
  }else fb.blit(page,0,0,image.images[0],false);
  if(flags==GPF_PALETTE_SHOW)fb.palette=image.palette;
 }
 Palette fade_target{};
 void z_palette_black() {fade_target=fb.palette;fb.palette.fill(0xff000000u);}
 void z_palette_black_in() {
  for(unsigned step=1;step<=16;++step) {
   frame_delay(1);
   for(unsigned i=0;i<16;++i) {
    std::uint32_t value=0xff000000u;
    for(unsigned shift:{0u,8u,16u})value|=std::min(step,(fade_target[i]>>shift&255)/17)*17<<shift;
    fb.palette[i]=value;
   }
   host.delay_ms(10);
  }
 }
 void mdrv2_bgm_load(const char* name) {if(opts.bgm_mode||menu_id==MID_MUSIC)host.audio("bgm_load",name);}
 void mdrv2_bgm_play() {if(opts.bgm_mode||menu_id==MID_MUSIC)host.audio("bgm_play","");}
 void mdrv2_bgm_stop() {host.audio("bgm_stop","");}
 void cfg_load() {
  std::ifstream in(profile/"reiiden.cfg",std::ios::binary);
  std::array<unsigned char,11> bytes{};
  if(!in.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return;
  if(std::string(reinterpret_cast<char*>(bytes.data()),7)!="REIIDEN")return;
  if(bytes[7]>3||bytes[8]>1||bytes[9]>5||bytes[10]>4)return;
  opts={std::int8_t(bytes[7]),std::int8_t(bytes[8]),std::int8_t(bytes[9]),std::int8_t(bytes[10])};
 }
 void cfg_save() {
  std::filesystem::create_directories(profile);
  std::ofstream out(profile/"reiiden.cfg",std::ios::binary|std::ios::trunc);
  const char bytes[]={'R','E','I','I','D','E','N',opts.rank,opts.bgm_mode,opts.credit_bombs,opts.credit_lives_extra};
  if(!out.write(bytes,sizeof(bytes)))throw std::runtime_error("Cannot save reiiden.cfg");
 }
 void start_game(bool new_game) {cfg_save();action=new_game?TitleAction::new_game:TitleAction::continue_game;}
#define OP_INPUT_PREV auto& input_prev=local<std::array<bool16,2>>(__func__,"input_prev",{})
#define OP_LOCAL(type,name,value) auto& name=local<type>(__func__,#name,value)
#define OP_BGM_REF opts.bgm_mode
#define RANKS_CAPS_CENTERED {" EASY ","NORMAL"," HARD ","LUNATIC"}
#define BGM_MODES_CENTERED {"  OFF ","  FM  "}
#define main_input_sense legacy_main_input_sense
#include "th01/title_input.inc"
#undef main_input_sense
#include "th01/title_graphics.inc"
#define main_update_and_render legacy_main_update_and_render
#include "th01/title_menu.inc"
#undef main_update_and_render
#undef OP_INPUT_PREV
#undef OP_LOCAL
#undef OP_BGM_REF
 bool practice_visible=false;
 MenuEdges main_edges;
 unsigned main_count()const{return practice_visible?5:4;}
 void main_input_sense() {
  auto main_keys=keys;
  main_keys.strike=false;
  main_edges.poll(main_keys);input_ok=main_edges.accept;input_shot=false;input_cancel=main_edges.back;
  if(main_edges.move)menu_sel=(menu_sel+main_count()+main_edges.move)%main_count();
 }
 void main_update_and_render() {
  menu_unput(5);const char* all[]={"START","CONTINUE","BOSS PRAC","OPTION","QUIT"};
  const char* fresh[]={"START","CONTINUE","OPTION","QUIT"};const char** labels=practice_visible?all:fresh;
  unsigned count=main_count();
  for(unsigned i=0;i<count;++i){auto text=practice_wide(labels[i]);font.shift_jis(fb,0,316-int(std::string(labels[i]).size())*8,316-int(count)*10+i*20,48|(menu_sel==i?15:5),text);}
  if(input_ok||input_shot){
   if(menu_sel==0)start_game(true);else if(menu_sel==1)start_game(false);
   else if(practice_visible&&menu_sel==2)action=TitleAction::boss_practice;
   else if(menu_sel==(practice_visible?3:2))menu_id=MID_OPTION;else quit=true;
  }
  if(input_cancel)quit=true;
 }
 Impl(const FontRom& bitmap,std::filesystem::path directory,TitleHost callbacks,std::optional<GameOptions> remembered)
  :font(bitmap),profile(std::move(directory)),host(std::move(callbacks)) {
  if(!host.load||!host.present_wait||!host.delay_ms||!host.audio)throw std::invalid_argument("Title host callbacks required");
  practice_visible=BossProgress(profile).has_completion();
  cfg_load();if(remembered)opts=*remembered;bgm_mode_cur=opts.bgm_mode;
  if(!opts.bgm_mode)host.audio("bgm_stop","");title_init();
 }
 TitleAction step(InputKeys snapshot) {
  auto prior_options=opts;
  if(action!=TitleAction::menu)return action;
  keys=snapshot;
  bool any_key=keys.enter||keys.shot||(!hit_key&&keys.strike)||keys.escape||keys.up||keys.down||keys.left||keys.right||keys.num_up||keys.num_down||keys.num_left||keys.num_right;
  if(hit_key) {
   if(any_key) {
    title_window_put();rng=0;hit_key=false;release_after_hit_key=true;
    // Display the choices immediately, but consume the press that dismissed
    // HIT KEY. A held Z/Enter/A must not also confirm Start or Quit.
    input_ok=input_shot=input_cancel=false;main_update_and_render();
   }else title_hit_key_put(frame_rand++);
   frame_delay(1);return action;
  }
  if(release_after_hit_key) {
   if(any_key) {++frame_rand;frame_delay(1);return action;}
   release_after_hit_key=false;
  }
  if(menu_id==MID_MAIN) {main_input_sense();main_update_and_render();}
  else if(menu_id==MID_OPTION) {option_input_sense();option_update_and_render();}
  else if(menu_id==MID_MUSIC) {option_input_sense();music_update_and_render();}
  else if(menu_id==MID_UPDATE_BGM_MODE__DELAY__SWITCH_TO_MAIN) {
   if(opts.bgm_mode!=bgm_mode_cur) {
    mdrv2_bgm_stop();if(opts.bgm_mode) {mdrv2_bgm_load("reimu.mdt");mdrv2_bgm_play();}
    bgm_mode_cur=opts.bgm_mode;
   }
   frame_delay(15);menu_id=MID_MAIN;menu_sel=practice_visible?3:2;
  }else if(menu_id==MID_DELAY__SWITCH_TO_OPTION) {if(!opts.bgm_mode)mdrv2_bgm_stop();frame_delay(15);menu_id=MID_OPTION;}
  ++frame_rand;frame_delay(1);
  if(opts.rank!=prior_options.rank||opts.bgm_mode!=prior_options.bgm_mode||opts.credit_lives_extra!=prior_options.credit_lives_extra)cfg_save();
  if(prior_options.bgm_mode!=opts.bgm_mode) {
   mdrv2_bgm_stop();
   if(opts.bgm_mode){mdrv2_bgm_load("reimu.mdt");mdrv2_bgm_play();}
   bgm_mode_cur=opts.bgm_mode;
  }
  if(quit) {cfg_save();mdrv2_bgm_stop();action=TitleAction::quit;}
  return action;
 }
 int bgm_mode_cur=1;
};
TitleMenu::TitleMenu(const FontRom& font,std::filesystem::path profile,TitleHost host,std::optional<GameOptions> remembered)
 :impl(std::make_unique<Impl>(font,std::move(profile),std::move(host),remembered)) {}
TitleMenu::~TitleMenu()=default;
TitleAction TitleMenu::step(const InputKeys& keys) {return impl->step(keys);}
void TitleMenu::resume_menu() {
 impl->action=TitleAction::menu;
 impl->release_after_hit_key=true;
}
const Framebuffer& TitleMenu::framebuffer() const {return impl->fb;}
GameOptions TitleMenu::options() const {return impl->opts;}
unsigned TitleMenu::selection() const {return impl->menu_sel;}
unsigned TitleMenu::menu_id() const {return impl->menu_id;}
}

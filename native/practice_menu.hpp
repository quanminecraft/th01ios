#pragma once
#include "boss_practice.hpp"
#include "font_rom.hpp"
#include "input_backend.hpp"
#include <vector>
#include <algorithm>
namespace th01_native {
inline std::string practice_wide(const std::string& text) {
 std::string out;for(unsigned char c:text){if(c==' '){out+=char(0x81);out+=char(0x40);}else if(c>='A'&&c<='Z'){out+=char(0x82);out+=char(0x60+c-'A');}else {out+=char(c);}}return out;
}
inline void practice_box(Framebuffer& fb,int left,int top,int width,int height) {
 fb.text_overlay.clear();for(int y=top;y<top+height;++y)for(int x=left;x<left+width;++x)fb.put(0,x,y,(x==left||x==left+width-1||y==top||y==top+height-1)?7:0);
}
class MenuEdges {
 bool released=false,dir=false;
public:
 int move=0;bool accept=false,back=false;
 void reset(){released=false;dir=false;}
 void poll(const InputKeys& k) {
  move=0;accept=back=false;
  if(!released){if(!any_menu_input(k))released=true;return;}
  bool direction=k.up||k.down||k.left||k.right||k.num_up||k.num_down||k.num_left||k.num_right;
  if(direction&&!dir)move=(k.up||k.left||k.num_up||k.num_left)?-1:1;
  dir=direction;
  if(k.strike||k.escape){back=true;reset();}
  else if(k.shot||k.enter){accept=true;reset();}
 }
};
class PracticeChoiceMenu {
 Framebuffer background,fb;const FontRom& font;std::vector<std::string> choices;std::string heading;MenuEdges input;unsigned selected=0,cancel_selection;
 void draw(){fb=background;practice_box(fb,200,176,240,56+choices.size()*24);font.ascii(fb,0,216,188,15,heading);for(unsigned i=0;i<choices.size();++i)font.ascii(fb,0,224,216+i*24,i==selected?15:5,choices[i]);}
public:
 PracticeChoiceMenu(const Framebuffer& back,const FontRom& f,std::string title,std::vector<std::string> items,unsigned cancel=~0u):background(back),fb(back),font(f),choices(std::move(items)),heading(std::move(title)),cancel_selection(cancel==~0u?unsigned(choices.size()-1):cancel){draw();}
 std::optional<unsigned> step(const InputKeys& keys){input.poll(keys);if(input.move){selected=(selected+choices.size()+input.move)%choices.size();draw();}if(input.back)return cancel_selection;if(input.accept)return selected;return {};}
 const Framebuffer& framebuffer()const{return fb;}
};
class BossPracticeMenu {
 Framebuffer background,fb;const FontRom& font;const BossProgress& progress;MenuEdges input;
 unsigned page=0,mode_index=0,rank=1,boss=0;bool launch=false,cancel=false;
 void text(int x,int y,const std::string& s,unsigned color=15){font.ascii(fb,0,x,y,color,s);}
 void wide(int x,int y,const std::string& s,unsigned color=15){font.shift_jis(fb,0,x,y,48|color,practice_wide(s));}
 void draw(){
  fb=background;
  if(page==0){practice_box(fb,224,264,192,104);wide(248,276,"BOSS PRAC");wide(256,308,"STANDARD",mode_index==0?15:5);wide(248,336,"CHALLENGE",mode_index==1?15:5);}
  else if(page==1){practice_box(fb,224,224,192,144);wide(248,236,"BOSS PRAC");const char* names[]={"EASY","NORMAL","HARD","LUNATIC"};for(unsigned i=0;i<4;++i)wide(320-int(std::string(names[i]).size())*8,268+i*24,names[i],i==rank?15:5);}
  else {practice_box(fb,192,200,256,176);wide(248,212,"BOSS PRAC");const char* ranks[]={"EASY","NORMAL","HARD","LUNATIC"};text(208,240,std::string(mode_index?"CHALLENGE":"STANDARD")+" / "+ranks[rank]);
   unsigned top=boss<4?0:3;
   for(unsigned i=top;i<std::min(top+4,7u);++i){bool open=progress.unlocked(mode(),rank,BossId(i));wide(208,264+(i-top)*20,boss_entries()[i].name,open?(i==boss?15:5):8);if(i==boss)text(196,264+(i-top)*20,">");if(!open)text(400,264+(i-top)*20,"LOCK",8);}
   text(208,348,std::to_string(boss+1)+" / 7");
  }
  if(!progress.error().empty())text(208,360,progress.error(),7);
 }
public:
 BossPracticeMenu(const Framebuffer& back,const FontRom& f,const BossProgress& p,unsigned initial_rank):background(back),fb(back),font(f),progress(p),rank(std::min(initial_rank,3u)){draw();}
 void step(const InputKeys& keys){input.poll(keys);if(input.back){if(page)--page;else cancel=true;draw();return;}if(input.move){unsigned* value=page==0?&mode_index:page==1?&rank:&boss;unsigned count=page==0?2:page==1?4:7;*value=(*value+count+input.move)%count;draw();}if(input.accept){if(page<2)++page;else launch=progress.unlocked(mode(),rank,BossId(boss));draw();}}
 void resume(){launch=false;cancel=false;page=2;input.reset();draw();}
 bool ready()const{return launch;}bool cancelled()const{return cancel;}
 GameMode mode()const{return mode_index?GameMode::challenge:GameMode::standard;}
 unsigned difficulty()const{return rank;}BossId selected()const{return BossId(boss);}
 const Framebuffer& framebuffer()const{return fb;}
};
}

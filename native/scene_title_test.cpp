#include "text_animation.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
int main(int argc,char** argv) {try {
 using namespace th01_native;
 auto require=[](bool value,const char* reason) {if(!value)throw std::runtime_error(reason);};
 #include "entrance_jis.inc"
 Bytes bytes(0x46800);
 for(unsigned i=0;i<6;++i) {
  unsigned jis=entrance_jis[i];auto offset=0x1800+((jis>>8)-0x21)*96*32+((jis&255)-0x20)*32;
  bytes.at(offset+i)=0x80u>>i;
 }
 FontRom font(std::move(bytes));auto fb=std::make_unique<Framebuffer>();fb->clear(0,3);fb->clear(1,5);fb->palette.fill(0xff123456u);
 auto before=std::make_unique<decltype(fb->pages)>(fb->pages);
 SpriteSet sprites;ShotBackend shots{*fb,sprites,{}};ShotBackendBinding binding(shots);
 unsigned phase=0;
 scene_title_text_animate(*fb,font,[&](unsigned frames) {
  require(frames==8,"Each original entrance glyph lasts eight frames");
  require(std::count(fb->text_overlay.begin(),fb->text_overlay.end(),0u)==256,"Exactly one scaled source dot reveals graphics");
  require(fb->text_overlay[(80+phase*16)*640+192+phase*16]==0,"Original centered 16x glyph position");
  require(fb->text_overlay.front()==0xff000000u&&fb->text_overlay.back()==0xff000000u,"Reverse-black TRAM hides margins");
  ++phase;
 });
 require(phase==6&&fb->text_overlay.empty()&&fb->pages==*before,"All six glyphs finish without modifying graphics pages");
 require(shots.sound_commands==std::vector<unsigned>(6,14),"Original SE14 before each glyph");
 bool interrupted=false;try {scene_title_text_animate(*fb,font,[](unsigned){throw std::runtime_error("Interrupted");});}catch(const std::runtime_error&){interrupted=true;}
 require(interrupted&&fb->text_overlay.empty(),"Interrupted scene entrance clears text mask");
 if(argc>=2) {
  FontRom actual(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));auto background=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));fb->palette=background.palette;fb->blit(0,0,0,background.images[0],false);phase=0;
  scene_title_text_animate(*fb,actual,[&](unsigned) {if(++phase==3&&argc==3)dump_bmp(argv[2],*fb);});
 }
 std::cout<<"Original six-glyph scene entrance, reverse mask, 16x geometry, 48-frame timing, SE and cleanup passed\n";
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}

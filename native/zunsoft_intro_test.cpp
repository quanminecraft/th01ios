#include "zunsoft_intro.hpp"
#include "assets.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc<2)throw std::invalid_argument("Usage: zunsoft_intro_test ASSETS [DUMP_PREFIX]");
 auto data=read_file(std::filesystem::path(argv[1])/"TOUHOU.DAT");auto logo=read_zunsoft_logo(data);
 require(logo.images.size()==16&&logo.palette[1]==0xff000066u&&logo.palette[6]==0xffffff00u,"Original packed BFNT patterns and BRG palette");
 auto intro=std::make_unique<ZunSoftIntro>(data);unsigned steps=0;
 while(intro->step()){
  auto frame=intro->frames()-1;++steps;
  for(unsigned page=0;page<2;++page)for(auto [x,y]:{std::pair<int,int>{95,192},{544,192},{320,99},{320,300}})
   require(intro->framebuffer().pixel(page,x,y)==0,"Original logo viewport clips objects");
  if(frame==65)require(intro->logo_pattern()==6,"First logo animation keyframes");
  if(frame==110)require(intro->logo_pattern()==8,"Wave switches to second logo");
  if(frame==165)require(intro->logo_pattern()==14,"Final logo animation keyframes");
  if(argc>2&&(frame==70||frame==100||frame==130||frame==165))dump_bmp(std::string(argv[2])+"-"+std::to_string(frame)+".bmp",intro->framebuffer());
 }
 require(steps==231&&!intro->skipped(),"Original231 two-VSync logo frames");
 require(std::all_of(intro->framebuffer().palette.begin(),intro->framebuffer().palette.end(),[](auto c){return c==0xff000000u;}),"Intro fades to black before title music");
 ZunSoftIntro skip(data);InputKeys key;key.shot=true;require(!skip.step(key)&&skip.skipped(),"Controller confirm skips intro");
 data.resize(100);bool rejected=false;try{read_zunsoft_logo(data);}catch(const std::runtime_error&){rejected=true;}require(rejected,"Truncated logo rejected");
 std::cout<<"ZUN Soft assets, keyframes, viewport,231x2 timing, fade and skip passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

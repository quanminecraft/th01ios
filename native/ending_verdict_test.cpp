#include "ending_verdict.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "font_rom.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
#include <string>

namespace title_fixture {
using pixel_t=int;
using shiftjis_t=char;
template<std::size_t N> constexpr int shiftjis_w(const char (&)[N]) {return (N-1)*8;}
#include "ending_verdict_strings.inc"
}

int main(int argc,char** argv) {try {
 using namespace th01_native;
 if(argc<2||argc>3)throw std::invalid_argument("Usage: ending_verdict_test ASSET_DIR [DUMP_DIR]");
 std::filesystem::path dir=argv[1];
 auto image=read_grp(read_file(dir/"ENDM_A.GRP"));
 FontRom font(read_file(dir/"FONT.ROM"));
 auto require=[](bool condition,const char* message) {if(!condition)throw std::runtime_error(message);};
 for(bool makai:{false,true})for(bool timeout:{false,true}) {
  auto fb=std::make_unique<Framebuffer>();
  unsigned frames=0,polls=0,resets=0,randoms=0,colors=0,registrations=0,clears=0;
  std::vector<std::string> lines;
  EndingResult result;result.makai=makai;result.score=0xffffffffu;result.highest=12345678;
  result.continues=42;result.scene_continues={1,2,15,24};
  EndingPalette palette(*fb,[&](unsigned count) {frames+=count;});
  palette.load(image.palette);
  VerdictHost host;
  host.load=[&](const char* name) {require(std::string(name)=="endm_a.grp","Original verdict background");return image;};
  host.wait=[&](unsigned count) {frames+=count;};
  host.text=[&](int x,int y,int fx,const char* text) {
   font.shift_jis(*fb,0,x,y,fx,text);
   if(y==96||y==128||y==192||y==224||y==256||y==288||y==320)lines.emplace_back(text);
  };
  host.input=[&](bool reset) {
   if(reset) {
    ++resets;
    if(argc==3)dump_bmp(std::filesystem::path(argv[2])/(makai?"verdict-makai.bmp":"verdict-jigoku.bmp"),*fb);
   }else ++polls;
   return std::array<bool,2>{!reset&&!timeout&&polls==2,false};
  };
  host.random=[&]() {++randoms;return 0u;};
  host.registration_colors=[&]() {require(registrations==0&&palette.tone()==50,"Dim verdict before registration colors");++colors;};
  host.registration=[&](std::uint32_t value,int stage,std::uint16_t route) {
   require(colors==1&&clears==0,"Registration order");
   require(value==0xffffffffu&&stage==(makai?50:60)&&route==44682,"Original cleared-route score registration arguments");
   require(fb->pages[0]==fb->pages[1],"Verdict text removed before registration");++registrations;
  };
  host.resident_clear=[&]() {require(registrations==1,"Resident clear follows registration");++clears;};
  EndingVerdict verdict(*fb,palette,result,std::move(host));verdict.run();
  require(resets==1&&polls==(timeout?2000u:2u)&&randoms==1&&colors==1&&registrations==1&&clears==1,"Original input timeout and completion callbacks");
  require(lines.size()==7&&lines[0].find("12345678")!=std::string::npos&&lines[1].find("4294967295")!=std::string::npos,"DOS unsigned-long score formatting on native ABI");
  require(lines.back().substr(lines.back().size()-5)=="   42","Original Continue total formatting");
  std::cout<<(makai?"Makai":"Jigoku")<<(timeout?" timeout":" confirm")<<": frames="<<frames<<" polls="<<polls<<" registration passed\n";
 }
 // Hand-selected skill totals -30, 0, 20, 40, 60, 80 exercise all six
 // levels. A score with its sign bit set must not earn the positive bonus.
 for(unsigned group=0;group<3;++group)for(unsigned level=0;level<6;++level) {
  auto fb=std::make_unique<Framebuffer>();
  EndingResult result;result.score=0xffffffffu;
  result.rank=level==0?0:(level<4?1:level-2);
  result.extra_lives=level==0?4:2;
  result.continues=level<2?21:(level==2?1:0);
  std::string title;unsigned randoms=0;
  EndingPalette palette(*fb,[](unsigned) {});palette.load(image.palette);
  VerdictHost host;
  host.load=[&](const char*) {return image;};host.wait=[](unsigned) {};
  host.text=[&](int,int y,int fx,const char* text) {if(y==384&&fx==18)title=text;};
  host.input=[](bool) {return std::array<bool,2>{true,false};};
  host.random=[&]() {++randoms;return group+3u;};
  host.registration_colors=[]() {};host.registration=[](std::uint32_t,int,std::uint16_t) {};host.resident_clear=[]() {};
  EndingVerdict verdict(*fb,palette,result,std::move(host));verdict.run();
  require(randoms==1&&title==title_fixture::VERDICT_TITLES[group][level],"Original signed score, RNG group modulo and all six verdict levels");
 }
 std::cout<<"All 18 verdict titles and signed score comparison passed\n";
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}

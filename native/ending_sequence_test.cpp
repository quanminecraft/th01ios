#include "ending_sequence.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "font_rom.hpp"
#include "framebuffer_dump.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <memory>

int main(int argc,char** argv) {try {
 using namespace th01_native;
 if(argc<2||argc>3)throw std::invalid_argument("Usage: ending_sequence_test ASSET_DIR [DUMP_DIR]");
 std::filesystem::path dir=argv[1];
 FontRom font(read_file(dir/"FONT.ROM"));
 std::map<std::string,SpriteSet> images;
 auto require=[](bool value,const char* reason) {if(!value)throw std::runtime_error(reason);};
 for(bool challenge:{false,true})for(bool makai:{false,true})for(unsigned continues:{0u,1u}) {
  auto fb=std::make_unique<Framebuffer>();
  unsigned frames=0,typed=0,verdicts=0;
  std::vector<std::string> loads,music;
  std::string captions;
  EndingHost host;
  host.load=[&](const char* name) {
   std::string key=name;for(auto& c:key)c=std::toupper(static_cast<unsigned char>(c));
   loads.push_back(key);
   if(!images.count(key))images.emplace(key,read_grp(read_file(dir/key)));
   return images.at(key);
  };
  host.wait=[&](unsigned count) {frames+=count;};
  host.text=[&](int x,int y,int fx,const char* text) {
   require(x>=0&&x<640&&(y==316||y==348)&&fx==15,"Original caption coordinates/color");
   ++typed;captions+=text;font.shift_jis(*fb,0,x,y,fx,text);
  };
  host.audio=[&](const char* op,const char* name) {music.push_back(std::string(op)+":"+name);};
  host.verdict=[&]() {
   ++verdicts;
   if(argc==3)dump_bmp(std::filesystem::path(argv[2])/
    (std::string(makai?"makai":"jigoku")+(continues?"-bad.bmp":"-good.bmp")),*fb);
  };
  EndingSequence ending(*fb,std::move(host),makai,continues,challenge);
  ending.run();
  std::vector<std::string> expected={"ED1A.GRP","ED1B.GRP","ED1C.GRP","ED1D.GRP","ED1E.GRP"};
  if(continues) {
   expected.push_back("ED2A.GRP");if(makai)expected.push_back("ED4A.GRP");
  }else {
   expected.push_back("ED3A.GRP");
   if(makai)for(auto name:{"ED5A.GRP","ED5B.GRP","ED5C.GRP"})expected.push_back(name);
   else expected.push_back("ED3B.GRP");
   expected.push_back(makai?"ENDB_A.GRP":"ENDB_B.GRP");
  }
  require(loads==expected,"Original good/bad route asset order");
  require(verdicts==1&&fb->display_scroll_y==0,"One verdict handoff and restored scroll");
  require(music==std::vector<std::string>({"load:iris.mdt","play:","fade:","stop:","load:st1.mdt","play:"}),"Original music command order");
  require(captions.find(continues?"Bad Ending":"Good Ending")!=std::string::npos,"Good/bad caption");
  if(challenge&&continues) {
   require(captions.find("Try 10 misses or less!")!=std::string::npos,"Challenge bad ending explains adjusted miss requirement");
   require(captions.find("Challenge - Bad Ending")!=std::string::npos&&captions.find("No continue")==std::string::npos,"Separate Challenge caption, no Standard continue requirement");
  }else require(captions.find("Challenge - Bad Ending")==std::string::npos,"Standard and good endings retain original captions");
  require((captions.find("STAGE 20 BOSS")!=std::string::npos)==(continues==0),"Slideshow only for no-Continue ending");
  require(frames>2000&&typed>35,"Ending frame waits and typed captions executed");
  std::cout<<(makai?"Makai":"Jigoku")<<(continues?" bad":" good")<<": frames="<<frames<<" typed="<<typed<<" pictures="<<loads.size()<<'\n';
 }
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}

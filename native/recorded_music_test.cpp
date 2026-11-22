#include "recorded_music.hpp"
#include <iostream>
#include <array>
#include <stdexcept>
using namespace th01_native;
int main(int argc,char** argv) {
 try {
  if(argc!=3)throw std::runtime_error("assets and recordings required");
  RecordedMusic music(argv[2],argv[1]);std::array<std::int16_t,2048> out{};
  for(auto* stem:{"REIMU","ST0","ST6","POSITIVE","ST1","ST2","LEGEND","ST3","ST4","KAMI","KAMI2","ST5","TENSI","SYUGEN","SHUGEN","ALICE","IRIS","ZIPANGU"}) {
   music.command("bgm_load",std::string(stem)+".MDT");music.command("bgm_play",{});
   bool audible=false;for(unsigned i=0;i<94;++i){music.render(out.data(),1024);audible|=std::any_of(out.begin(),out.end(),[](auto v){return v!=0;});}
   if(!audible)throw std::runtime_error(std::string("Silent recording: ")+stem);
   music.command("bgm_stop",{});music.render(out.data(),1024);if(std::any_of(out.begin(),out.end(),[](auto v){return v!=0;}))throw std::runtime_error("Stop leaked BGM");
  }
  music.command("bgm_load","REIMU.MDT");music.command("bgm_play",{});
  for(unsigned i=0;i<12000;++i)music.render(out.data(),1024); // Cross the recording loop twice.
  if(std::none_of(out.begin(),out.end(),[](auto v){return v!=0;}))throw std::runtime_error("Loop stopped");
  music.command("bgm_fade",{});for(unsigned i=0;i<1000;++i)music.render(out.data(),1024);
  if(std::any_of(out.begin(),out.end(),[](auto v){return v!=0;}))throw std::runtime_error("Fade did not stop");
  music.command("bgm_load","INIT.MDT");music.command("bgm_play",{});music.render(out.data(),1024);
  if(std::any_of(out.begin(),out.end(),[](auto v){return v!=0;}))throw std::runtime_error("INIT not silent");
  std::cout<<"All recordings, aliases, loop, fade, stop and INIT passed\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

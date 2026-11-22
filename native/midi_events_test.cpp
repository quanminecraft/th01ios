#include "mdt_player.hpp"
#include "assets.hpp"
#include <iostream>
using namespace th01_native;
int main(int argc,char** argv) {try {
 if(argc!=2)throw std::invalid_argument("Usage: midi_events_test ASSET_DIR");
 // A tied C1->D1 phrase must keep one MIDI voice, bend it, then release
 // one tick before the final rest, matching MDRV98 gate dispatch at 0x1bdc.
 // This fixture does not depend on the production song data.
 Bytes fixture{2,3,1,0,1,0,16,0,0x80,0,0,0,0,0,0,0};
 fixture.insert(fixture.end(),{0xeb,0,0xea,8,0x00,4,0x91,0x02,4,0x90,4,0xff});
 unsigned bank=fixture.size();fixture[10]=bank;fixture[12]=bank+32;fixture.resize(bank+32);
 fixture[bank+3]=0x78;
 bool stereo=false;
 MdtPlayer tied(fixture,[&](unsigned reg,unsigned value){
  if(reg==0xb4&&value==0xc0)stereo=true;
  if(reg==0x28&&(value&0xf0)&&!stereo)throw std::runtime_error("FM note starts with muted chip-reset routing");
 },[](unsigned){});
 unsigned on=0,off=0,on_tick=999,off_tick=999,tick=0;bool bent=false;
 tied.set_midi([&](unsigned s,unsigned a,unsigned b){
  if(s==0x90){if(a!=24)throw std::runtime_error("C1 MIDI octave wrong");++on;on_tick=tick;}
  if(s==0x80){if(a!=24)throw std::runtime_error("Tie changed voice identity");++off;off_tick=tick;}
  if(s==0xe0&&tick==4&&(a|(b<<7))>9500)bent=true;
 });
 while(!tied.finished()&&tick<32){tied.tick();++tick;}
 if(on!=1||off!=1||on_tick!=0||off_tick!=7||!bent)throw std::runtime_error("MIDI tie/gate/bend timing incorrect");
 unsigned files=0;
 for(auto& entry:std::filesystem::directory_iterator(argv[1])) {
  if(entry.path().extension()!=".MDT")continue;
  auto bytes=read_file(entry.path());std::uint64_t hash[2]={0,0},count[2]={0,0};
  unsigned noteons=0,bends=0,offs=0;
  auto write=[&](unsigned i,unsigned reg,unsigned value){hash[i]=(hash[i]*1099511628211ull)^(reg<<8)^value;++count[i];};
  MdtPlayer original(bytes,[&](unsigned r,unsigned v){write(0,r,v);},[](unsigned){});
  MdtPlayer observed(bytes,[&](unsigned r,unsigned v){write(1,r,v);},[](unsigned){});
  observed.set_midi([&](unsigned s,unsigned a,unsigned b){
   if(s<0x80||s>=0xf0||a>127||b>127)throw std::runtime_error("MIDI range error");
   if((s&240)==0x90) {if(!b)throw std::runtime_error("Zero-velocity note-on");++noteons;}
   if((s&240)==0x80)++offs;
   if((s&240)==0xe0)++bends;
  });
  for(unsigned i=0;i<15000&&!original.finished();++i) {
   original.tick();observed.tick();
   if(hash[0]!=hash[1]||count[0]!=count[1]||original.timer_period()!=observed.timer_period()||original.notes()!=observed.notes()||original.loops()!=observed.loops())
    throw std::runtime_error("MIDI observer altered original sequencer");
  }
  if(entry.path().stem()!="INIT"&&(!noteons||!offs||!bends))throw std::runtime_error("Missing melodic MIDI events");
  std::cout<<entry.path().filename()<<" on="<<noteons<<" off="<<offs<<" bends="<<bends<<'\n';++files;
 }
 if(files!=19)throw std::runtime_error("Expected all 19 MDT inputs");
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

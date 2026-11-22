#include "mdt_player.hpp"
#include "assets.hpp"
#include <fstream>
#include <iostream>
using namespace th01_native;
static void be(Bytes& out,unsigned value,unsigned size) {for(unsigned i=size;i;--i)out.push_back(value>>((i-1)*8));}
static void vlq(Bytes& out,std::uint64_t value) {
 if(value>0x0fffffff)throw std::runtime_error("MIDI delta exceeds SMF range");
 unsigned data[4],n=0;data[n++]=value&127;
 while(value>>=7)data[n++]=(value&127)|128;
 while(n)out.push_back(data[--n]);
}
int main(int argc,char** argv) {try {
 if(argc<3||argc>4)throw std::invalid_argument("Usage: midi_export INPUT.MDT OUTPUT.mid [SECONDS=120]");
 unsigned seconds=argc==4?std::stoul(argv[3]):120;
 if(seconds==0||seconds>3600)throw std::invalid_argument("MIDI duration must be 1..3600 seconds");
 Bytes track{0,0xff,0x51,3,7,0xa1,0x20}; // 500000 us/quarter, 480 PPQ
 std::uint64_t clocks=0,last=0,events=0;
 MdtPlayer player(read_file(argv[1]),[](unsigned,unsigned){},[](unsigned){});
 auto message=[&](unsigned status,unsigned a,unsigned b) {
  if(status<0x80||status>=0xf0||a>127||b>127)throw std::runtime_error("Invalid MIDI event");
  auto time=(clocks*960+83200)/166400;vlq(track,time-last);last=time;
  track.push_back(status);track.push_back(a);if((status&0xf0)!=0xc0)track.push_back(b);++events;
 };
 player.set_midi(message);
 while(clocks<std::uint64_t(seconds)*166400&&!player.finished()) {player.tick();clocks+=player.timer_period()*48;}
 for(unsigned c=0;c<16;++c)message(0xb0|c,123,0);
 vlq(track,0);track.insert(track.end(),{0xff,0x2f,0});
 Bytes file{'M','T','h','d'};be(file,6,4);be(file,0,2);be(file,1,2);be(file,480,2);
 file.insert(file.end(),{'M','T','r','k'});be(file,track.size(),4);file.insert(file.end(),track.begin(),track.end());
 std::ofstream output(argv[2],std::ios::binary);output.write(reinterpret_cast<const char*>(file.data()),file.size());
 if(!output)throw std::runtime_error("Cannot write MIDI file");
 std::cout<<"MIDI events="<<events<<" notes="<<player.notes()<<" loops="<<player.loops()<<" bytes="<<file.size()<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

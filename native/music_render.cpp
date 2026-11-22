#include "music_synth.hpp"
#include <fstream>
#include <iostream>
#include <vector>
int main(int argc,char** argv) {try {
 using namespace th01_native;
 if(argc<4||argc>6)throw std::invalid_argument("Usage: music_render ASSETS MDT OUTPUT.wav [SECONDS] [MASK: FM=1 SSG=2 RHYTHM=4 ALL=7]");
 unsigned seconds=argc>=5?std::stoul(argv[4]):6;
 if(seconds<1||seconds>600)throw std::invalid_argument("Audio duration outside range");
 MusicSynth music(argv[1],argc==6?std::stoul(argv[5]):7);music.command("bgm_load",argv[2]);music.command("bgm_play");
 std::vector<std::int16_t> samples(seconds*48000*2);music.render(samples.data(),seconds*48000);
 std::uint64_t energy=0,hash=1469598103934665603ull;unsigned peak=0;
 for(auto value:samples){unsigned magnitude=value<0?-int(value):value;peak=std::max(peak,magnitude);energy+=magnitude;hash^=std::uint16_t(value);hash*=1099511628211ull;}
 if(!energy||!music.notes())throw std::runtime_error("MDT rendered silence");
 std::ofstream out(argv[3],std::ios::binary|std::ios::trunc);
 auto le=[&](std::uint32_t value,unsigned n) {for(unsigned i=0;i<n;++i)out.put(char(value>>(i*8)));};
 unsigned size=samples.size()*2;
 out.write("RIFF",4);le(size+36,4);out.write("WAVEfmt ",8);le(16,4);le(1,2);le(2,2);le(48000,4);le(192000,4);le(4,2);le(16,2);out.write("data",4);le(size,4);
 for(auto value:samples)le(std::uint16_t(value),2);
 if(!out)throw std::runtime_error("Cannot write rendered audio");
 music.command("bgm_stop");std::int16_t silence[128];music.render(silence,64);for(auto value:silence)if(value)throw std::runtime_error("Music stop did not silence output");
 std::cout<<"Native MDT FM/SSG/rhythm: peak="<<peak<<" energy="<<energy<<" PCM hash="<<std::hex<<hash<<'\n';
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}

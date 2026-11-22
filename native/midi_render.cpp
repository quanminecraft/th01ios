#include "sdl_midi.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
int main(int argc,char** argv) {try {
 using namespace th01_native;
 if(argc<5||argc>6)throw std::invalid_argument("Usage: midi_render ASSETS MDT SOUNDFONT OUTPUT.wav [SECONDS=6]");
 unsigned seconds=argc==6?std::stoul(argv[5]):6;
 if(seconds<1||seconds>600)throw std::invalid_argument("Invalid duration");
 SdlMidi music(argv[1],argv[3]);music.command("bgm_load",argv[2]);music.command("bgm_play","");
 auto start=std::chrono::steady_clock::now();
 std::vector<std::int16_t> samples(seconds*48000*2);music.render(samples.data(),seconds*48000);
 auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
 std::uint64_t energy=0;unsigned peak=0,clipped=0;
 for(auto value:samples){unsigned magnitude=value<0?-int(value):value;peak=std::max(peak,magnitude);energy+=magnitude;clipped+=magnitude>=32767;}
 if(!energy||clipped)throw std::runtime_error("MIDI output silent or clipping");
 std::ofstream out(argv[4],std::ios::binary);
 auto le=[&](std::uint32_t v,unsigned n){for(unsigned i=0;i<n;++i)out.put(char(v>>(i*8)));};
 unsigned size=samples.size()*2;out.write("RIFF",4);le(size+36,4);out.write("WAVEfmt ",8);le(16,4);le(1,2);le(2,2);le(48000,4);le(192000,4);le(4,2);le(16,2);out.write("data",4);le(size,4);
 for(auto value:samples)le(std::uint16_t(value),2);
 if(!out)throw std::runtime_error("Cannot write MIDI WAV");
 music.command("bgm_stop","");std::int16_t tail[128];music.render(tail,64);for(auto value:tail)if(value)throw std::runtime_error("MIDI stop failed");
 music.command("bgm_play","");music.command("bgm_fade","");music.render(samples.data(),samples.size()/2);
 std::cout<<"MIDI peak="<<peak<<" energy="<<energy<<" render_seconds="<<elapsed<<" audio_seconds="<<seconds<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

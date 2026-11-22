#pragma once
#include "mdt_player.hpp"
#include <SDL.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <vector>
namespace th01_native {
// One decoded recording at a time. The callback performs no file IO or allocation.
class RecordedMusic {
 std::filesystem::path directory,assets;
 std::vector<std::int16_t> pcm;
 Bytes score;
 std::unique_ptr<MdtPlayer> clock;
 std::uint64_t position=0,loop_start=0,loop_end=0;
 unsigned rate=44100,tick_left=0,remainder=0,fade=0,fade_clock=0;
 bool active=false,fading=false;
public:
 RecordedMusic(std::filesystem::path dir,std::filesystem::path data):directory(std::move(dir)),assets(std::move(data)) {}
 unsigned timer_period() const {return clock?clock->timer_period():57;}
 void command(const std::string& op,const std::string& name) {
  if(op=="bgm_load") {
   active=false;clock.reset();pcm.clear();score.clear();
   auto stem=std::filesystem::path(name).stem().string();for(auto& c:stem)c=std::toupper(static_cast<unsigned char>(c));
   if(stem=="INIT")return;
   auto original=stem;if(stem=="ST6")stem="ST0";if(stem=="SHUGEN")stem="SYUGEN";
   SDL_AudioSpec spec{};Uint8* bytes=nullptr;Uint32 length=0;
   auto file=directory/(stem+".wav");
   if(!SDL_LoadWAV(file.string().c_str(),&spec,&bytes,&length))throw std::runtime_error("Cannot load recorded music: "+file.string()+": "+SDL_GetError());
   if(spec.format!=AUDIO_S16LSB||spec.channels!=2||spec.freq!=44100) {SDL_FreeWAV(bytes);throw std::runtime_error("Recorded music must be stereo PCM16 44100 Hz");}
   loop_start=0;loop_end=length/4;
   std::ifstream points(directory/(stem+".loop"));std::uint64_t begin,end;
   if(!(points>>begin>>end)||begin>=end||end>loop_end) {SDL_FreeWAV(bytes);throw std::runtime_error("Missing or invalid recorded music loop: "+stem);}
   loop_start=begin;loop_end=end;rate=spec.freq;
   pcm.resize(loop_end*2);std::memcpy(pcm.data(),bytes,pcm.size()*2);SDL_FreeWAV(bytes);
   std::ifstream input(assets/(original+".MDT"),std::ios::binary);score.assign(std::istreambuf_iterator<char>(input),{});
   if(score.empty())throw std::runtime_error("Missing MDT timing for recorded music: "+original);
  }else if(op=="bgm_play") {
   position=0;tick_left=remainder=fade=fade_clock=0;fading=false;active=!pcm.empty()&&!score.empty();
   if(active)clock=std::make_unique<MdtPlayer>(score,[](unsigned,unsigned){},[](unsigned){});
  }else if(op=="bgm_stop") {active=false;clock.reset();}
  else if(op=="bgm_fade") {fading=active;fade_clock=0;}
  else throw std::runtime_error("Unknown recorded music command: "+op);
 }
 void render(std::int16_t* output,unsigned frames) {
  for(unsigned f=0;f<frames;++f) {
   if(!active){output[f*2]=output[f*2+1]=0;continue;}
   if(!tick_left) {
    clock->tick();unsigned numerator=timer_period()*180+remainder;tick_left=std::max(1u,numerator/13);remainder=numerator%13;
    if(fading&&++fade_clock==2){fade_clock=0;if(++fade>=127){active=false;output[f*2]=output[f*2+1]=0;continue;}}
   }
   --tick_left;
   auto index=position/48000;auto fraction=position%48000;auto next=index+1<loop_end?index+1:loop_start;
   // Linear interpolation retains the recording's pitch at the 48kHz output rate.
   for(unsigned c=0;c<2;++c){std::int64_t sample=(std::int64_t(pcm[index*2+c])*(48000-fraction)+std::int64_t(pcm[next*2+c])*fraction)/48000;output[f*2+c]=sample*(127-fade)/254;}
   position+=rate;if(position>=loop_end*48000)position-=(loop_end-loop_start)*48000;
  }
 }
};
}

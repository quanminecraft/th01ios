#include "music_synth.hpp"
#include "mdt_player.hpp"
#include "assets.hpp"
#include <iostream>
int main(int argc,char** argv) {try {
 using namespace th01_native;
 if(argc!=2)throw std::invalid_argument("Usage: effects_test ASSET_DIR");
 auto data=read_file(std::filesystem::path(argv[1])/"ZIGOKU.MDE");MusicSynth audio(argv[1]);
 std::vector<std::int16_t> pcm(3*48000*2);
 for(unsigned id=1;id<=17;++id) {
  unsigned writes=0,ticks=0,channels=0;
  MdtPlayer effect(data,id,[&](unsigned reg,unsigned value) {if(reg>=16||value>255)throw std::runtime_error("MDE wrote invalid SSG register");if(reg>=8&&reg<=10)channels|=1u<<(reg-8);++writes;});
  while(!effect.finished()&&ticks<5000) {effect.tick();++ticks;}
  if(!effect.finished()||!effect.notes()||!writes)throw std::runtime_error("MDE stream did not finish: "+std::to_string(id));
  if(channels!=(id==9?6u:4u))throw std::runtime_error("MDE must override SSG C then B, preserving other music channels");
  audio.command("bgm_stop");audio.effect(id);audio.render(pcm.data(),pcm.size()/2);
  std::uint64_t energy=0;unsigned clipped=0;for(auto value:pcm){energy+=value<0?-int(value):value;clipped+=value==32767||value==-32768;}
  unsigned first=0;while(first<pcm.size()&&pcm[first]==0)++first;
  if((id==8||id==9)&&first/2>2400)throw std::runtime_error("Bomb SE onset exceeds 50 ms");
  if(!energy||clipped)throw std::runtime_error("MDE output silent or clipping: "+std::to_string(id));
  std::cout<<"SE"<<id<<" ticks="<<ticks<<" notes="<<effect.notes()<<" energy="<<energy<<'\n';
 }
 MusicSynth reference(argv[1]);audio.command("bgm_load","REIMU.MDT");audio.command("bgm_play");reference.command("bgm_load","REIMU.MDT");reference.command("bgm_play");
 audio.render(pcm.data(),48000);reference.render(pcm.data(),48000);audio.effect(9);
 audio.render(pcm.data(),48000);reference.render(pcm.data(),48000);
 if(audio.notes()!=reference.notes()||!audio.playing())throw std::runtime_error("SE restarted or stopped background music");
 bool invalid=false;try {audio.effect(18);}catch(const std::invalid_argument&){invalid=true;}
 if(!invalid)throw std::runtime_error("Invalid SE ID accepted");
 // Sariel's form change repeats SE9 every four frames while TENSI fades.
 // The transition lowers only the effect, and the next song restores defaults.
 MusicSynth before(argv[1]),after(argv[1]);
 for(auto* synth:{&before,&after}) {
  synth->command("bgm_load","TENSI.MDT");synth->command("bgm_play");
 }
 std::vector<std::int16_t> a(48000*2),b(a.size());
 before.render(a.data(),48000);after.render(b.data(),48000);
 if(a!=b)throw std::runtime_error("Ordinary music changed before Sariel transition");
 before.command("bgm_fade");after.command("se_transition");after.command("bgm_fade");
 long double old_energy=0,new_energy=0;
 for(unsigned frame=0;frame<200;++frame) {
  if(frame && frame%4==0) {before.effect(9);after.effect(9);}
  before.render(a.data(),800);after.render(b.data(),800);
  for(unsigned i=0;i<1600;++i) {
   old_energy+=static_cast<long double>(a[i])*a[i];
   new_energy+=static_cast<long double>(b[i])*b[i];
  }
 }
 if(!old_energy || new_energy>=old_energy*0.9L)
  throw std::runtime_error("Sariel repeated explosion is not quieter");
 for(auto* synth:{&before,&after}) {
  synth->command("bgm_load","SYUGEN.MDT");synth->command("bgm_play");synth->effect(9);
 }
 before.render(a.data(),48000);after.render(b.data(),48000);
 if(a!=b)throw std::runtime_error("Sariel transition attenuation leaked into next song/effect");
 std::cout<<"Sariel transition energy ratio="<<double(new_energy/old_energy)<<" with normal next-song PCM restored\n";
 std::cout<<"All original MDE effects and continuing BGM passed\n";
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}

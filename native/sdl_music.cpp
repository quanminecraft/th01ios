#include "sdl_music.hpp"
#include <cstring>
#include <array>
#include <algorithm>
#include <iostream>
#include "audio_mix.hpp"
namespace th01_native {
SdlMusic::SdlMusic(const std::filesystem::path& assets):synth(assets) {
 if(auto* value=SDL_getenv("TH01_AUDIO_MONO"))mono=std::string(value)=="1";
 if(auto* dir=SDL_getenv("TH01_WAV_DIR");dir&&*dir){recorded=std::make_unique<RecordedMusic>(dir,assets);std::clog<<"BGM: recorded WAV, "<<dir<<"; original MDE effects; mono="<<mono<<"\n";}
 if(!recorded)if(auto* sf=SDL_getenv("TH01_MIDI_SOUNDFONT");sf&&*sf)midi=std::make_unique<SdlMidi>(assets,sf);
 SDL_AudioSpec desired{};desired.freq=48000;desired.format=AUDIO_S16SYS;desired.channels=2;
 desired.samples=1024;desired.callback=callback;desired.userdata=this;
 device=SDL_OpenAudioDevice(nullptr,0,&desired,nullptr,0);
 if(!device)throw std::runtime_error(std::string("Cannot open native music output: ")+SDL_GetError());
 SDL_PauseAudioDevice(device,0);
}
SdlMusic::~SdlMusic() {if(device){SDL_PauseAudioDevice(device,1);SDL_CloseAudioDevice(device);}}
void SdlMusic::callback(void* userdata,Uint8* output,int length) {
 auto& self=*static_cast<SdlMusic*>(userdata);
 try {
  if(!self.error.empty()) {std::memset(output,0,length);return;}
  auto* pcm=reinterpret_cast<std::int16_t*>(output);unsigned remaining=length/4;
  if(!self.midi&&!self.recorded)self.synth.render(pcm,remaining);
  else {
   std::array<std::int16_t,512> effects;
   while(remaining) {
    unsigned n=std::min(remaining,256u);
    if(self.recorded)self.recorded->render(pcm,n);else self.midi->render(pcm,n);
    self.synth.set_effect_timer_period(self.recorded?self.recorded->timer_period():self.midi->timer_period());self.synth.render(effects.data(),n);
    for(unsigned i=0;i<n*2;++i)pcm[i]=std::clamp(int(pcm[i])+int(effects[i]),-32768,32767);
    pcm+=n*2;remaining-=n;
   }
  }
  if(self.mono) {
   stereo_to_mono(reinterpret_cast<std::int16_t*>(output),length/4);
  }
  self.frames+=length/4;
 }catch(const std::exception& e) {self.error=e.what();std::memset(output,0,length);}
}
void SdlMusic::command(const std::string& operation,const std::string& asset) {
 SDL_LockAudioDevice(device);
 try {
  if(operation=="bgm_load"||operation=="bgm_stop")synth.command("se_transition_end");
  if(operation=="se_transition")synth.command(operation,asset);
  else if(recorded&&operation!="se_load") recorded->command(operation,asset);
  else if(midi&&operation!="se_load") {
   midi->command(operation,asset);
   if(operation=="bgm_load"||operation=="bgm_stop")synth.command("bgm_stop");
  }else synth.command(operation,asset);
 }catch(...) {SDL_UnlockAudioDevice(device);throw;}
 SDL_UnlockAudioDevice(device);
}
void SdlMusic::check() {
 SDL_LockAudioDevice(device);auto message=error;SDL_UnlockAudioDevice(device);
 if(!message.empty())throw std::runtime_error("Native MDT playback: "+message);
}
void SdlMusic::effect(unsigned id) {
 SDL_LockAudioDevice(device);
 try {synth.effect(id);}catch(...) {SDL_UnlockAudioDevice(device);throw;}
 SDL_UnlockAudioDevice(device);
}
}

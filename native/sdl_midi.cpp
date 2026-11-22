#include "sdl_midi.hpp"
#include "mdt_player.hpp"
#include "assets.hpp"
#include <SDL.h>
#include <algorithm>
#include <cctype>
#include <iostream>
namespace th01_native {
struct SdlMidi::Impl {
 void* library=nullptr;
 void* settings=nullptr;
 void* synth=nullptr;
 void* (*new_settings)()=nullptr;
 void (*delete_settings)(void*)=nullptr;
 int (*setnum)(void*,const char*,double)=nullptr;
 int (*setint)(void*,const char*,int)=nullptr;
 void* (*new_synth)(void*)=nullptr;
 void (*delete_synth)(void*)=nullptr;
 int (*sfload)(void*,const char*,int)=nullptr;
 int (*noteon)(void*,int,int,int)=nullptr;
 int (*noteoff)(void*,int,int)=nullptr;
 int (*cc)(void*,int,int,int)=nullptr;
 int (*program)(void*,int,int)=nullptr;
 int (*bend)(void*,int,int)=nullptr;
 int (*system_reset)(void*)=nullptr;
 void (*gain)(void*,float)=nullptr;
 int (*write)(void*,int,void*,int,int,void*,int,int)=nullptr;
 std::filesystem::path assets;
 Bytes loaded;
 std::unique_ptr<MdtPlayer> song;
 unsigned left=0,remainder=0,fade=0,fade_clock=0;
 bool active=false,fading=false;
 template<class T> void bind(T& target,const char* name) {
  target=reinterpret_cast<T>(SDL_LoadFunction(library,name));
  if(!target)throw std::runtime_error(std::string("FluidSynth symbol missing: ")+name);
 }
 void cleanup() {
  if(synth&&delete_synth)delete_synth(synth);
  if(settings&&delete_settings)delete_settings(settings);
  if(library)SDL_UnloadObject(library);
  synth=settings=library=nullptr;
 }
 Impl(const std::filesystem::path& root,const std::filesystem::path& sf):assets(root) {
  try {
   for(auto name:{"libfluidsynth.so.3","libfluidsynth.so.1","libfluidsynth-3.dll","libfluidsynth-2.dll"}) {
    library=SDL_LoadObject(name);if(library)break;
   }
   if(!library)throw std::runtime_error(std::string("Cannot load FluidSynth: ")+SDL_GetError());
   bind(new_settings,"new_fluid_settings");bind(delete_settings,"delete_fluid_settings");
   bind(setnum,"fluid_settings_setnum");bind(setint,"fluid_settings_setint");
   bind(new_synth,"new_fluid_synth");bind(delete_synth,"delete_fluid_synth");
   bind(sfload,"fluid_synth_sfload");bind(noteon,"fluid_synth_noteon");
   bind(noteoff,"fluid_synth_noteoff");bind(cc,"fluid_synth_cc");
   bind(program,"fluid_synth_program_change");bind(bend,"fluid_synth_pitch_bend");
   bind(system_reset,"fluid_synth_system_reset");bind(gain,"fluid_synth_set_gain");
   bind(write,"fluid_synth_write_s16");
   settings=new_settings();if(!settings)throw std::runtime_error("Cannot allocate MIDI settings");
   setnum(settings,"synth.sample-rate",48000);setnum(settings,"synth.gain",0.35);
   setint(settings,"synth.reverb.active",0);
   setint(settings,"synth.chorus.active",0);setint(settings,"synth.polyphony",64);
   setint(settings,"synth.cpu-cores",1);setint(settings,"synth.threadsafe-api",0);
   synth=new_synth(settings);if(!synth)throw std::runtime_error("Cannot allocate MIDI synth");
   if(sfload(synth,sf.string().c_str(),1)<0)throw std::runtime_error("Cannot load MIDI soundfont: "+sf.string());
   std::cout<<"MIDI: original MDT events -> FluidSynth / "<<sf.string()<<'\n';
  }catch(...) {cleanup();throw;}
 }
 ~Impl() {cleanup();}
 void message(unsigned status,unsigned a,unsigned b) {
  unsigned channel=status&15;
  switch(status&240) {
   case 0x80:noteoff(synth,channel,a);break;
   case 0x90:noteon(synth,channel,a,b);break;
   case 0xb0:cc(synth,channel,a,b);break;
   case 0xc0:program(synth,channel,a);break;
   case 0xe0:bend(synth,channel,a|(b<<7));break;
  }
 }
 void stop() {active=fading=false;song.reset();left=remainder=fade=fade_clock=0;system_reset(synth);gain(synth,0.35f);}
 void command(const std::string& op,const std::string& name) {
  if(op=="bgm_load") {
   stop();std::string filename=name;for(auto& c:filename)c=std::toupper(static_cast<unsigned char>(c));
   loaded=read_file(assets/filename);
  }else if(op=="bgm_play") {
   stop();if(loaded.empty())throw std::runtime_error("MIDI play without MDT data");
   song=std::make_unique<MdtPlayer>(loaded,[](unsigned,unsigned){},[](unsigned){});
   song->set_midi([this](unsigned s,unsigned a,unsigned b){message(s,a,b);});active=true;
  }else if(op=="bgm_stop")stop();
  else if(op=="bgm_fade") {fading=active;fade_clock=0;}
  else throw std::runtime_error("Unknown MIDI command: "+op);
 }
 void render(std::int16_t* out,unsigned frames) {
  while(frames) {
   if(!active) {std::fill(out,out+frames*2,0);return;}
   if(!left) {
    song->tick();unsigned numerator=song->timer_period()*180+remainder;
    left=numerator/13;remainder=numerator%13;
    if(fading&&++fade_clock==2) {fade_clock=0;gain(synth,0.35f*(127-std::min(++fade,127u))/127);if(fade==127){stop();continue;}}
   }
   unsigned n=std::min(frames,left);
   if(write(synth,n,out,0,2,out,1,2)<0)throw std::runtime_error("MIDI render failed");
   out+=n*2;frames-=n;left-=n;
  }
 }
};
SdlMidi::SdlMidi(const std::filesystem::path& a,const std::filesystem::path& sf):impl(std::make_unique<Impl>(a,sf)) {}
SdlMidi::~SdlMidi()=default;
void SdlMidi::command(const std::string& op,const std::string& name) {impl->command(op,name);}
void SdlMidi::render(std::int16_t* out,unsigned n) {impl->render(out,n);}
unsigned SdlMidi::timer_period() const {return impl->song?impl->song->timer_period():62;}
}

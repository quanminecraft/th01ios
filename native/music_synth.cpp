#include "music_synth.hpp"
#include "mdt_player.hpp"
#include "assets.hpp"
#include "third_party/ymfm/ymfm_opn.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <map>

namespace th01_native {
struct MusicSynth::Impl {
 ymfm::ymfm_interface interface;
 ymfm::ym2608 chip{interface};
 std::map<std::string,Bytes> files;
 std::array<std::vector<std::int16_t>,6> drums;
 std::array<std::size_t,6> position{};
 std::array<unsigned,6> drum_control{};
 unsigned drum_level=63,phase=0,tick_left=0,fade=0,fade_clock=0;
 std::string loaded;
 std::unique_ptr<MdtPlayer> song;
 std::unique_ptr<MdtPlayer> effect_player;
 Bytes effect_data;
 unsigned effect_mask=0,last_period=62,output_mask=7;
 std::array<unsigned,16> music_regs{},effect_regs{};
 bool active=false,fading=false,transition_effects=false;
 std::int64_t dc_ssg=0;
 static std::string uppercase(std::string value) {for(auto& c:value)c=std::toupper(static_cast<unsigned char>(c));return value;}
 void write(unsigned reg,unsigned value) {
  if(reg==0x11)drum_level=value&63;
  if(reg>=0x18&&reg<0x1e)drum_control[reg-0x18]=value;
  chip.write((reg>=0x100)?2:0,reg&255);chip.write((reg>=0x100)?3:1,value);
 }
 void rhythm(unsigned mask) {for(unsigned i=0;i<6;++i)if(mask&(1u<<i))position[i]=0;}
 void write_music(unsigned reg,unsigned value) {
  if(reg<16) {
   music_regs[reg]=value;
   if(effect_mask) {
    if(reg==6)return;
    if(reg==7) {unsigned mask=effect_mask|(effect_mask<<3);value=(value&~mask)|(effect_regs[7]&mask);}
    else if(reg<6&&(effect_mask&(1u<<(reg/2))))return;
    else if(reg>=8&&reg<11&&(effect_mask&(1u<<(reg-8))))return;
   }
  }
  write(reg,value);
 }
 void write_effect(unsigned reg,unsigned value) {
  if(reg>=16)throw std::runtime_error("MDE effect wrote non-SSG register");
  effect_regs[reg]=value;
  // Sariel repeatedly restarts the two-channel explosion during form change.
  // Lower its software-envelope amplitude by two logarithmic SSG steps.
  if(transition_effects && reg>=8 && reg<=10 && value<16)
   value=value>2 ? value-2 : 0;
  if(reg==7) {unsigned mask=effect_mask|(effect_mask<<3);value=(music_regs[7]&~mask)|(value&mask);}
  write(reg,value);
 }
 void restore_music_ssg() {
  effect_mask=0;effect_player.reset();
  for(unsigned reg=0;reg<11;++reg)write(reg,music_regs[reg]);
 }
 void reset() {
  chip.reset();chip.set_fidelity(ymfm::OPN_FIDELITY_MIN);
  write(0x29,0x80);write(0x27,0x30);write(7,0x3f);
  position.fill(std::size_t(-1));drum_control.fill(0xdf);drum_level=63;
  phase=tick_left=fade=fade_clock=0;fading=false;transition_effects=false;dc_ssg=0;
  effect_player.reset();effect_mask=0;music_regs.fill(0);music_regs[7]=0x3f;
 }
 static std::vector<std::int16_t> wav(const Bytes& bytes) {
  if(bytes.size()<44||std::string(bytes.begin(),bytes.begin()+4)!="RIFF"||std::string(bytes.begin()+8,bytes.begin()+12)!="WAVE")throw std::runtime_error("Invalid OPNA rhythm WAV");
  auto le=[&](unsigned p,unsigned n) {std::uint32_t value=0;for(unsigned i=0;i<n;++i)value|=std::uint32_t(bytes.at(p+i))<<(i*8);return value;};
  bool format=false;
  for(unsigned p=12;p+8<=bytes.size();) {
   unsigned size=le(p+4,4);if(size>bytes.size()-p-8)throw std::runtime_error("Truncated rhythm WAV");
   std::string name(bytes.begin()+p,bytes.begin()+p+4);
   if(name=="fmt ")format=size>=16&&le(p+8,2)==1&&le(p+10,2)==1&&le(p+12,4)==48000&&le(p+22,2)==16;
   if(name=="data") {
    if(!format||size%2)throw std::runtime_error("Rhythm WAV must be mono 48kHz 16-bit PCM");
    std::vector<std::int16_t> samples;for(unsigned i=0;i<size;i+=2)samples.push_back(std::int16_t(le(p+8+i,2)));return samples;
   }
   p+=8+size+(size&1);
  }
  throw std::runtime_error("Rhythm WAV has no samples");
 }
 explicit Impl(const std::filesystem::path& assets,unsigned mask):output_mask(mask) {
  if(mask>7)throw std::invalid_argument("Invalid audio diagnostic mask");
  for(auto& entry:std::filesystem::directory_iterator(assets))if(uppercase(entry.path().extension().string())==".MDT")files.emplace(uppercase(entry.path().filename().string()),read_file(entry.path()));
  unsigned index=0;for(auto name:{"BD","SD","TOP","HH","TOM","RIM"})drums[index++]=wav(read_file(assets/(std::string("2608_")+name+".WAV")));
  effect_data=read_file(assets/"ZIGOKU.MDE");
  reset();
 }
 void command(const std::string& operation,const std::string& name) {
  if(operation=="bgm_load") {auto key=uppercase(name);if(!files.count(key))throw std::runtime_error("Missing MDT music: "+key);loaded=key;active=false;song.reset();reset();}
  else if(operation=="bgm_play") {
   if(loaded.empty())throw std::runtime_error("Music play requested without MDT data");
   reset();song=std::make_unique<MdtPlayer>(files.at(loaded),[this](unsigned r,unsigned v){write_music(r,v);},[this](unsigned mask){rhythm(mask);});active=true;
  }else if(operation=="bgm_stop") {active=false;song.reset();reset();}
  else if(operation=="bgm_fade") {if(active){fading=true;fade_clock=0;}}
  else if(operation=="se_transition") {transition_effects=true;}
  else if(operation=="se_transition_end") {transition_effects=false;}
  else if(operation=="se_load") {if(uppercase(name)!="ZIGOKU.MDE")throw std::runtime_error("Unknown effect bank: "+name);}
  else throw std::invalid_argument("Unknown music command: "+operation);
 }
 void effect(unsigned id) {
  if(id==0)return;
  if(effect_data.size()<16||id>unsigned(effect_data[2]|effect_data[3]<<8))throw std::invalid_argument("Invalid MDE effect ID");
  if(effect_player)restore_music_ssg();
  unsigned count=effect_data.at(16+(id-1)*4);
  if(count<1||count>3)throw std::runtime_error("Invalid MDE effect track count");
  effect_mask=((1u<<count)-1)<<(3-count);effect_regs.fill(0);effect_regs[7]=0x3f;
  effect_player=std::make_unique<MdtPlayer>(effect_data,id,[this](unsigned r,unsigned v){write_effect(r,v);});
 }
 void render(std::int16_t* output,unsigned frames) {
  for(unsigned frame=0;frame<frames;++frame) {
   if(!active&&!effect_player) {output[frame*2]=output[frame*2+1]=0;continue;}
   std::int64_t left=0,right=0,ssg=0;unsigned count=0;
   phase+=166400;
   while(phase>=48000) {
    phase-=48000;
    if(tick_left==0) {
     if(active) {song->tick();last_period=song->timer_period();}
     if(effect_player) {effect_player->tick();if(effect_player->finished())restore_music_ssg();}
     tick_left=std::max(1u,last_period)*48;
     if(fading&&++fade_clock==2) {fade_clock=0;if(++fade>=127)active=false;}
    }
    --tick_left;
    ymfm::ym2608::output_data sample;chip.generate(&sample);
    // OPNA FM output is 13-bit. Match its scale to the SSG/rhythm
    // paths before the shared output attenuation (validated with solo probes).
    if(output_mask&1){left+=sample.data[0]*4;right+=sample.data[1]*4;}
    if(output_mask&2)ssg+=sample.data[2];++count;
   }
   left/=count;right/=count;ssg/=count;
   // Only SSG is unipolar. Filtering the complete mix also attenuates bass
   // and drum transients, which are already signed, zero-centered PCM.
   dc_ssg+=((ssg*65536)-dc_ssg)/384;
   ssg-=dc_ssg/65536;left+=ssg;right+=ssg;
   for(unsigned i=0;i<6;++i)if(position[i]<drums[i].size()) {
    int sample=drums[i][position[i]++];
    unsigned attenuation=(63-drum_level)+(31-(drum_control[i]&31));
    int gain=std::max(0,15-int(attenuation&7));sample=sample*gain/15/(1<<(attenuation>>3));
    if((output_mask&4)&&(drum_control[i]&0x80))left+=sample;
    if((output_mask&4)&&(drum_control[i]&0x40))right+=sample;
   }
   left=left*(127-fade)/508;right=right*(127-fade)/508;
   output[frame*2]=std::clamp<std::int64_t>(left,-32768,32767);
   output[frame*2+1]=std::clamp<std::int64_t>(right,-32768,32767);
  }
 }
};
MusicSynth::MusicSynth(const std::filesystem::path& assets,unsigned mask):impl(std::make_unique<Impl>(assets,mask)) {}
MusicSynth::~MusicSynth()=default;
void MusicSynth::command(const std::string& operation,const std::string& asset) {impl->command(operation,asset);}
void MusicSynth::effect(unsigned id) {impl->effect(id);}
void MusicSynth::render(std::int16_t* stereo,unsigned frames) {impl->render(stereo,frames);}
bool MusicSynth::playing() const {return impl->active;}
unsigned MusicSynth::notes() const {return impl->song?impl->song->notes():0;}
void MusicSynth::set_effect_timer_period(unsigned period) {impl->last_period=std::clamp(period,1u,255u);}
}

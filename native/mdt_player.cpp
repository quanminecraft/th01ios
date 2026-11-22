#include "mdt_player.hpp"
#include <algorithm>
#include <array>
#include <utility>
#include <cmath>

namespace th01_native {
struct MdtPlayer::Impl {
 struct Loop {std::size_t start;unsigned left;unsigned kind;};
 struct Channel {
  unsigned id=0,number=0,wait=0,gate=0,q=8,volume=0,instrument=0,pan=3,slots=0xf0;
  std::size_t pc=0;
  int detune=0,transpose=0,pitch=0,glide=0;
  bool ended=false,tied=false,on=false;
  unsigned env_stage=0,env_level=0;
  std::array<unsigned,6> env{};
  std::vector<Loop> loops;
  std::vector<std::size_t> returns;
  std::array<unsigned,4> pitch_lfo{},amplitude_lfo{};
  std::array<unsigned,2> lfo_phase{},lfo_delay{},lfo_counter{};
  std::array<bool,2> saw{};
  unsigned age=0;
  int lfo_pitch=0,lfo_volume=0;
  int midi_note=-1,midi_expression=-1,midi_bend=-1;
 };
 Bytes bytes;
 Write write;
 Midi midi;
 std::function<void(unsigned)> rhythm;
 std::vector<Channel> channels;
 std::size_t fm=0,ssg=0;
 unsigned period=62,loop_count=0,note_count=0,mixer=0x3f;
 unsigned u8(std::size_t& pc) {if(pc>=bytes.size())throw std::runtime_error("MDT event outside file");return bytes[pc++];}
 unsigned u16(std::size_t& pc) {unsigned lo=u8(pc);return lo|(u8(pc)<<8);}
 int s8(std::size_t& pc) {return std::int8_t(u8(pc));}
 int s16(std::size_t& pc) {return std::int16_t(u16(pc));}
 bool is_ssg(const Channel& c) const {return (c.id&0x40)!=0;}
 bool is_fm(const Channel& c) const {return (c.id&0x80)!=0;}
 unsigned midi_channel(const Channel& c) const {return is_fm(c)?c.number:is_ssg(c)?6+c.number:9;}
 void message(const Channel& c,unsigned status,unsigned a,unsigned b=0) {if(midi)midi(status|midi_channel(c),a,b);}
 void midi_frequency(Channel& c) {
  if(!midi||c.midi_note<0)return;
  double hz=0;
  int pitch=c.pitch+c.lfo_pitch;
  if(is_fm(c))hz=7987200.0*(pitch&2047)*std::ldexp(1.0,(pitch>>11)&7)/(144.0*1048576.0);
  else if(is_ssg(c))hz=7987200.0/(64.0*std::max(1,pitch));
  if(hz<=0)return;
  int bend=std::clamp(int(std::lround(8192+(69+12*std::log2(hz/440.0)-c.midi_note)*8192/12)),0,16383);
  if(bend!=c.midi_bend) {message(c,0xe0,bend&127,bend>>7);c.midi_bend=bend;}
 }
 unsigned address(const Channel& c,unsigned reg) const {return reg+c.number%3+(c.number>=3?0x100:0);}
 void reg(const Channel& c,unsigned r,unsigned value) {write(address(c,r),value&255);}
 static bool carrier(unsigned algorithm,unsigned op) {
  return op==3||(algorithm>=4&&op==2)||(algorithm>=5&&op==1)||(algorithm==7&&op==0);
 }
 std::array<unsigned,32> voice(const Channel& c) const {
  std::array<unsigned,32> data{};
  auto offset=fm+(c.instrument&63)*32;
  if(offset+32>ssg)throw std::runtime_error("MDT FM instrument outside bank");
  for(unsigned i=0;i<32;++i)data[i]=bytes.at(offset+i);
  return data;
 }
 void midi_volume(Channel& c) {
  if(midi&&(is_fm(c)||is_ssg(c))) {
   int expression=is_ssg(c)?int(c.volume)*127/15:int(c.volume);
   expression=std::clamp(expression-c.lfo_volume,0,127);
   if(is_fm(c)&&c.pan==0)expression=0;
   if(expression!=c.midi_expression) {message(c,0xb0,11,expression);c.midi_expression=expression;}
  }
 }
 void volume(Channel& c) {
  midi_volume(c);
  if(is_fm(c)) {
   auto inst=voice(c);unsigned algorithm=inst[4]&7;
   for(unsigned op=0;op<4;++op) {
    int attenuation=inst[10+op];
    // MDRV98 default FM master attenuation is 8 at resident 0x12f.
    if(carrier(algorithm,op))attenuation=std::uint8_t(attenuation+127-std::clamp(int(c.volume)-8,0,127)+c.lfo_volume);
    reg(c,0x40+op*4,std::min(attenuation,127));
   }
  }else if(is_ssg(c)) {
   int value=c.on?int(c.env_level)*(std::min(c.volume,15u)+1)/256:0;
   value=std::max(0,value-(c.lfo_volume>>3));
   write(8+c.number,value);
  }
 }
 void instrument(Channel& c,unsigned value) {
  c.instrument=value;
  if(is_fm(c)) {
   auto inst=voice(c);c.slots=(inst[3]&0x78)<<1;
   reg(c,0xb0,inst[4]&0x3f);
   // MDRV98 0x12c8..0x12d5 forces both output channels when loading an
   // OPN voice. Leaving B4 at chip-reset zero silently drops that FM part.
   c.pan=3;reg(c,0xb4,0xc0);
   for(unsigned op=0;op<4;++op) {
    reg(c,0x30+op*4,inst[6+op]);
    reg(c,0x50+op*4,inst[14+op]);reg(c,0x60+op*4,inst[18+op]);
    reg(c,0x70+op*4,inst[22+op]);reg(c,0x80+op*4,inst[26+op]);
   }
   volume(c);
  }else if(is_ssg(c)) {
   auto offset=ssg+(value&63)*6;
   if(offset+6>bytes.size())throw std::runtime_error("MDT SSG envelope outside bank");
   for(unsigned i=0;i<6;++i)c.env[i]=bytes.at(offset+i);
   mixer=(mixer&~((1u<<c.number)|(8u<<c.number)))|((value&0x40)?1u<<c.number:0)|((value&0x80)?8u<<c.number:0);
   write(7,mixer);
  }
 }
 void frequency(Channel& c) {
  midi_frequency(c);
  if(is_fm(c)) {
   int value=c.pitch+c.lfo_pitch;
   reg(c,0xa4,(value>>8)&0x3f);reg(c,0xa0,value&255);
  }else if(is_ssg(c)) {
   int value=std::clamp(c.pitch+c.lfo_pitch,0,4095);
   write(c.number*2,value&255);write(c.number*2+1,value>>8);
  }
 }
 void off(Channel& c) {
  if(c.midi_note>=0) {message(c,0x80,c.midi_note);c.midi_note=-1;}
  if(is_fm(c)) {write(0x28,c.number%3+(c.number>=3?4:0));c.on=false;}
  else if(is_ssg(c)) {c.env_stage=3;}
 }
 void note(Channel& c,unsigned value,unsigned duration) {
  static constexpr int fnums[]={0x26a,0x28f,0x2b6,0x2df,0x30b,0x339,0x36a,0x39e,0x3d5,0x410,0x44e,0x48f};
  static constexpr int periods[]={3816,3602,3400,3209,3029,2859,2698,2547,2404,2269,2142,2022};
  if(value==0x90) {off(c);c.wait=duration?duration:256;c.gate=0;return;}
  int semitone=(value>>4)*12+(value&15)+c.transpose;
  semitone=std::clamp(semitone,0,95);
  if(is_fm(c))c.pitch=fnums[semitone%12]+(semitone/12<<11)+c.detune;
  else if(is_ssg(c))c.pitch=(periods[semitone%12]-c.detune)>>(semitone/12);
  frequency(c);
  if(!c.tied) {
   if(c.id==0x10)rhythm(c.instrument&63);
   if(is_fm(c)) {
    // Gate release already issued the hardware key-off on its own tick.
    // An immediate off/on pair is collapsed by the FM engine before clocking.
    if(c.midi_note>=0){message(c,0x80,c.midi_note);c.midi_note=-1;}
    write(0x28,(c.number%3)+(c.number>=3?4:0)+c.slots);
   }
   if(is_ssg(c)) {c.env_stage=0;c.env_level=c.env[0];}
   c.age=0;c.lfo_pitch=c.lfo_volume=0;c.on=true;
   c.lfo_delay={c.pitch_lfo[3]+1,c.amplitude_lfo[3]+1};c.lfo_phase.fill(0);
   if(midi) {
    if(c.id==0x10) {
     static constexpr unsigned drum_notes[]={36,38,51,42,45,37};
     for(unsigned i=0;i<6;++i)if(c.instrument&(1u<<i)) {message(c,0x80,drum_notes[i]);message(c,0x90,drum_notes[i],100);}
    }else {
     c.midi_note=semitone+24;c.midi_bend=-1;midi_frequency(c);message(c,0x90,c.midi_note,100);
    }
   }
  }
  c.tied=false;c.wait=duration?duration:256;c.gate=c.wait*(c.q&7)/8;
  if((c.q&7)==0)c.gate=c.wait;
  if(is_fm(c)&&c.q==0)c.gate=0;
  if(c.pc<bytes.size()&&bytes[c.pc]==0x91) {++c.pc;c.tied=true;c.gate=c.wait+1;}
  volume(c);++note_count;
  // MDRV98 note dispatch enters 0x1bdc immediately, decrementing the new
  // FM gate once on the note-on tick. Native previously started one tick late.
  if(is_fm(c)&&!c.tied&&c.gate&&--c.gate==0)off(c);
 }
 unsigned payload(unsigned op,const Channel& c,std::size_t pc) const {
  if(op<0x80||op==0x90)return 1;
  if(op==0xe8||op==0xed||op==0xf2||op==0xfb||op==0xfc||op==0xfd)return 4;
  if(op==0xf6||op==0xf7)return 3;
  if(op==0xee||op==0xf3||op==0xf9||op==0xfa)return 2;
  if(op==0xf1)return c.id==0x10?2:1;
  if(op==0xec&&c.id==0x10)return bytes.at(pc)&128?2:7;
  if(op==0xe0||op==0xe4||op==0xe6||op==0xe7||op==0xe9||op==0xea||op==0xeb||op==0xec||op==0xef||op==0xf0||op==0xf4||op==0xf5||op==0xf8)return 1;
  return 0;
 }
 void skip_last(Channel& c,unsigned open,unsigned close) {
  unsigned depth=0;
  for(unsigned guard=0;guard<bytes.size();++guard) {
   unsigned op=u8(c.pc);if(op==open)++depth;
   if(op==close) {if(depth==0){c.pc+=payload(op,c,c.pc);return;}--depth;}
   c.pc+=payload(op,c,c.pc);
  }
  throw std::runtime_error("Unterminated MDT loop escape");
 }
 void events(Channel& c) {
  for(unsigned guard=0;guard<10000;++guard) {
   unsigned op=u8(c.pc);
   if(op<0x80||op==0x90) {
    unsigned duration=u8(c.pc);
    // MDRV98 0xcac..0xcc4 coalesces same-pitch FM ties before applying
    // quantization. Quantizing only the final fragment releases too late.
    if(is_fm(c)&&op<0x80) {
     if(!duration)duration=256;
     while(c.pc+2<bytes.size()&&bytes[c.pc]==0x91&&bytes[c.pc+1]==op) {
      c.pc+=2;unsigned part=u8(c.pc);duration+=part?part:256;
     }
    }
    c.glide=0;note(c,op,duration);return;
   }
   switch(op) {
    case 0xff:if(!c.returns.empty()){c.pc=c.returns.back();c.returns.pop_back();break;}off(c);c.ended=true;return;
    case 0x91:c.tied=true;break;
    case 0xe0:case 0xe4:case 0xf6: {unsigned count=u8(c.pc);if(op==0xf6)c.pc+=2;c.loops.push_back({c.pc,count,op});break;}
    case 0xe2:case 0xe5:case 0xf7: {
     if(op==0xf7)c.pc+=3;
     if(c.loops.empty())throw std::runtime_error("MDT loop stack underflow");
     auto& loop=c.loops.back();if(loop.left==0||--loop.left)c.pc=loop.start;else c.loops.pop_back();break;
    }
    case 0xe1:case 0xf9: {
     if(op==0xf9)c.pc+=2;
     if(c.loops.empty())throw std::runtime_error("MDT escape outside loop");
     if(c.loops.back().left==1) {skip_last(c,op==0xe1?0xe0:0xf6,op==0xe1?0xe2:0xf7);c.loops.pop_back();}break;
    }
    case 0xe3:off(c);break;
    case 0xe6:c.detune=s8(c.pc);break;
    case 0xe7:c.transpose=s8(c.pc);break;
    case 0xe8:case 0xed:case 0xfb:case 0xfc: {
     auto& lfo=(op==0xed||op==0xfb)?c.pitch_lfo:c.amplitude_lfo;
     unsigned index=(op==0xed||op==0xfb)?0:1;c.saw[index]=op==0xfb||op==0xfc;
     for(auto& value:lfo)value=u8(c.pc);
     if(index==0&&is_fm(c))lfo[2]>>=2;
     c.lfo_delay[index]=lfo[3]+1;break;
    }
    case 0xe9:{unsigned tempo=u8(c.pc);period=tempo<17?255:4340/tempo;break;}
    case 0xea:c.q=u8(c.pc);break;
    case 0xeb:instrument(c,u8(c.pc));break;
    case 0xec: {
     unsigned value=u8(c.pc);
     if(c.id==0x10) {
      if(value&128){unsigned level=u8(c.pc);write(0x18+(value&127),0xc0|std::min(level,31u));}
      else {// MDRV98 defaults rhythm attenuation to 8 (0x130), then adds one.
      write(0x11,unsigned(std::max(0,int(value&63)-8)+1));for(unsigned i=0;i<6;++i){unsigned level=u8(c.pc);write(0x18+i,0xc0|std::min(level,31u));}}
     }else {c.volume=value;volume(c);}break;
    }
    case 0xee:{unsigned reg=u8(c.pc),value=u8(c.pc);write(reg,value);break;}
    case 0xef:{unsigned value=u8(c.pc);if(is_ssg(c))write(6,value&31);break;}
    case 0xf0:u8(c.pc);break;
    case 0xf1: {
     unsigned pan=u8(c.pc);c.pan=pan;
     if(is_fm(c))reg(c,0xb4,(pan==1?2:pan==2?1:pan)<<6);
     if(is_fm(c))message(c,0xb0,10,pan==1?0:pan==2?127:64);
     if(is_fm(c))midi_volume(c);
     else if(c.id==0x10){unsigned sample=pan;pan=u8(c.pc);write(0x18+sample,pan<<6);}break;
    }
    case 0xf2:{unsigned start=u8(c.pc),duration=u8(c.pc);int change=s16(c.pc);note(c,start,duration);c.glide=is_fm(c)?(change>>2):change;return;}
    case 0xf3:{int delta=s16(c.pc);auto destination=std::int64_t(c.pc)+delta-1;if(destination<0||destination>=std::int64_t(bytes.size()))throw std::runtime_error("MDT song loop outside file");c.pc=destination;++loop_count;break;}
    case 0xf4:case 0xf5:{int delta=u8(c.pc);c.volume=std::clamp(int(c.volume)+(op==0xf4?delta:-delta),0,is_ssg(c)?15:127);volume(c);break;}
    case 0xf8:u8(c.pc);break;
    case 0xfa:{unsigned destination=u16(c.pc);c.returns.push_back(c.pc);c.pc=destination;if(c.returns.size()>32)throw std::runtime_error("MDT macro recursion");break;}
    case 0xfd:c.pc+=4;break;
    case 0xfe:break;
    default:throw std::runtime_error("Unsupported MDT opcode "+std::to_string(op));
   }
  }
  throw std::runtime_error("MDT zero-duration command cycle");
 }
 void envelope(Channel& c) {
  unsigned step=c.env[c.env_stage==0?1:c.env_stage==1?2:c.env_stage==2?4:5];
  unsigned target=c.env_stage==0?255:c.env_stage==1?c.env[3]:0;
  if(c.env_stage==0)c.env_level=std::min(c.env_level+step,target);
  else c.env_level=c.env_level>target+step?c.env_level-step:target;
  if(c.env_level==target) {if(c.env_stage<2)++c.env_stage;else if(target==0)c.on=false;}
  volume(c);
 }
 void lfo(Channel& c,unsigned index) {
  const auto& config=index==0?c.pitch_lfo:c.amplitude_lfo;
  if(config[0]==0)return;
  unsigned depth=config[1];if(c.saw[index])depth=(depth>>1)|((depth&1)<<7);
  if(c.lfo_delay[index]) {
   if(--c.lfo_delay[index])return;
   c.lfo_phase[index]=0;c.lfo_counter[index]=(std::int8_t(depth)>>1)&0xbf;
   if(index==1)c.lfo_volume=c.saw[index]&&!(c.lfo_counter[index]&128)?127:0;
  }
  unsigned phase=c.lfo_phase[index]+config[0];c.lfo_phase[index]=phase&255;
  if(phase<256)return;
  unsigned& counter=c.lfo_counter[index];
  if(index==0) {c.lfo_pitch+=(counter&128)?-int(config[2]):int(config[2]);frequency(c);}
  else {c.lfo_volume=(c.lfo_volume+((counter&128)?int(config[2]):-int(config[2])))&255;volume(c);}
  if(!c.saw[index]) {counter=(counter-1)&255;if((counter&127)==0)counter=((depth&127)+counter)^128;}
 }
 Impl(Bytes data,Write callback,std::function<void(unsigned)> drums):bytes(std::move(data)),write(std::move(callback)),rhythm(std::move(drums)) {
  if(!write||!rhythm||bytes.size()<12||bytes[0]!=2||bytes[1]!=3)throw std::runtime_error("Invalid MDT header or sound callbacks");
  std::size_t cursor=2;unsigned count=u16(cursor),chip=u16(cursor);
  unsigned fm_number=0,ssg_number=0;
  if(count>32||chip!=1)throw std::runtime_error("Unsupported MDT chip/channel count");
  for(unsigned i=0;i<count;++i) {
   unsigned pc=u16(cursor),id=u16(cursor);if(!id)continue;
   if(id!=0x10&&(id<0x40||id>0x42)&&(id<0x80||id>0x85))throw std::runtime_error("Unsupported MDT channel");
   // MDRV98 0x99e..0x9e2 scans the table by type mask, assigning each
   // present stream to the next physical channel. Low ID bits are not slots.
   Channel c;c.id=id;c.number=(id&0x80)?fm_number++:(id&0x40)?ssg_number++:0;c.pc=pc;c.volume=is_ssg(c)?15:127;channels.push_back(c);
  }
  fm=u16(cursor);ssg=u16(cursor);u16(cursor);
  if(fm>ssg||ssg>bytes.size()||(ssg-fm)%32)throw std::runtime_error("Invalid MDT instrument banks");
 }
 Impl(Bytes data,unsigned id,Write callback):bytes(std::move(data)),write(std::move(callback)),rhythm([](unsigned){}) {
  if(!write||bytes.size()<16)throw std::runtime_error("Invalid MDE header");
  std::size_t cursor=0;unsigned version=u16(cursor),count=u16(cursor);ssg=u16(cursor);fm=ssg;
  if(version!=1||id==0||id>count||ssg>bytes.size())throw std::runtime_error("Invalid MDE effect ID or envelope bank");
  cursor=16+(id-1)*4;unsigned tracks=u8(cursor),kind=u8(cursor),table=u16(cursor);
  if(kind!=0x40||tracks==0||tracks>3)throw std::runtime_error("Unsupported MDE channel type/count");
  cursor=table;
  // MDRV98 0x1eab..0x1ec4 copies effect pointers to C, then B, then A.
  for(unsigned i=0;i<tracks;++i) {Channel c;c.number=2-i;c.id=0x40+c.number;c.pc=u16(cursor);c.volume=15;channels.push_back(c);}
 }
 void tick() {
  for(auto& c:channels) {
   if(c.ended)continue;
   if(c.wait) {
    --c.wait;
    if(c.gate&&--c.gate==0&&!c.tied)off(c);
    if(c.on) {
     ++c.age;
     if(c.glide){c.pitch+=c.glide;frequency(c);}
     lfo(c,0);lfo(c,1);
     if(is_ssg(c))envelope(c);
    }
   }
   if(c.wait==0)events(c);
  }
 }
};
MdtPlayer::MdtPlayer(Bytes data,Write write,std::function<void(unsigned)> rhythm)
 :impl(std::make_unique<Impl>(std::move(data),std::move(write),std::move(rhythm))) {}
MdtPlayer::MdtPlayer(Bytes effects,unsigned id,Write write)
 :impl(std::make_unique<Impl>(std::move(effects),id,std::move(write))) {}
MdtPlayer::~MdtPlayer()=default;
void MdtPlayer::tick() {impl->tick();}
unsigned MdtPlayer::timer_period() const {return impl->period;}
unsigned MdtPlayer::loops() const {return impl->loop_count;}
unsigned MdtPlayer::notes() const {return impl->note_count;}
bool MdtPlayer::finished() const {return std::all_of(impl->channels.begin(),impl->channels.end(),[](const auto& c){return c.ended;});}
void MdtPlayer::set_midi(Midi callback) {
 impl->midi=std::move(callback);
 // General MIDI approximation: FM lead/harmony/bass and square-wave SSG.
 // Source FM instrument IDs are not GM program numbers.
 static constexpr unsigned programs[]={80,81,4,33,48,80,80,80,80};
 for(auto& c:impl->channels) {
  unsigned channel=impl->midi_channel(c);
  if(channel<9) {
   impl->message(c,0xc0,programs[channel]);
   impl->message(c,0xb0,101,0);impl->message(c,0xb0,100,0);
   impl->message(c,0xb0,6,12);impl->message(c,0xb0,38,0);
   impl->message(c,0xb0,101,127);impl->message(c,0xb0,100,127);
  }
  c.midi_expression=c.midi_bend=-1;
 }
}
}

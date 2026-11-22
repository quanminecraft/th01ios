#include "music_synth.hpp"
#include <cmath>
#include <iostream>
#include <vector>
using namespace th01_native;
int main(int argc,char** argv){try{
 if(argc!=2)throw std::runtime_error("Usage: music_balance_test ASSETS");
 double rms[2]{};unsigned notes=0;
 for(unsigned g=0;g<2;++g){MusicSynth synth(argv[1],1u<<g);synth.command("bgm_load","REIMU.MDT");synth.command("bgm_play");std::vector<std::int16_t> pcm(120000*2);synth.render(pcm.data(),120000);double sum=0;for(unsigned i=24000*2;i<pcm.size();++i)sum+=double(pcm[i])*pcm[i];rms[g]=std::sqrt(sum/(pcm.size()-48000));if(!g)notes=synth.notes();else if(notes!=synth.notes())throw std::runtime_error("Solo mask changed sequencing");}
 // Original MDRV98 + reference core: FM/SSG RMS ~= 1.146 in 0.5..2.5s.
 // A tolerance covers envelope/core/resampling differences, not a missing 4x gain.
 double ratio=rms[0]/rms[1];if(ratio<0.95||ratio>1.35)throw std::runtime_error("FM/SSG balance differs from reference: "+std::to_string(ratio));
 std::cout<<"FM/SSG RMS ratio="<<ratio<<", reference approximately 1.146\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

#pragma once
#include <cstdint>
namespace th01_native {
inline void stereo_to_mono(std::int16_t* samples,unsigned frames) {
 for(unsigned i=0;i<frames;++i) {
  auto mixed=(int(samples[i*2])+int(samples[i*2+1]))/2;
  samples[i*2]=samples[i*2+1]=mixed;
 }
}
}

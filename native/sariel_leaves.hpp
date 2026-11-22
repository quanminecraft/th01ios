#pragma once
#include "assets.hpp"
#include <memory>
namespace th01_native {
struct SarielLeafState {int flag,left,top,vx,vy;};
class SarielLeaves {
 struct Impl;std::unique_ptr<Impl> impl;
public:
 SarielLeaves(const std::vector<Entry>& archive,std::uint32_t seed);~SarielLeaves();
 SarielLeaves(const SarielLeaves&)=delete;SarielLeaves& operator=(const SarielLeaves&)=delete;
 void reset();void update(int frame,int spawn_interval);SarielLeafState state(unsigned index)const;
};
}

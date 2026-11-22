#pragma once
#include <memory>
namespace th01_native {
struct SarielBodyState{int hp,phase,frame,shield_cel,dress_cel,wand_cel;};
class SarielBody{struct Impl;std::unique_ptr<Impl> impl;public:SarielBody();~SarielBody();SarielBody(const SarielBody&)=delete;SarielBody& operator=(const SarielBody&)=delete;void render(int frame);bool raise_wand(bool restart=false);void capture_wand_background();void lower_wand();SarielBodyState state()const;};
}
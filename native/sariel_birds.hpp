#pragma once
#include "assets.hpp"
#include "sariel_body.hpp"
#include <functional>
#include <memory>
namespace th01_native {
enum class SarielBirdFire {Aimed,Ring,Rain};
struct SarielBirdState {bool alive;int hatch;double left,top,vx,vy;};
class SarielBirds {
 struct Impl;std::unique_ptr<Impl> impl;
public:
 explicit SarielBirds(const std::vector<Entry>& archive,std::function<void(unsigned)> delay={});
 ~SarielBirds();
 SarielBirds(const SarielBirds&)=delete;
 SarielBirds& operator=(const SarielBirds&)=delete;
 void reset();void spawn(double left,double top,double vx,double vy);
 void fire(SarielBirdFire group);void update(int frame);
 void begin_arc(SarielBody& body,std::uint32_t seed);
 bool advance_arc();int arc_frame()const;
 void begin_bottom(std::uint32_t seed);bool advance_bottom();int bottom_frame()const;
 SarielBirdState state(unsigned index)const;
};
}

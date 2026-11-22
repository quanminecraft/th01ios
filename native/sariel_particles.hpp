#pragma once
#include <cstdint>
#include <memory>
namespace th01_native {
enum class SarielParticleKind {Vertical,Wavy,Horizontal};
struct SarielParticleState {bool alive;double left,top,velocity;int age;};
class SarielParticles {
 struct Impl;std::unique_ptr<Impl> impl;
public:
 explicit SarielParticles(std::uint32_t seed);~SarielParticles();
 SarielParticles(const SarielParticles&)=delete;SarielParticles& operator=(const SarielParticles&)=delete;
 void reset();void update(SarielParticleKind kind,int frame,bool from_bottom=false);
 SarielParticleState state(SarielParticleKind kind,unsigned slot)const;
};
}

#pragma once
#include <cstdint>
namespace th01_native {
class SarielBody;
enum class SarielPatternKind {RandomLasers,OrbRings,SlingClusters,Semicircles,SnowflakeWithRings,FourAimedLasers,FourLasersWithRain,RadialStacksAndLasers,CurvedSprayOnce,CurvedSprayTwice};
class SarielPattern {
    SarielPatternKind kind_;
    SarielBody* body_;
    int frame_=0;
    bool done_=false;
public:
    explicit SarielPattern(std::uint32_t seed,SarielPatternKind kind=SarielPatternKind::RandomLasers,SarielBody* body=nullptr);
    ~SarielPattern();
    SarielPattern(const SarielPattern&)=delete;
    SarielPattern& operator=(const SarielPattern&)=delete;
    bool advance();
    int frame()const{return frame_;}
};
using SarielRandomLasers=SarielPattern;
}

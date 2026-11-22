#pragma once
#include "physics_types.hpp"
#include <memory>
#include <array>
namespace th01_native {
struct YuugenmaganEyeState {int left,top,cel,lock;bool hitbox_active;};
struct YuugenmaganMissileState {int frame,subphase,pairs,iterations,target,angle_west,angle_east;};
struct YuugenmaganLaserState {int frame,subphase,offset;};
struct YuugenmaganSprayState {int frame,subphase,subframe,iterations,angle;};
struct YuugenmaganPoint {int x,y;};
struct YuugenmaganPentagramState {int frame,subphase,distance;std::array<YuugenmaganPoint,5> points;};
struct YuugenmaganRegularState {int frame,phase,angle,x,y,radius;bool handoff;std::array<YuugenmaganPoint,5> points;};
// Original eye order: west,east,southwest,southeast,north. Bits 0..4 select eyes.
class YuugenmaganEyes {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    YuugenmaganEyes();~YuugenmaganEyes();
    YuugenmaganEyes(const YuugenmaganEyes&)=delete;
    YuugenmaganEyes& operator=(const YuugenmaganEyes&)=delete;
    void track_and_render(unsigned mask=31);
    void set_cel(unsigned mask,unsigned cel);
    YuugenmaganEyeState eye(unsigned index) const;
    bool advance_intro();
    int intro_frame() const;
    // Isolated phase-one attack tick; full HP/phase progression is separate.
    void advance_lateral();
    // Caller binds MissileBackend; this loads the original missile bank.
    void begin_missiles(bool clockwise);
    void advance_missiles();
    YuugenmaganMissileState missile_state() const;
    void begin_lasers();bool advance_lasers();
    YuugenmaganLaserState laser_state() const;
    void begin_spray();void advance_spray();
    YuugenmaganSprayState spray_state() const;
    void begin_pentagram_shrink();bool advance_pentagram_shrink();
    YuugenmaganPentagramState pentagram_state() const;
    void begin_regular_pentagram(bool grow);
    bool advance_regular_pentagram();
    YuugenmaganRegularState regular_state() const;
    void begin_transition(unsigned close,unsigned open,unsigned decrease,unsigned increase);
    bool advance_transition();int transition_frame() const;
};
}

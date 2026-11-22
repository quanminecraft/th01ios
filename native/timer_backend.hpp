#pragma once
#include "card_backend.hpp"
#include "pellet_backend.hpp"
#include "th01/main/playfld.hpp"
#include "th01/main/stage/stages.hpp"
#include "th01/main/stage/timer.hpp"
#include "th01/route.hpp"
#include "th01/math/str_val.hpp"
constexpr int PTN_QUARTER_W=16;
constexpr int V_RED=2;
namespace th01_native {
struct TimerBackend {
	Framebuffer& framebuffer;
	std::array<std::array<std::uint8_t,256>,2> background{};
	std::array<bool,2> captured{};
	std::function<void()> harryup_animation;
};
class TimerBackendBinding {
	TimerBackend* previous;
public:
	explicit TimerBackendBinding(TimerBackend& backend);
	~TimerBackendBinding();
	TimerBackendBinding(const TimerBackendBinding&)=delete;
	TimerBackendBinding& operator=(const TimerBackendBinding&)=delete;
};
}
void native_timer_snap_quarter(screen_x_t left,screen_y_t top,int id,int quarter);
void native_timer_put_quarter(screen_x_t left,screen_y_t top,int id,int quarter);
void native_harryup_animate();
#define ptn_snap_quarter_8 native_timer_snap_quarter
#define ptn_put_quarter_noalpha_8 native_timer_put_quarter

#include "timer_backend.hpp"
namespace {
th01_native::TimerBackend* active=nullptr;
th01_native::TimerBackend& backend() {
	if(!active) throw std::runtime_error("Native timer backend is not bound");
	return *active;
}
void validate(int id,int quarter) {
	if(id!=PTN_BG_TIMER || quarter<0 || quarter>1) throw std::invalid_argument("Timer background quarter");
}
int aligned(int x) {return x>=0 ? x/8*8 : -((-x+7)/8)*8;}
}
namespace th01_native {
TimerBackendBinding::TimerBackendBinding(TimerBackend& value):previous(active) {active=&value;}
TimerBackendBinding::~TimerBackendBinding() {active=previous;}
}
void native_timer_snap_quarter(screen_x_t left,screen_y_t top,int id,int quarter) {
	validate(id,quarter);auto& b=backend();left=aligned(left);
	for(int y=0;y<16;++y) for(int x=0;x<16;++x) b.background[quarter][y*16+x]=b.framebuffer.pixel(th01_native::current_render_page(),left+x,top+y);
	b.captured[quarter]=true;
}
void native_timer_put_quarter(screen_x_t left,screen_y_t top,int id,int quarter) {
	validate(id,quarter);auto& b=backend();left=aligned(left);
	if(!b.captured[quarter]) throw std::runtime_error("Timer background was not captured");
	for(int y=0;y<16;++y) for(int x=0;x<16;++x) b.framebuffer.put(th01_native::current_render_page(),left+x,top+y,b.background[quarter][y*16+x]);
}
void native_harryup_animate() {
	if(!backend().harryup_animation) throw std::runtime_error("Native HARRY UP animation is not connected");
	mdrv2_se_play(17);backend().harryup_animation();
}

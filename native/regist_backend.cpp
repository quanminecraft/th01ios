#include "regist_backend.hpp"
namespace {
th01_native::RegistBackend* active=nullptr;
th01_native::RegistBackend& backend() {if(!active) throw std::runtime_error("Score registration is not bound");return *active;}
}
namespace th01_native {
RegistBinding::RegistBinding(RegistBackend& value):previous(active) {if(!value.poll || !value.wait || !value.present) throw std::invalid_argument("Registration input/presentation/timing are required");active=&value;}
RegistBinding::~RegistBinding() {active=previous;}
}
void native_regist_input(bool16 reset) {backend().present();th01_native::set_input_keys(backend().poll());input_sense(reset);}
void native_regist_reset() {backend().present();th01_native::set_input_keys(backend().poll());input_reset_sense();}
void native_regist_wait(unsigned frames) {backend().wait(frames);}
void graph_2xscale_byterect_1_to_0_slow(screen_x_t left,screen_y_t top,screen_x_t src_left,screen_y_t src_top,pixel_t width,pixel_t height) {
	auto& fb=th01_native::graphics_framebuffer();left=left/8*8;src_left=src_left/8*8;width=width/16*16;
	for(int y=0;y<height;++y) for(int x=0;x<width;++x) for(int yy=0;yy<2;++yy) for(int xx=0;xx<2;++xx) fb.put(0,left+x*2+xx,top+y*2+yy,fb.pixel(1,src_left+x,src_top+y));
}
void graph_move_byterect_interpage(screen_x_t src_left,screen_y_t src_top,screen_x_t right,screen_y_t bottom,screen_x_t dst_left,screen_y_t dst_top,unsigned source) {
	auto& fb=th01_native::graphics_framebuffer();int width=(right-src_left)/8*8;src_left=src_left/8*8;dst_left=dst_left/8*8;
	for(int y=0;y<bottom-src_top;++y) for(int x=0;x<width;++x) fb.put(!source,dst_left+x,dst_top+y,fb.pixel(source,src_left+x,src_top+y));
	page_access(!source);
}

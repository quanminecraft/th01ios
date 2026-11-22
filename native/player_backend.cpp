#include "player_backend.hpp"

std::uint8_t rank=1;
bool bomb_damaging=false;
std::int8_t rem_bombs=3;
std::uint32_t bomb_frame=0;
th01_native::PlayerAnim player_48x48(false),player_48x32(true);
namespace {
th01_native::PlayerBackend* active=nullptr;
th01_native::PlayerBackend& backend() {
	if(!active) throw std::runtime_error("Player backend is not bound");
	return *active;
}
int aligned(int value) {return value>=0 ? value/8*8 : -((-value+7)/8)*8;}
void restore_pixel(int x,int y) {
	int offset=y*640+x;
	if(offset<0 || offset>=640*400) return;
	x=offset%640;y=offset/640;
	auto& fb=backend().shots.framebuffer;
	fb.put(0,x,y,fb.pixel(1,x,y));
}
const th01_native::Sprite& ptn(int id) {
	if(id<0) throw std::runtime_error("Invalid PTN ID");
	if(id/64==0) return backend().shots.sprites.images.at(id%64);
	if(id/64==1) return backend().miko.images.at(id%64);
	throw std::runtime_error("PTN slot is not loaded");
}
void ptn_op(int left,int top,int id,bool restore) {
	auto& sprite=ptn(id);
	auto& fb=backend().shots.framebuffer;
	left=aligned(left);
	for(int y=0;y<sprite.height;++y) for(int x=0;x<sprite.width;++x) {
		auto src=y*sprite.width+x;
		if(!sprite.opaque[src]) continue;
		int offset=(top+y)*640+left+x;
		if(offset<0 || offset>=640*400) continue;
		int dx=offset%640,dy=offset/640;
		fb.put(restore ? 0 : backend().shots.page,dx,dy,restore ? fb.pixel(1,dx,dy) : sprite.color[src]);
	}
	if(restore) page_access(0);
}
const th01_native::Sprite& animation(bool short_image,int image) {
	return (short_image ? backend().action32 : backend().action48).images.at(image);
}
void anim_op(int left,int top,const th01_native::Sprite& sprite,bool restore) {
	int row=top*80+aligned(left)/8;
	auto& fb=backend().shots.framebuffer;
	for(int y=0;y<sprite.height;++y,row+=80) {
		int intended=row/80+(left<0 ? 1 : 0);
		for(int x=0;x<sprite.width;++x) {
			int offset=row+x/8;
			if(offset<0 || offset>=32000 || offset/80!=intended) continue;
			auto src=y*sprite.width+x;
			if(!sprite.opaque[src]) continue;
			int dx=(offset%80)*8+x%8,dy=offset/80;
			fb.put(0,dx,dy,restore ? fb.pixel(1,dx,dy) : sprite.color[src]);
		}
	}
	page_access(0);
}
}
namespace th01_native {
PlayerBackendBinding::PlayerBackendBinding(PlayerBackend& value):previous(active) {active=&value;}
PlayerBackendBinding::~PlayerBackendBinding() {active=previous;}
void PlayerAnim::unput_8(screen_x_t left,screen_y_t top,int image) const {anim_op(left,top,animation(short_image,image),true);}
void PlayerAnim::put_0_8(screen_x_t left,screen_y_t top,int image) const {anim_op(left,top,animation(short_image,image),false);}
void PlayerAnim::unput_and_put_overlapped_8(screen_x_t put_left,screen_y_t put_top,screen_x_t unput_left,screen_y_t unput_top,int put_image,int unput_image) const {
	auto& old=animation(short_image,unput_image);
	auto& next=animation(short_image,put_image);
	int distance=(aligned(put_left+32)-aligned(unput_left+32))/8;
	// Preserve the original one-byte overlap masks even for larger distances.
	for(int y=0;y<old.height;++y) for(int x=unput_left<0 ? 8 : 0;x<old.width;++x) {
		int compare=x+(distance>0 ? -8 : distance<0 ? 8 : 0);
		bool covered=compare>=0 && compare<next.width && next.opaque.at(y*next.width+compare);
		if(old.opaque.at(y*old.width+x) && !covered)
			restore_pixel((unput_left<0 ? -8 : aligned(unput_left))+x,unput_top+y);
	}
	anim_op(put_left,put_top,next,false);
}
}
void ptn_put_8(screen_x_t left,screen_y_t top,int id) {ptn_op(left,top,id,false);}
void ptn_unput_8(screen_x_t left,screen_y_t top,int id) {ptn_op(left,top,id,true);}
bool16 bomb_update_and_render(int frame) {
	if(!backend().bomb_update) throw std::runtime_error("Native bomb effects are not connected");
	return backend().bomb_update(frame);
}
void hud_bombs_put(int previous) {
	if(!backend().bombs_hud) throw std::runtime_error("Native bomb HUD is not connected");
	backend().bombs_hud(previous);
}
void pellet_speed_raise(float amount) {
	if(!backend().raise_pellet_speed) throw std::runtime_error("Native pellet speed system is not connected");
	backend().raise_pellet_speed(amount);
}

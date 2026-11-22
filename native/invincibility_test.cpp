#include "invincibility_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		fb->clear(0,3);fb->clear(1,5);page_access(1);shape8x8_invincibility_put(-1,100,0);
		require(current_render_page()==1 && fb->pixel(0,0,100)==3,"Negative X returns without changing accessed page");
		shape8x8_invincibility_put(638,100,0);
		require(current_render_page()==0 && fb->pixel(0,0,104)==2 && fb->pixel(1,0,104)==5,"Unaligned shape wraps across right edge and preserves back page");
		fb->clear(0,3);invincibility_sprites_update_and_render(false);
		require(fb->pixel(0,0,0)==5,"Original zero-initialized inactive state performs initial cleanup");
		fb->put(0,0,0,9);native_random_seed=1;invincibility_sprites_update_and_render(false);
		require(fb->pixel(0,0,0)==9 && native_random_seed==1,"Inactive sentinel skips subsequent cleanup and RNG");
		player_left=304;invincibility_sprites_update_and_render(true);
		require(native_random_seed==0xb6a1d149u && fb->pixel(0,309,387)==2,"Eight sprites consume original 24 RNG rolls and render first slot's expected cel");
		std::uint64_t trace=14695981039346656037ull;
		for(int frame=0;frame<64;++frame) {
			invincibility_sprites_update_and_render(true);
			for(auto& page:fb->pages) for(auto& plane:page) for(auto value:plane) {trace^=value;trace*=1099511628211ull;}
		}
		auto seed=native_random_seed;invincibility_sprites_update_and_render(false);invincibility_sprites_update_and_render(false);
		require(trace==0x4c9c92021b8da4ffull,"Invincibility framebuffer trace regression");
		require(seed==native_random_seed,"Disabling invincibility consumes no RNG");
		for(int i=0;i<24;++i) seed=seed*0x015a4e35u+1;
		invincibility_sprites_update_and_render(true);require(native_random_seed==seed,"Reactivation resets all eight slots and consumes 24 RNG rolls");
		std::cout << "Original invincibility RNG, animation, edge wrap and cleanup passed; trace " << std::hex << trace << '\n';
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

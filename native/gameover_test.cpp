#include "gameover_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<2 || argc>3) throw std::invalid_argument("Usage: gameover_test ASSET_DIR [BMP]");
		auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN")),kuji1=read_sprites(asset(archive,"KUZI1.GRC")),kuji2=read_sprites(asset(archive,"KUZI2.GRC"));
		auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));fb->palette=bg.palette;for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
		unsigned vsync=0;BombBackend bomb{*fb,kuji1,kuji2,[&] {++vsync;}};BombBackendBinding bomb_binding(bomb,player);
		std::vector<unsigned> delays;std::int16_t music=0;MissBackend miss{[&](unsigned n) {delays.push_back(n);},music};MissBackendBinding miss_binding(miss);
		std::int8_t saved=3;GameOverBackend gameover{saved};GameOverBinding gameover_binding(gameover);
		player_left=304;orb_cur_left=100;orb_cur_top=200;native_random_seed=1;rem_bombs=3;page_access(0);
		player_gameover_animate();
		require(delays.size()==16 && vsync==17 && fb->display_scroll_y==0,"Game Over renders 16 effect frames with original shake and final scroll reset");
		unsigned total=0;for(unsigned i=0;i<delays.size();++i) {require(delays[i]==i/6,"Original Game Over delay ramp");total+=delays[i];}
		require(total==14 && rem_bombs==1 && saved==1 && native_random_seed==0x015a4e36u,"Original timing, saved bomb reset and one miss-sprite RNG roll");
		require(fb->pages[0]!=fb->pages[1],"Effect trails stay on foreground while miss sprite background is preserved");
		if(argc==3) dump_bmp(argv[2],*fb);
		std::cout<<"Original Game Over effects, shake, delay ramp, saved bombs and RNG passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

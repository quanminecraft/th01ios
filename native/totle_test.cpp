#include "totle_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<2 || argc>3) throw std::invalid_argument("Usage: totle_test ASSET_DIR [BMP]");
		auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN")),kuji1=read_sprites(asset(archive,"KUZI1.GRC")),kuji2=read_sprites(asset(archive,"KUZI2.GRC")),numbers=read_sprites(asset(archive,"NUMB.PTN"));
		auto clear=read_grp(read_file(std::filesystem::path(argv[1])/"CLEAR3.GRP")),old=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS1.GRP"));fb->palette=old.palette;for(unsigned p=0;p<2;++p) fb->blit(p,0,0,old.images[0],false);
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
		BombBackend bomb{*fb,kuji1,kuji2,[] {}};BombBackendBinding bomb_binding(bomb,player);std::int16_t lives=4;std::int8_t saved=4;ExtendBackend stock{lives,saved,[](int) {}};ExtendBackendBinding stock_binding(stock);
		std::vector<Palette> fade_frames;
		unsigned frames=0,milliseconds=0,rolls=0,polls=0;TotleBackend backend{*fb,clear,numbers,[&]() {InputKeys keys;keys.enter=polls++==1;return keys;},[&](unsigned n) {frames+=n;if(n==1){fade_frames.push_back(fb->palette);if(argc==3&&fade_frames.size()==1)dump_bmp(argv[2],*fb);}},[&](unsigned n) {require(n==4,"Original digit delay is four milliseconds");milliseconds+=n;++rolls;}};TotleBinding binding(backend);
		Pellets.native_reset_all();for(unsigned i=0;i<8;++i) Shots.moving[i]=false;Shots.add(200,300);stage_timer=700;cardcombo_max=5;rem_bombs=3;score=100;native_random_seed=123;page_access(0);
		totle_animate(5);
		require(score==121100 && score_bonus==0 && !backend.loaded && !backend.sloppy,"Four original bonus metrics commit once and transient numeral/bonus state resets");
		require(frames==275 && rolls==296 && milliseconds==1184 && polls==3,"Original cadence plus 30-frame acknowledgement fade");
		require(std::count(shots.sound_commands.begin(),shots.sound_commands.end(),2)==296 && native_random_seed==123 && !Shots.moving[0],"Original rolling SE cadence, unchanged RNG and shot cleanup");
		require(fade_frames.size()==30,"Acknowledgement fades for exactly half a second at 60 Hz");
  auto brightness=[](const Palette& p){unsigned sum=0;for(auto color:p)for(unsigned shift:{0u,8u,16u})sum+=(color>>shift)&255;return sum;};
  require(brightness(fade_frames.front())>0&&brightness(fade_frames.back())==0,"TOTLE fades from visible results to black");
  for(unsigned i=1;i<fade_frames.size();++i)require(brightness(fade_frames[i])<=brightness(fade_frames[i-1]),"TOTLE fade must be smooth and monotonic");
		polls=0;lives=0;rem_bombs=0;cardcombo_max=0;stage_timer=14000;score=0;totle_animate(5);
		require(score==94640,"Unsigned 16-bit timer product wraps to 4464 before long assignment and cap");
		polls=0;lives=8;rem_bombs=5;cardcombo_max=40;stage_timer=2000;score=0;totle_animate(20);
		require(score==381060,"Original metric caps and final-stage number bonus");
		std::cout<<"Original TOTLE numeral animation, bonus arithmetic, sound cadence, acknowledgement and DOS widths passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

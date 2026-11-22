#include "totle_transition.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc>3) throw std::invalid_argument("Usage: totle_transition_test [ASSET_DIR [BMP_PREFIX]]");
		auto fb=std::make_unique<Framebuffer>();SpriteSet stg,kuji1,kuji2,target;
		for(int i=0;i<34;++i) stg.images.push_back({32,32,Bytes(1024,2),Bytes(1024,1)});kuji1.images.assign(8,stg.images[0]);kuji2.images.assign(1,stg.images[0]);
		target.images.push_back({640,400,Bytes(640*400),Bytes(640*400,1)});
		for(unsigned i=0;i<16;++i) {fb->palette[i]=0xff111111u;target.palette[i]=0xff000000u|i*0x111111u;}
		for(int y=0;y<400;++y) for(int x=0;x<640;++x) {fb->put(0,x,y,(x/32+5*y+7)%16);target.images[0].color[y*640+x]=(x/32+3*y)%16;}
		if(argc>=2) {auto archive=load_archive(argv[1]);stg=read_sprites(asset(archive,"STG.PTN"));kuji1=read_sprites(asset(archive,"KUZI1.GRC"));kuji2=read_sprites(asset(archive,"KUZI2.GRC"));auto old=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS1.GRP"));target=read_grp(read_file(std::filesystem::path(argv[1])/"CLEAR3.GRP"));fb->palette=old.palette;fb->blit(0,0,0,old.images[0],false);}
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,stg,stg,stg,{},{},{}};PlayerBackendBinding player_binding(player);
		BombBackend bomb{*fb,kuji1,kuji2,[] {}};BombBackendBinding bomb_binding(bomb,player);
		unsigned calls=0,frames=0;std::uint64_t trace=14695981039346656037ull;
		totle_transition(*fb,target,[&](unsigned n) {
			++calls;frames+=n;require(n==2,"Original transition waits two frames per iteration");
			if(argc==1 && calls==1) require(fb->pixel(0,0,0)==7 && fb->pixel(0,0,1)==12,"First iteration only activates rows");
			if(argc==1 && calls==2) require(fb->pixel(0,0,0)==8 && fb->pixel(0,608,0)==0 && fb->pixel(0,0,1)==6 && fb->pixel(0,32,1)==12 && fb->pixel(0,0,8)==15,"Even rows shift left, odd rows right, next batch remains inactive");
			for(auto& plane:fb->pages[0]) for(auto byte:plane) {trace^=byte;trace*=1099511628211ull;}
			for(auto color:fb->palette) {trace^=color;trace*=1099511628211ull;}
			if(argc==3 && (calls==2 || calls==30 || calls==70)) dump_bmp(std::string(argv[2])+"-"+std::to_string(calls)+".bmp",*fb);
		});
		require(calls==70 && frames==140 && fb->pages[0]==fb->pages[1] && fb->palette==target.palette && current_render_page()==0,"All 400 rows finish in original 70 iterations with target pixels/palette and page 0 active");
		require(trace==(argc==1 ? 0x526e81c9d9f23b4dull : 0x4b6c02cf8ae757a6ull),"Row/palette progression trace regression");
		std::cout<<(argc==1 ? "Synthetic" : "Real")<<" TOTLE transition trace "<<std::hex<<trace<<std::dec<<" passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

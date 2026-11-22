#include "assets.hpp"
#include "grp.hpp"
#include "shot_backend.hpp"
#include "th01/main/player/shot.hpp"
#include "th01/main/player/orb.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
int main(int argc,char** argv) {
	try {
		if(argc!=2) throw std::runtime_error("Usage: shot_real_test ASSET_DIR");
		auto archive=load_archive(argv[1]);
		auto sprites=read_sprites(asset(archive,"STG.PTN"));
		auto background=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));
		auto fb=std::make_unique<Framebuffer>();
		fb->palette=background.palette;
		fb->blit(1,0,0,background.images[0],false);fb->pages[0]=fb->pages[1];
		ShotBackend backend{*fb,sprites,{}};
		ShotBackendBinding binding(backend);
		orb_in_portal=false;orb_cur_left=100;orb_cur_top=200;
		orb_force_new(-8,OF_IMMEDIATE);orb_velocity_y_update();
		Shots.add(108,368);
		unsigned frames=0;
		while(Shots.moving[0] && frames<30) {Shots.unput_update_render();++frames;}
		if(frames!=12 || Shots.moving[0] || orb_cur_left!=100 || orb_cur_top!=186 || orb_velocity_x!=OVX_0 || orb_force!=-14)
			throw std::runtime_error("Real-asset shot/orb integration failed");
		if(backend.sound_commands!=std::vector<unsigned>{1}) throw std::runtime_error("Unexpected shot sound commands");
		std::uint64_t hash=14695981039346656037ull;
		for(auto& plane:fb->pages[0]) for(auto value:plane) {hash^=value;hash*=1099511628211ull;}
		if(hash!=0x0d65c5fdd2da6afaull) throw std::runtime_error("Real-asset shot rendering regression");
		std::cout << "Real STG.PTN shot/orb collision passed in " << frames << " frames; framebuffer " << std::hex << hash << '\n';
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

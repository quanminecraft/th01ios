#include "bomb_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include "pellet_speed_binding.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
static SpriteSet bank(unsigned count) {SpriteSet set;for(unsigned i=0;i<count;++i) set.images.push_back({32,32,Bytes(1024,2),Bytes(1024,1)});return set;}
int main(int argc,char** argv) {
	try {
		if(argc>3) throw std::invalid_argument("Usage: bomb_test [ASSET_DIR [BMP_PREFIX]]");
		auto fb=std::make_unique<Framebuffer>();auto stg=bank(34),miko=bank(28),kuji1=bank(8),kuji2=bank(1);
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
		PelletSpeedBinding speed_binding(player);
		for(unsigned i=0;i<16;++i) fb->palette[i]=0xff000000u|i*0x111111u;
		unsigned waits=0;BombBackend bomb{*fb,kuji1,kuji2,[&]() {++waits;}};BombBackendBinding bomb_binding(bomb,player);
		fb->clear(0,3);fb->clear(1,5);page_access(0);
		require(!graph_invert_32x32_8(15,100,VRAM_PLANE_B) && fb->pixel(0,8,100)==2 && fb->pixel(0,40,100)==3,"Bomb inversion flips only blue plane in aligned 32x32 region");
		require(graph_invert_32x32_8(608,100,VRAM_PLANE_B),"Original inversion excludes exact right boundary");
		fb->clear(0,3);require(!graph_invert_32x32_8(15,100,VRAM_PLANE_R) && fb->pixel(0,8,100)==1 && fb->pixel(0,40,100)==3,"Kikuri entrance inversion flips only red plane with original alignment" );
		fb->clear(0,3);grc_put_8(-8,0,GRC_SLOT_BOMB_KUJI_1,0,7);
		require(fb->pixel(0,0,0)==3 && fb->pixel(0,0,1)==7,"Original GRC negative-X/top clipping behavior");
		z_palette_set_show(7,-1,22,5);require(fb->palette[7]==0xff00ff55u,"Palette components clamp to original RGB4 range");z_palette_set_all_show(bomb.stage);
		auto run=[&](bool real) {
			page_access(0);player_left=304;player_is_hit=false;player_invincible=false;player_invincibility_time=0;bomb_damaging=false;pellet_destroy_score_delta=0;native_pellet_speed=0;rank=RANK_EASY;Pellets.native_reset_all();
			Pellets.add_group(100,100,PG_1,56);shots.sound_commands.clear();waits=0;auto initial_palette=fb->palette;auto original_back=std::make_unique<Framebuffer::Page>(fb->pages[1]);
			require(!bomb_update_and_render(0) && player_invincible && Pellets.native_at(0).decay_frame==1 && pellet_destroy_score_delta==10,"Bomb start makes player invincible and decays active bullets");
			require(shots.sound_commands==std::vector<unsigned>{8},"Cast sound dispatches at frame zero");
            std::uint64_t trace=14695981039346656037ull;
			for(int frame=1;frame<140;++frame) {
				require(!bomb_update_and_render(frame),"Bomb remains active before frame 140");
				if(frame==49) require(!bomb_damaging,"Bomb not damaging before frame 50");
				if(frame==50) require(bomb_damaging && shots.sound_commands.back()==9,"Bomb damaging starts on frame 50");
				if(frame==64) require(fb->display_scroll_y==392,"Original bomb scroll shake phase");
				for(auto& page:fb->pages) for(auto& plane:page) for(auto v:plane) {trace^=v;trace*=1099511628211ull;}
				for(auto color:fb->palette) for(int byte=0;byte<4;++byte) {trace^=(color>>(byte*8))&255;trace*=1099511628211ull;}
				trace^=unsigned(fb->display_scroll_y);trace*=1099511628211ull;
				if(real && argc==3 && (frame==24 || frame==64)) dump_bmp(std::string(argv[2])+"-"+std::to_string(frame)+".bmp",*fb);
			}
			require(bomb_update_and_render(140) && !bomb_damaging && player_invincibility_time==100 && fb->display_scroll_y==0 && fb->palette==initial_palette && fb->pages[0]==*original_back && fb->pages[1]==*original_back,"Bomb completion restores scroll, palette, both pages and post-bomb invincibility time");
			require(waits==23 && shots.sound_commands.size()==11 && shots.sound_commands[0]==8 && shots.sound_commands[1]==8,"Original extra VSync waits and bomb sound cadence");
			for(unsigned i=2;i<11;++i) require(shots.sound_commands[i]==9,"Active-phase original bomb SE command");
			require(trace==(real ? 0x5aeb01cd158b1213ull : 0xfdff07922223e267ull),"Bomb framebuffer/palette/scroll trace regression");
			std::cout << (real ? "Real" : "Synthetic") << " original bomb trace " << std::hex << trace << std::dec << '\n';
		};
		fb->clear(0,3);fb->clear(1,5);run(false);
		player_left=304;orb_cur_left=0;orb_cur_top=64;orb_in_portal=false;player_is_hit=false;player_reset();
		rem_bombs=2;native_pellet_speed=0;bomb_frame=200;int previous_bombs=-1;player.bombs_hud=[&](int previous) {previous_bombs=previous;};
		input_lr=INPUT_NONE;input_shot=false;input_strike=false;input_bomb=true;player_unput_update_render();
		require(rem_bombs==2 && native_pellet_speed==1,"Original player action arms bomb and raises pellet speed before consuming stock");
		player_unput_update_render();require(rem_bombs==1 && previous_bombs==2 && bomb_frame==0 && player_invincible,"Next player update starts actual bomb and consumes one stock");
		for(int frame=1;frame<=140;++frame) {bomb_frame=frame;player_unput_update_render();}
		require(rem_bombs==1 && !player_deflecting && !bomb_damaging && player_invincibility_time==100,"Player action completes original bomb without repeated stock consumption");
		if(argc>=2) {
			auto archive=load_archive(argv[1]);stg=read_sprites(asset(archive,"STG.PTN"));miko=read_sprites(asset(archive,"MIKO.PTN"));auto first=read_sprites(asset(archive,"KUZI1.GRC")),second=read_sprites(asset(archive,"KUZI2.GRC"));auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));
			fb->palette=bg.palette;for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);
			BombBackend real_bomb{*fb,first,second,[&]() {++waits;}};BombBackendBinding real_binding(real_bomb,player);run(true);
		}
		std::cout << "Original bomb keyframes, kuji, palette, shake, decay and cleanup passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

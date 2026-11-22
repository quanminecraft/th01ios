#include "miss_backend.hpp"
#include "hud_icons.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
static SpriteSet bank(unsigned count) {SpriteSet set;for(unsigned i=0;i<count;++i) set.images.push_back({32,32,Bytes(1024,2),Bytes(1024,1)});return set;}
int main(int argc,char** argv) {
	try {
		if(argc>3) throw std::invalid_argument("Usage: miss_test [ASSET_DIR [BMP_PREFIX]]");
		auto fb=std::make_unique<Framebuffer>();auto stg=bank(34),miko=bank(28),kuji1=bank(8),kuji2=bank(1);FontRom font(Bytes(0x46800));
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},native_hud_bombs_render,{}};PlayerBackendBinding player_binding(player);
		CardRenderer renderer{*fb,stg,font,{}};CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);
		std::int16_t lives=3;std::int8_t saved=3;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hud_binding(hud);ExtendBackend extend{lives,saved,native_hud_lives_render};ExtendBackendBinding extend_binding(extend);
		TimerBackend timer{*fb};TimerBackendBinding timer_binding(timer);unsigned vsyncs=0;std::int16_t music=0;std::vector<unsigned> delays;std::uint64_t trace=0;bool real=false;
		MissBackend miss{{},music};
		miss.delay=[&](unsigned n) {
			delays.push_back(n);for(auto& page:fb->pages) for(auto& plane:page) for(auto v:plane) {trace^=v;trace*=1099511628211ull;}
			trace^=unsigned(fb->display_scroll_y);trace*=1099511628211ull;trace^=std::uint16_t(player_left);trace*=1099511628211ull;
			if(real && argc==3 && (delays.size()==26 || delays.size()==37)) dump_bmp(std::string(argv[2])+"-"+std::to_string(delays.size())+".bmp",*fb);
		};
		MissBackendBinding miss_binding(miss);
		auto run=[&]() {
			lives=3;saved=3;rem_bombs=4;hud.backgrounds.clear();page_access(0);native_hud_lives_init();native_hud_bombs_init();timer_init_for(0,ROUTE_MAKAI);stage_timer=0;timer_bg_snap_and_put();
			player_left=304;player_reset();player_is_hit=true;player_invincibility_time=0;stage_cleared=false;native_pellet_speed=10;native_random_seed=1;orb_cur_left=100;orb_cur_top=100;orb_in_portal=false;
			for(int i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;Shots.left[i]=0;Shots.top[i]=0;}
			Shots.add(300,300);Pellets.native_reset_all();Pellets.add_group(200,100,PG_1,56);shots.sound_commands.clear();delays.clear();music=0;vsyncs=0;trace=14695981039346656037ull;
			require(resume_after_miss(),"Hit with remaining stock resumes life loop");
			unsigned sum=0;for(auto n:delays) sum+=n;
			require(lives==2 && saved==2 && !player_is_hit && player_left==304 && player_invincibility_time==150,"Original life decrement, center return, hit reset and respawn invincibility");
			require(rem_bombs==5 && stage_timer==1000 && native_pellet_speed==0 && bomb_frame==200,"Original bomb grant, expired timer extension and speed lowering");
			require(delays.size()==46 && sum==82 && vsyncs==17 && fb->display_scroll_y==0 && music==1,"Original blocking miss timing, shake and BGM reload-suppression counter");
			require(native_random_seed==0x015a4e36u && shots.sound_commands==std::vector<unsigned>({5}),"Miss sprite selection consumes one RNG call and life loss emits SE 5");
			require(!Shots.moving[0] && Pellets.native_at(0).decay_frame==1,"Miss resets shots and decays pellets");
			require(trace==(real ? 0x838515a9ea101049ull : 0xe255208d1fd34de5ull),"Miss animation framebuffer/scroll trace regression");
			if(!real) require(fb->pixel(0,160,0)==3 && fb->pixel(1,160,0)==3 && fb->pixel(0,192,16)==2 && fb->pixel(1,192,16)==2,"Miss updates actual life/bomb HUD on both pages");
			std::cout << (real ? "Real" : "Synthetic") << " original miss trace " << std::hex << trace << std::dec << '\n';
		};
		fb->clear(0,3);fb->clear(1,3);BombBackend bomb{*fb,kuji1,kuji2,[&]() {++vsyncs;}};BombBackendBinding bomb_binding(bomb,player);run();
		player_is_hit=true;lives=0;saved=0;require(!resume_after_miss() && lives==0 && saved==0 && music==1,"Game Over does not respawn or decrement below zero");
		lives=3;saved=3;stage_cleared=true;require(!resume_after_miss() && lives==3 && saved==3,"Stage completion does not consume a life");stage_cleared=false;
		native_hud_lives_render(2);lives=2;saved=2;
		player_left=100;rem_bombs=5;native_pellet_speed=-5;stage_timer=1000;player_miss_animate_and_update();
		require(player_left==300 && rem_bombs==5 && native_pellet_speed==-7 && stage_timer==1200,"Original return-position truncation, bomb cap and running timer/speed updates");
		if(argc>=2) {
			auto archive=load_archive(argv[1]);stg=read_sprites(asset(archive,"STG.PTN"));miko=read_sprites(asset(archive,"MIKO.PTN"));kuji1=read_sprites(asset(archive,"KUZI1.GRC"));kuji2=read_sprites(asset(archive,"KUZI2.GRC"));font=FontRom(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));
			fb->palette=bg.palette;for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);real=true;
			BombBackend real_bomb{*fb,kuji1,kuji2,[&]() {++vsyncs;}};BombBackendBinding real_bomb_binding(real_bomb,player);run();
		}
		std::cout << "Original life loss, miss animation, HUD, shot/pellet reset and respawn passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

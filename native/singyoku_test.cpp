#include "singyoku_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<2 || argc>3) throw std::invalid_argument("Usage: singyoku_test ASSET_DIR [BMP]");
		auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));
		auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS1.GRP"));fb->palette=bg.palette;for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
		CardEvents events;CardBackendBinding card_binding(events);std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hud_binding(hud);
		BombBackend bomb{*fb,miko,miko,[] {}};BombBackendBinding bomb_binding(bomb,player);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding entity_binding(entities);
		unsigned waits=0,defeats=0,fades=0;SingyokuBackend boss{[&](unsigned n) {waits+=n;},[&] {++fades;},[&] {++defeats;},{}};SingyokuBinding binding(boss);
		std::uint32_t frame=1;BossHitBackend hit{frame,lives,hud_hp_decrement,[&] {z_palette_set_all_show(boss.boss_palette);}};BossHitBinding hit_binding(hit);
		boss.boss_palette=native_bomb_palette();native_random_seed=1;rank=RANK_NORMAL;test_damage=false;bomb_damaging=false;player_invincible=true;player_is_hit=false;player_left=300;orb_prev_left=0;orb_prev_top=380;score=0;Pellets.native_reset_all();
		for(unsigned i=0;i<8;++i) Shots.moving[i]=false;
		singyoku_load();require(native_singyoku_hp()==8 && native_singyoku_phase()==0,"Original load initializes eight HP and entrance phase");
		singyoku_main();require(waits==200 && native_singyoku_phase()==1,"Original blocking 200-frame entrance completes");
		auto tick=[&]() {++frame;Pellets.unput_update_render();singyoku_main();};
		for(unsigned i=0;i<120;++i) tick();
		unsigned bullets=Pellets.native_alive_count();
		require(bullets>0 && native_singyoku_hp()==8,"First sphere pattern emits real native pellets without spurious orb hits");
		for(unsigned i=0;i<2;++i) {test_damage=true;tick();}
		for(unsigned i=0;i<600 && native_singyoku_phase()!=2;++i) tick();
		require(native_singyoku_phase()==2 && native_singyoku_hp()==6,"Original HP threshold and safe pattern boundary enter phase 2");
		for(unsigned i=0;i<3000;++i) tick();
		require(!player_is_hit && native_singyoku_phase()==2 && defeats==0,"Phase 2 runs repeated transformation/slam cycles without entering defeat");
		if(argc==3) dump_bmp(argv[2],*fb);
		for(unsigned i=0;i<6;++i) {test_damage=true;tick();}
		require(native_singyoku_hp()==0 && native_singyoku_phase()==8,"Original phase 2 HP zero enters defeat phase");
		singyoku_main();require(defeats==1 && fades==1,"Defeat and music fade are explicit host handoffs, not silent stubs");
		singyoku_free();require(entities.slots[0].images.empty() && entities.slots[1].images.empty() && entities.slots[2].images.empty(),"Boss frees all three original banks");
		std::cout<<"Original Singyoku entrance, HP phases, pellet patterns, transformations and defeat handoff passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

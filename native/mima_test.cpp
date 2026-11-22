#include "mima_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        if(argc<2 || argc>3) throw std::invalid_argument("Usage: mima_test ASSET_DIR [BMP]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));
        auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS3.GRP"));fb->palette=bg.palette;for(unsigned p=0;p<2;++p) fb->blit(p,0,0,bg.images[0],false);
        ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
        CardEvents events;CardBackendBinding card_binding(events);std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hud_binding(hud);
        BombBackend bomb{*fb,miko,miko,[]() {}};BombBackendBinding bomb_binding(bomb,player);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding entity_binding(entities);
        MissileBackend missiles{archive};MissileBinding missile_binding(missiles);std::uint32_t frame=1;unsigned waits=0,delay=0,fades=0,defeats=0;
        MimaBackend boss{*fb,missiles,frame,[&](unsigned n) {waits+=n;},[&](unsigned ms) {delay+=ms;},[&] {++fades;},[&] {++defeats;},{}};MimaBinding binding(boss);
        BossHitBackend hit{frame,lives,hud_hp_decrement,[&]() {z_palette_set_all_show(boss.boss_palette);}};BossHitBinding hit_binding(hit);
        boss.boss_palette=bomb.stage;native_random_seed=1;rank=RANK_NORMAL;native_pellet_speed=0;score=0;test_damage=false;bomb_damaging=false;player_invincible=true;player_is_hit=false;player_left=300;Pellets.native_reset_all();for(auto& laser:shootout_lasers) laser={};
        orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;for(unsigned i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;}
        mima_original::mima_load();require(mima_original::boss_hp==12 && mima_original::boss_phase==0 && waits==16 && delay==160,"Original load initializes twelve HP and sixteen-step white-in");
        mima_original::mima_main();require(mima_original::boss_phase==1 && waits==116 && boss.tiles[2].size()==24,"Original entrance completes with hundred frame waits and allocated background bank");
        auto tick=[&](bool orb_hit=false) {++frame;orb_cur_left=orb_prev_left=orb_hit ? 316 : 0;orb_cur_top=orb_prev_top=orb_hit ? 200 : 380;mima_original::mima_main();update_shootout_lasers();Pellets.unput_update_render();};
        for(unsigned i=0;i<25;++i) tick();tick(true);require(mima_original::boss_hp==11 && score==5000,"Real orb hit decrements Mima HP and awards original five-thousand points");
        std::array<bool,4> visited{};unsigned cycle_ticks=0;
        do {tick();if(boss.pattern<4) visited[boss.pattern]=true;++cycle_ticks;if(cycle_ticks>2000) throw std::runtime_error("Mima phase-one cycle stalled");}while(!(visited[0] && visited[1] && visited[2] && visited[3] && boss.pattern==0 && mima_original::boss_phase_frame==0));
        require(mima_original::boss_phase==1 && mima_original::boss_hp==11 && !player_is_hit,"Full phase-one cycle runs real projectiles and all four patterns");
        while(mima_original::boss_hp>6) {test_damage=true;tick();}
        require(mima_original::boss_phase==1,"HP threshold waits for current pattern cleanup");
        for(unsigned i=0;i<1000 && mima_original::boss_phase!=2;++i) tick();require(mima_original::boss_phase==2,"Safe pattern boundary enters original phase two");
        for(unsigned i=0;i<100 && mima_original::boss_phase!=3;++i) tick();require(mima_original::boss_phase==3 && mima_original::boss_hp==6,"Recenter spread transitions into phase three");
        visited={};cycle_ticks=0;
        do {tick();if(boss.pattern<4) visited[boss.pattern]=true;++cycle_ticks;if(cycle_ticks>2000) throw std::runtime_error("Mima phase-three cycle stalled");}while(!(visited[0] && visited[1] && visited[2] && visited[3] && boss.pattern==0 && mima_original::boss_phase_frame==0));
        require(!player_is_hit && defeats==0 && mima_original::boss_hp==6,"All late patterns run with missiles, particles, pellets and lasers");
        if(argc==3) dump_bmp(argv[2],*fb);
        while(mima_original::boss_hp>0) {test_damage=true;tick();}
        require(boss.defeated && fades==1 && defeats==1,"Zero HP performs original music/actor cleanup and explicit defeat handoff");
        mima_original::mima_main();require(fades==1 && defeats==1,"Native defeat boundary is idempotent");
        mima_original::mima_free();require(entities.slots[0].images.empty() && entities.slots[1].images.empty() && boss.tiles[2].empty() && missiles.sprites.images.empty(),"Original free releases sprite, background and missile banks");
        unsigned waits_before=waits;
        for(unsigned p=0;p<2;++p) fb->blit(p,0,0,bg.images[0],false);z_palette_set_all_show(bomb.stage);hud.backgrounds.clear();Pellets.native_reset_all();for(auto& laser:shootout_lasers) laser={};native_random_seed=1;
        mima_original::mima_load();mima_original::mima_main();
        require(!boss.defeated && mima_original::boss_hp==12 && mima_original::boss_phase==1 && boss.pattern==0 && boss.invincibility==0 && waits-waits_before==116,"Reload reconstructs original fresh MAIN boss and clears prior static hit/phase state");
        require(fades==1 && defeats==1 && native_random_seed==0x015a4e36u,"Reload starts fresh particles/hop initialization without stale defeat dispatch");
        mima_original::mima_free();
        std::cout<<"Full original Mima entrance, orb HP, all eight attacks, phase boundaries and defeat handoff passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
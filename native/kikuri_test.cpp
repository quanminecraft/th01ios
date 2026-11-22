#include "kikuri_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        if(argc<2 || argc>3) throw std::invalid_argument("Usage: kikuri_test ASSET_DIR [BMP]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN")),ripples=read_sprites(asset(archive,"TAMAYEN.PTN"));
        auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS4.GRP"));fb->palette=bg.palette;fb->blit(1,0,0,bg.images[0],false);fb->clear(0,0);
        ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
        CardEvents events;CardBackendBinding card_binding(events);std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hud_binding(hud);
        BombBackend bomb{*fb,miko,miko,[]() {}};BombBackendBinding bomb_binding(bomb,player);const auto original_palette=bomb.stage;
        BossEntityBackend entities{*fb,archive,{}};BossEntityBinding entity_binding(entities);KikuriRippleBackend ripple{*fb,ripples};KikuriRippleBinding ripple_binding(ripple);
        unsigned waits=0,fades=0,defeats=0;KikuriBackend boss{*fb,archive,ripples,[&](unsigned n) {waits+=n;},[&] {++fades;},[&] {++defeats;}};KikuriBinding binding(boss);
        std::uint32_t frame=1;BossHitBackend hit{frame,lives,hud_hp_decrement,[]() {kikuri_original::boss_palette_show();}};BossHitBinding hit_binding(hit);
        native_random_seed=1;rank=RANK_NORMAL;native_pellet_speed=0;score=0;player_left=300;player_invincible=true;player_is_hit=false;player_deflecting=false;test_damage=false;bomb_damaging=false;
        orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;Pellets.native_reset_all();for(auto& laser:shootout_lasers) laser={};for(unsigned i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;}
        kikuri_original::kikuri_load();kikuri_original::boss_palette_snap();require(kikuri_original::boss_hp==14 && kikuri_original::kikuri_phase==0 && pellet_interlace,"Original Kikuri load initializes fourteen HP and interlaced pellets");
        kikuri_original::kikuri_main();require(kikuri_original::kikuri_phase==2 && waits==752 && boss.patterns_done==0,"Original barn-door and expanding red-plane ring entrance completes");unsigned entrance_waits=waits;
        auto tick=[&](bool orb_hit=false) {++frame;orb_cur_left=orb_prev_left=orb_hit ? 300 : 0;orb_cur_top=orb_prev_top=orb_hit ? 100 : 380;kikuri_original::kikuri_main();update_shootout_lasers();Pellets.unput_update_render();};
        for(unsigned i=0;i<25;++i) tick();orb_cur_left=300;orb_cur_top=180;require(!kikuri_original::kikuri_hittest_orb(),"Original inverted-Y hitbox rejects geometric disc center");orb_cur_top=100;require(kikuri_original::kikuri_hittest_orb(),"Original inverted-Y hitbox accepts upper position");
        tick(true);require(kikuri_original::boss_hp==13 && score==7000,"Real Kikuri orb hit awards seven-thousand and one HP damage");
        for(unsigned i=0;i<200;++i) tick();require(Pellets.native_alive_count()>0 && kikuri_original::kikuri_phase==2,"First phase emits real symmetric spiral pellets");
        while(kikuri_original::boss_hp>10) {test_damage=true;tick();}
        for(unsigned i=0;i<300 && kikuri_original::kikuri_phase!=4;++i) tick();require(kikuri_original::kikuri_phase==4,"HP-ten threshold and lightball palette transition reach soul activation");
        for(unsigned i=0;i<1000 && kikuri_original::kikuri_phase!=5;++i) tick();require(kikuri_original::kikuri_phase==5,"Spinning rings and soul spread sequence reach moving-soul phase");
        bool saw_tears=false;for(unsigned i=0;i<800;++i) {tick();saw_tears|=kikuri_original::native_kikuri_tears_alive()!=0;}
        require(saw_tears && !player_is_hit && kikuri_original::kikuri_phase==5,"Moving souls drop live tears/ripples and crossed lasers without invincible-player hits");
        while(kikuri_original::boss_hp>6) {test_damage=true;tick();}for(unsigned i=0;i<100 && kikuri_original::kikuri_phase!=6;++i) tick();require(kikuri_original::kikuri_phase==6,"HP-six threshold reaches final phase");
        std::array<bool,4> visited{};unsigned cycles=0;
        do {tick();if(boss.subphase<4) visited[boss.subphase]=true;if(++cycles>4000) throw std::runtime_error("Kikuri final pattern cycle stalled");}while(!(visited[0] && visited[1] && visited[2] && visited[3] && boss.subphase==0 && kikuri_original::boss_phase_frame==0 && boss.patterns_done>=4));
        require(!player_is_hit && defeats==0,"All four final patterns run real pellets and lasers");if(argc==3) dump_bmp(argv[2],*fb);
        while(kikuri_original::boss_hp>0) {test_damage=true;tick();}require(boss.defeated && fades==1 && defeats==1,"Zero HP cleans actors and hands off defeat exactly once");kikuri_original::kikuri_main();require(defeats==1,"Defeat boundary is idempotent");
        kikuri_original::kikuri_free();require(entities.slots[0].images.empty() && entities.slots[1].images.empty() && ripples.images.empty(),"Kikuri frees soul/tear/ripple banks");
        fb->blit(1,0,0,bg.images[0],false);fb->clear(0,0);bomb.stage=original_palette;z_palette_set_all_show(original_palette);hud.backgrounds.clear();Pellets.native_reset_all();for(auto& laser:shootout_lasers) laser={};native_random_seed=1;unsigned before=waits;
        kikuri_original::kikuri_load();kikuri_original::boss_palette_snap();kikuri_original::kikuri_main();require(kikuri_original::boss_hp==14 && kikuri_original::kikuri_phase==2 && boss.patterns_done==0 && !boss.defeated && waits-before==entrance_waits,"Reload reconstructs fresh soul and static phase state");kikuri_original::kikuri_free();
        std::cout<<"Original Kikuri entrance, inverted-Y orb HP, soul/tear phases, final patterns and reload passed; entrance waits="<<entrance_waits<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
#include "konngara_backend.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        if(argc<2)throw std::invalid_argument("Usage: konngara_test ASSET_DIR [BMP]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));
        ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);
        CardEvents events;CardBackendBinding cb(events);std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hb(hud);
        BombBackend bomb{*fb,miko,miko,[](){}};BombBackendBinding bb(bomb,player);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);
        unsigned waits=0,stops=0,fades=0,releases=0;KonngaraEntrance graphics{*fb,argv[1],[](){},[&](unsigned n){waits+=n;},[](const std::string&){},[](){}};
        graphics.release_boss=[&](){++releases;konngara_original::konngara_free();};graphics.music_stop=[&](){++stops;};graphics.music_fade=[&](){++fades;};
        KonngaraBackend boss{graphics};KonngaraBinding kb(boss);std::uint32_t frame=1;BossHitBackend hit{frame,lives,hud_hp_decrement,[](){konngara_original::boss_palette_show();}};BossHitBinding hit_binding(hit);
        rank=RANK_NORMAL;native_pellet_speed=0;native_random_seed=1;score=0;player_left=300;player_invincible=true;player_is_hit=false;player_deflecting=false;test_damage=false;bomb_damaging=false;
        orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;Pellets.native_reset_all();for(auto& laser:shootout_lasers)laser={};for(unsigned i=0;i<8;++i){Shots.moving[i]=false;Shots.decay_frame[i]=0;}page_access(0);
        konngara_original::konngara_load_and_entrance(0);konngara_original::konngara_init();
        require(waits==710 && konngara_original::boss_hp==18 && konngara_original::boss_phase==0 && pellet_interlace,"Real Konngara entrance initializes eighteen HP and interlace");
        konngara_original::konngara_main();require(konngara_original::boss_phase==1,"Original phase zero initializes first attack phase");
        auto tick=[&](bool orb_hit=false){++frame;orb_cur_left=orb_prev_left=orb_hit ? 300 : 0;orb_cur_top=orb_prev_top=orb_hit ? 140 : 380;konngara_original::konngara_main();update_shootout_lasers();Pellets.unput_update_render();};
        for(unsigned i=0;i<25;++i)tick();tick(true);require(konngara_original::boss_hp==17 && score==10000,"Real orb hit awards ten thousand and one HP damage");
        auto visit=[&](int phase,unsigned count) {
            std::array<bool,12> seen{};unsigned ticks=0;
            while(true) {tick();if(konngara_original::boss_phase==phase && boss.pattern>=0 && boss.pattern<int(count))seen[boss.pattern]=true;bool all=true;for(unsigned i=0;i<count;++i)all&=seen[i];if(all)break;if(++ticks>60000)throw std::runtime_error("Konngara phase coverage stalled: expected="+std::to_string(phase)+" actual="+std::to_string(konngara_original::boss_phase)+" pattern="+std::to_string(boss.pattern)+" done="+std::to_string(boss.patterns_done));}
            require(!player_is_hit && !graphics.game_cleared,"Live attack phase retains invincible player and active game");
            std::cout<<"Konngara phase "<<phase<<" visited "<<count<<" patterns in "<<ticks+1<<" ticks\n";
        };
        visit(1,3);konngara_original::boss_hp=15;
        unsigned guard=0;while(konngara_original::boss_phase!=3){tick();if(++guard>2000)throw std::runtime_error("HP16 transition stalled");}visit(3,4);
        konngara_original::boss_hp=12;guard=0;while(konngara_original::boss_phase!=5){tick();if(++guard>2000)throw std::runtime_error("HP13 transition stalled");}visit(5,3);
        konngara_original::boss_hp=9;guard=0;while(konngara_original::boss_phase!=7){tick();if(++guard>2000)throw std::runtime_error("HP10 transition stalled");}visit(7,12);
        if(argc>=3)dump_bmp(argv[2],*fb);
        auto before=score;konngara_original::boss_hp=0;tick();require(graphics.game_cleared && score==before+50000 && waits==710+418 && stops==1 && fades==1 && releases==2,"Full boss defeat plays original scene and grants clear bonus once");
        auto after=score;konngara_original::konngara_main();require(score==after && releases==2,"Defeat handoff is idempotent");
        for(auto& slot:entities.slots)require(slot.images.empty(),"Boss entity banks freed at defeat");
        konngara_original::konngara_load_and_entrance(0);konngara_original::konngara_init();konngara_original::konngara_main();require(konngara_original::boss_hp==18 && konngara_original::boss_phase==1 && !graphics.game_cleared && boss.pattern==99,"Fresh load resets complete boss lifecycle");konngara_original::konngara_free();
        std::cout<<"Full Konngara phase/HP/actor/defeat lifecycle passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

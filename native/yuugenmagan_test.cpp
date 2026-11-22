#include "yuugenmagan_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
namespace y=yuugenmagan_original;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: yuugenmagan_test ASSET_DIR [BMP]");
    auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();
    auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS2.GRP"));
    auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));
    fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);
    ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);
    PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);
    CardEvents events;CardBackendBinding cb(events);std::int16_t lives=4;
    HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hb(hud);
    BombBackend bomb{*fb,miko,miko,[](){}};BombBackendBinding bb(bomb,player);auto original_palette=bomb.stage;
    BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);
    MissileBackend missiles{archive};MissileBinding mb(missiles);
    unsigned fades=0,defeats=0;YuugenmaganBackend boss{*fb,missiles,[&](){++fades;},[&](){++defeats;}};YuugenmaganBinding binding(boss);
    std::uint32_t frame=0;BossHitBackend hit{frame,lives,hud_hp_decrement,[](){y::boss_palette_show();}};BossHitBinding hit_binding(hit);
    auto tick=[&](int x=0,int top=380){++frame;orb_cur_left=orb_prev_left=x;orb_cur_top=orb_prev_top=top;y::yuugenmagan_main();Pellets.unput_update_render();};
    auto reset=[&](){
        fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);
        bomb.stage=original_palette;z_palette_set_all_show(original_palette);hud.backgrounds.clear();
        native_random_seed=1;native_pellet_speed=0;player_left=300;player_invincible=true;player_is_hit=false;player_deflecting=false;
        test_damage=false;bomb_damaging=false;score=0;frame=0;Pellets.native_reset_all();
        for(unsigned i=0;i<8;++i){Shots.moving[i]=false;Shots.decay_frame[i]=0;}
        y::yuugenmagan_load();y::boss_palette_snap();
        require(y::boss_hp==16 && y::boss_phase==0 && y::boss_phase_frame==0 && !boss.defeated,"Load/reset initializes sixteen HP and intro");
    };
    rank=RANK_NORMAL;reset();
    for(unsigned i=0;i<331;++i)tick();require(y::boss_phase==1 && y::boss_phase_frame==0,"Full main completes original 331-tick opening");
    tick(80,140);require(y::boss_hp==15 && score==5000 && y::boss_phase==2,"Real lateral eye orb hit awards 5000 and enters phase two");
    for(unsigned i=0;i<70;++i)tick();require(y::boss_phase==3,"Seventy-frame eye/palette transition enters missiles");
    unsigned ticks=0;bool saw_missiles=false,saw_pellets=false;std::array<bool,14> visited{};
    while(y::boss_phase!=13){
        visited.at(y::boss_phase)=true;tick();
        for(unsigned i=0;i<MISSILE_COUNT;++i)saw_missiles|=Missiles.native_at(i).flag!=0;
        saw_pellets|=Pellets.native_alive_count()>0;
        if(++ticks>20000)throw std::runtime_error("Full YuugenMagan timed phases stalled at "+std::to_string(y::boss_phase));
    }
    for(unsigned p=3;p<=12;++p)require(visited[p],"Timeout path visits every original phase through pentagram");
    require(saw_missiles && saw_pellets && y::boss_hp==15 && defeats==0,"Real missiles/pellets run without synthetic damage");
    unsigned loop_ticks=0;while(y::boss_phase==13){tick();if(++loop_ticks>2000)throw std::runtime_error("Regular pentagram failed to cycle");}
    require(y::boss_phase==10,"Final pentagram loops back to eye-opening phase");
    while(y::boss_phase!=13){tick();if(++loop_ticks>3000)throw std::runtime_error("Second pentagram failed to start");}
    if(argc>=3)dump_bmp(argv[2],*fb);
    // Real hit, forty-frame immunity, and cycling to the next vulnerable eye.
    auto old_score=score;tick(80,140);require(y::boss_hp==14 && score==old_score+5000,"Final phase accepts real eye hit");
    tick(80,140);require(y::boss_hp==14,"Immediate repeated eye contact is invincible");
    for(unsigned i=0;i<75;++i)tick();require(boss.eyes_open==2,"After hit final phase advances west to east eye");
    tick(530,140);require(y::boss_hp==13 && score==old_score+10000,"Next eye becomes hittable after transition");
    while(y::boss_hp>0){test_damage=true;tick();}
    require(boss.defeated && fades==1 && defeats==1,"Zero HP hands off defeat once");
    auto before=fb->pages;for(unsigned i=0;i<10;++i)tick();require(defeats==1 && fades==1 && before==fb->pages,"Completed boss is inert");
    y::yuugenmagan_free();require(entities.slots[0].images.empty() && missiles.sprites.images.empty() && missiles.slot==-1,"Free releases original eye and missile banks");
    // Reload must clear function-local phase/hit/pentagram state.
    reset();for(unsigned i=0;i<331;++i)tick();require(y::boss_phase==1 && y::boss_hp==16,"Reload starts fresh intro and HP");
    tick(80,140);require(y::boss_phase==2 && y::boss_hp==15 && score==5000,"Reload clears previous invincibility");
    y::yuugenmagan_free();
    // Damage-driven thresholds must also work, independently of timeout exits.
    for(unsigned difficulty=0;difficulty<4;++difficulty){
        rank=difficulty;reset();for(unsigned i=0;i<331;++i)tick();
        const int phases[]={1,3,5,7,9};const int thresholds[]={15,12,10,8,2};
        for(unsigned step=0;step<5;++step){
            require(y::boss_phase==phases[step],"Damage route reaches expected attack");
            while(y::boss_hp>thresholds[step]){test_damage=true;tick();}
            require(y::boss_phase==phases[step]+1,"HP threshold ends attack immediately");
            if(step<4){for(unsigned i=0;i<70;++i)tick();}
        }
        for(unsigned i=0;i<70;++i)tick();require(y::boss_phase==11,"Two HP enters shrink sequence");
        test_damage=true;tick();test_damage=true;tick();
        require(boss.defeated && y::boss_hp==0,"Shrink phase also supports zero-HP defeat");
        y::yuugenmagan_free();
    }
    std::cout<<"YuugenMagan full original intro, timeout phases 1-13, missiles, pentagram loop, real orb HP, eye cycling, defeat and reload passed; timeout ticks="<<ticks<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
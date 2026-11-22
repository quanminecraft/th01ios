#include "elis_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
namespace e=elis_original;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: elis_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS5.GRP"));fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);
    auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);CardEvents events;CardBackendBinding cb(events);std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hb(hud);BombBackend bomb{*fb,miko,miko,[](){}};BombBackendBinding bb(bomb,player);auto palette=bomb.stage;
    BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);MissileBackend missiles{archive};MissileBinding mb(missiles);unsigned waits=0,fades=0,defeats=0;
    ElisBackend boss{*fb,archive,missiles,[&](unsigned n){waits+=n;},[](unsigned){},[&](){++fades;},[&](){++defeats;}};ElisBinding binding(boss);std::uint32_t frame=1;BossHitBackend hit{frame,lives,hud_hp_decrement,[](){e::boss_palette_show();}};BossHitBinding hit_binding(hit);
    auto reset=[&](){for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);bomb.stage=palette;z_palette_set_all_show(palette);hud.backgrounds.clear();rank=RANK_NORMAL;native_pellet_speed=0;score=0;player_left=300;player_invincible=true;player_is_hit=false;player_deflecting=false;test_damage=false;bomb_damaging=false;frame=1;orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;Pellets.native_reset_all();for(auto& l:shootout_lasers)l={};for(unsigned i=0;i<8;++i){Shots.moving[i]=false;Shots.decay_frame[i]=0;}e::elis_load();e::boss_palette_snap();};
    auto tick=[&](bool orb_hit=false){++frame;orb_cur_left=orb_prev_left=orb_hit ? 300:0;orb_cur_top=orb_prev_top=orb_hit ? 120:380;e::elis_main();update_shootout_lasers();Pellets.unput_update_render();};
    for(unsigned pass=0;pass<2;++pass){
        reset();require(e::boss_hp==14 && e::boss_phase==0 && !boss.defeated && pellet_interlace,"Load reconstructs fourteen HP, intro and interlaced pellets");unsigned before_waits=waits;tick();require(e::boss_phase==1 && e::boss_phase_frame==0 && waits-before_waits==799,"Full main completes original 799-wait entrance");
        for(unsigned i=0;i<20;++i)tick();
        tick(true);require(e::boss_hp==13 && score==7000,"Real human orb hit awards seven thousand and one HP");tick(true);require(e::boss_hp==13,"Immediate repeated orb hit respects invincibility");
        while(e::boss_hp>10){test_damage=true;tick();}unsigned ticks=0;while(e::boss_phase!=3){
            if(e::boss_phase==1 && boss.pattern==0 && e::boss_phase_frame==51){
                std::uint32_t rng=native_random_seed;rng=rng*0x015a4e35u+1;int top=64+int((rng>>16)&0x7fff)%136;rng=rng*0x015a4e35u+1;int left=int((rng>>16)&0x7fff)%512;tick();
                auto& tile=boss.tiles[PTN_SLOT_BOSS_1][0];for(int y=0;y<32;++y)for(int x=0;x<32;++x)require(tile[y*32+x]==bg.images[0].color[(top+y)*640+left/8*8+x],"Native teleport evaluates random Y before X consistently across architectures");
            }else tick();if(++ticks>5000)throw std::runtime_error("Early Elis transition stalled");
        }require(e::boss_hp==10,"Ten HP and teleport enter phase three");
        while(e::boss_hp>6){test_damage=true;tick();}while(e::boss_phase!=5){tick();if(++ticks>10000)throw std::runtime_error("Elis bat transition stalled");}require(boss.bat && e::boss_hp==6,"Six HP enters final bat phase after central teleport/transform");
        if(pass==1){auto before_score=score;tick(true);require(e::boss_hp==5 && score==before_score+7000,"Real bat orb hit uses bat hitbox and same score");tick(true);require(e::boss_hp==5,"Bat hit also respects invincibility");}
        bool saw_pellets=false;while(boss.bat!=(pass==1)){tick();saw_pellets|=Pellets.native_alive_count()>0;if(++ticks>15000)throw std::runtime_error("Final Elis form transition stalled");}
        if(pass==0){require(saw_pellets,"Full final bat flight emits real pellets");if(argc>2)dump_bmp(argv[2],*fb);}
        while(e::boss_hp>0){test_damage=true;tick();}require(boss.defeated && fades==pass+1 && defeats==pass+1,"Zero HP cleans actors and invokes defeat once in either form");auto before=fb->pages;e::elis_main();require(before==fb->pages && defeats==pass+1,"Defeated boss stays inert");
        e::elis_free();require(entities.slots[0].images.empty() && entities.slots[1].images.empty() && entities.slots[2].images.empty() && boss.grc.images.empty() && missiles.slot==-1 && boss.tiles[PTN_SLOT_BOSS_1].empty(),"Free releases all Elis sprite/GRC/background/missile banks");
    }
    std::cout<<"Full original Elis intro, real orb HP/invincibility, early/final phases, both-form defeat and reload passed\n";
 }catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}
}
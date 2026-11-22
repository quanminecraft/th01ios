#include "yuugenmagan_eyes.hpp"
#include "boss_entity_backend.hpp"
#include "bomb_backend.hpp"
#include "hud_icons.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
    try{
        if(argc<2)throw std::invalid_argument("Usage: yuugenmagan_intro_test ASSET_DIR [BMP]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS2.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));
        ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hb(hud);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);
        std::uint64_t trace=1469598103934665603ull;
        for(unsigned difficulty=0;difficulty<4;++difficulty){
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=0;player_invincible=false;player_is_hit=false;Pellets.native_reset_all();fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);
            BombBackend bomb{*fb,miko,miko,[](){}};BombBackendBinding bb(bomb,player);YuugenmaganEyes eyes;unsigned ticks=0,emissions=0;
            while(true){
                ++ticks;player_is_hit=false;player_left=0;player_invincible=false;
                if(ticks==161)player_left=68;if(ticks==162)player_left=69;if(ticks==163)player_left=84;if(ticks==164)player_left=83;if(ticks==165)player_left=102;if(ticks==166)player_left=550;if(ticks==167){player_left=550;player_invincible=true;}
                bool done=eyes.advance_intro();require(done==(ticks==331),"Original phase-zero duration is 331 frames");
                if(ticks==99)require(eyes.eye(0).cel==0 && !eyes.eye(0).hitbox_active,"Eyes stay hidden before opening");
                if(ticks==100)require(eyes.eye(0).cel==1 && eyes.eye(1).hitbox_active && !eyes.eye(2).hitbox_active,"Lateral eyes open first");
                if(ticks==120)require(eyes.eye(0).cel==2 && eyes.eye(2).cel==1 && eyes.eye(2).hitbox_active,"South opening also advances lateral cels");
                if(ticks==140)require(eyes.eye(0).cel==6 && eyes.eye(2).cel==2 && eyes.eye(4).cel==1 && eyes.eye(4).hitbox_active,"North opening advances all eye groups");
                if(ticks==200)for(unsigned i=0;i<5;++i)require(eyes.eye(i).cel==3,"All eyes look down before closing");
                if(ticks>=161 && ticks<=167)require(bool(player_is_hit)==(ticks==162 || ticks==164 || ticks==166),"Strict laser boundaries, enlarged east hitbox and invincibility");
                if(ticks==240)for(unsigned i=0;i<5;++i)require(eyes.eye(i).cel==2 && !eyes.eye(i).hitbox_active,"All eyes close and deactivate hitboxes");
                unsigned expected=(ticks==240 && difficulty==2) ? 15 : (ticks==260 && difficulty==3) ? 25 : 0;require(Pellets.native_alive_count()==int(expected),"Original Hard/Lunatic closing volleys, including unreachable nested Lunatic branch");emissions+=expected;Pellets.native_reset_all();
                if(argc>=3 && difficulty==1 && ticks==220)dump_bmp(argv[2],*fb);
                for(const auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
                if(done)break;
            }
            require(bomb.stage[15].c.r==13 && bomb.stage[15].c.g==13 && bomb.stage[15].c.b==5,"Eight recolor steps end at original DD5 background color");
            require(eyes.eye(0).hitbox_active && eyes.eye(1).hitbox_active && !eyes.eye(4).hitbox_active && eyes.intro_frame()==0,"Phase one handoff activates only lateral eye hitboxes");
            require(emissions==(difficulty==2 ? 15u : difficulty==3 ? 25u : 0u) && native_random_seed==1,"Opening projectile totals consume no RNG");
            auto before=fb->pages;require(eyes.advance_intro() && fb->pages==before,"Completed intro does not replay");
            const unsigned periods[]={350,300,200,130};const unsigned offsets[]={10,25,40,60,80,110};const int speeds[]={24,28,32,44,50,68};
            unsigned lateral_emissions=0;shots.sound_commands.clear();
            for(unsigned tick=1;tick<=periods[difficulty]*2;++tick){
                player_left=tick<=periods[difficulty] ? 300 : 100;
                eyes.advance_lateral();int volley=-1;for(unsigned i=0;i<6;++i)if(tick%periods[difficulty]==offsets[i])volley=i;
                unsigned count=volley<0 ? 0 : volley==5 ? 2 : 4;require(Pellets.native_alive_count()==int(count),"Lateral rank period and six original firing offsets");
                for(unsigned i=0;i<count;++i){
                    const auto& p=Pellets.native_at(i);int origin=(volley==5 ? i : i/2)==0 ? 92 : 540;
                    require(p.cur_left.v==origin*16 && p.cur_top.v==160*16,"Both lateral eyes fire from original pupil-bottom muzzle");pixel_t vx,vy;subpixel_t speed=speeds[volley]+difficulty*6;
                    if(volley==5)vector2_between(pixel_t(origin),pixel_t(160),pixel_t(player_center_x()-PELLET_W),player_center_y(),vx,vy,speed);
                    else {unsigned char angle=iatan2(player_center_y()-160,player_center_x()-PELLET_W/2-origin)+(i%2 ? -12 : 12);vector2(vx,vy,speed,angle);}
                    require(p.velocity.x.v==vx && p.velocity.y.v==vy && p.from_group==(volley==5 ? PG_1_AIMED : PG_2_SPREAD_WIDE_AIMED),"Lateral velocities preserve six speeds, spread angles and distinct aimed target offsets");
                    trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;trace^=std::uint16_t(p.velocity.y.v);trace*=1099511628211ull;
                }
                lateral_emissions+=count;Pellets.native_reset_all();
            }
            require(lateral_emissions==44 && native_random_seed==1 && shots.sound_commands.empty(),"Two lateral cycles emit 44 projectiles without RNG or SE");
        }
        std::cout<<"YuugenMagan opening/lasers and lateral aimed spreads passed across four ranks; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

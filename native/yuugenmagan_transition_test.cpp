#include "yuugenmagan_eyes.hpp"
#include "boss_entity_backend.hpp"
#include "bomb_backend.hpp"
#include "hud_icons.hpp"
#include "grp.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
    try{
        if(argc!=2)throw std::invalid_argument("Usage: yuugenmagan_transition_test ASSET_DIR");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS2.GRP"));fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};HudIconsBinding hb(hud);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);BombBackend bomb{*fb,miko,miko,[](){}};BombBackendBinding bb(bomb,player);
        rank=1;player_left=0;player_invincible=true;player_is_hit=false;native_random_seed=1;YuugenmaganEyes eyes;while(!eyes.advance_intro()){};require(bomb.stage[15].c.r==13 && bomb.stage[15].c.g==13 && bomb.stage[15].c.b==5,"Opening provides DD5 transition palette");
        eyes.begin_transition(3,12,0,2);auto previous=bomb.stage;unsigned steps=0;
        for(unsigned frame=1;frame<=70;++frame){bool done=eyes.advance_transition();require(done==(frame==70) && eyes.transition_frame()==int(frame),"Original transition lasts70 frames");if(frame%10==0)require(bomb.stage[15].c.r==previous[15].c.r && bomb.stage[15].c.b==previous[15].c.b,"Every tenth frame skips recolor");else ++steps;
            require(bomb.stage[15].c.r==std::max(0,13-int(steps)) && bomb.stage[15].c.b==5+int(steps),"Stored transition palette decrements to zero but increments unclamped");require(bomb.current[15].c.r==std::max(0,13-int(steps)) && bomb.current[15].c.b==std::min(15,5+int(steps)),"Displayed palette clamps separately");
            if(frame==20)require(eyes.eye(0).cel==2 && eyes.eye(0).hitbox_active && eyes.eye(2).cel==1 && eyes.eye(2).hitbox_active,"At20 old eyes half-close and new eyes activate closed");if(frame==40)require(eyes.eye(0).cel==1 && !eyes.eye(0).hitbox_active && eyes.eye(2).cel==2,"At40 old eyes deactivate and new eyes half-open");if(frame==60)require(eyes.eye(0).cel==0 && eyes.eye(2).cel==6,"At60 old eyes hide and new eyes look ahead");previous=bomb.stage;
        }
        require(steps==63,"Exactly63 recolor increments");
        auto transition=[&](unsigned close,unsigned open,unsigned dec,unsigned inc,int r,int g,int b){eyes.begin_transition(close,open,dec,inc);for(unsigned i=0;i<70;++i)eyes.advance_transition();auto c=bomb.stage[15].c;require(c.r==r && c.g==g && c.b==b,"Original repeated transition component values");require(bomb.current[15].c.r==std::clamp(r,0,15) && bomb.current[15].c.g==std::clamp(g,0,15) && bomb.current[15].c.b==std::clamp(b,0,15),"Signed stored components clamp for display");};
        transition(12,3,1,0,63,0,68);transition(3,12,0,3,0,0,68);transition(12,16,2,1,0,63,5);transition(16,31,1,0,63,0,5);transition(30,1,1,0,126,0,5);transition(30,1,1,0,-67,0,5);transition(30,1,1,0,-4,0,5);transition(30,1,1,0,59,0,5);
        auto palette=fb->palette;eyes.begin_transition(1,2,3,3);for(unsigned i=0;i<70;++i)eyes.advance_transition();require(fb->palette==palette,"Sentinel component indices toggle eyes without recoloring");auto before=fb->pages;require(eyes.advance_transition() && fb->pages==before,"Completed transition is idempotent");require(native_random_seed==1 && !player_is_hit,"Transitions consume no RNG or player hit");
        std::cout<<"YuugenMagan eye toggles,63-step palette ramps and signed8 repeated overflow passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

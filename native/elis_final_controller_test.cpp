#include "laser_backend.hpp"
#include "elis_teleport.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "card_backend.hpp"
#include "grp.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: elis_final_controller_test ASSET_DIR");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS5.GRP"));
    auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);CardEvents events;CardBackendBinding cb(events);
    std::uint64_t trace=1469598103934665603ull;
    for(unsigned difficulty=0;difficulty<4;++difficulty){
        fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);rank=difficulty;native_pellet_speed=0;player_left=300;player_invincible=true;player_is_hit=false;player_deflecting=false;orb_cur_left=orb_prev_left=0;orb_cur_top=orb_prev_top=380;Pellets.native_reset_all();for(auto& l:shootout_lasers)l={};
        ElisTeleport elis(3);unsigned waits=0;elis.entrance(archive,[&](unsigned n){waits+=n;});elis.set_delay([](unsigned){});elis.begin_transform(true);for(unsigned i=0;i<101;++i)elis.advance_transform();elis.begin_final_phase();
        std::array<bool,5> bats{};std::array<bool,4> girls{};bool previous=true,saw_pellets=false,saw_laser=false;unsigned transitions=0,ticks=0;
        for(;ticks<25000;++ticks){
            elis.advance_final_phase();auto s=elis.final_state();require(s.bat_pattern>=0 && s.bat_pattern<=4 && s.human_pattern>=0 && s.human_pattern<=3,"Original final controller selects valid pattern IDs");
            if(s.bat_pattern)bats[s.bat_pattern]=true;if(s.human_pattern)girls[s.human_pattern]=true;if(s.bat!=previous){++transitions;previous=s.bat;require(s.frame==0,"Both final-phase transformations reset clock");}
            saw_pellets|=Pellets.native_alive_count()>0;for(auto& l:shootout_lasers)saw_laser|=l.alive;update_shootout_lasers();Pellets.unput_update_render();
            require(!player_is_hit,"Invincible player survives real final-controller actors");trace^=unsigned(s.frame)|(unsigned(s.bat)<<16)|(unsigned(s.bat_pattern)<<17)|(unsigned(s.human_pattern)<<20);trace*=1099511628211ull;
            if(transitions>=8 && bats[1] && bats[2] && bats[3] && bats[4] && girls[1] && girls[2] && girls[3] && s.bat)break;
        }
        require(ticks<25000 && saw_pellets && saw_laser && waits>1000,"Live final phase covers all four bat and three girl attacks, star, transforms and real actors");
        std::cout<<"Rank "<<difficulty<<" final-controller ticks="<<ticks+1<<" form changes="<<transitions<<'\n';
    }
    std::cout<<"Elis original final form/pattern controller passed across four ranks; trace "<<std::hex<<trace<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
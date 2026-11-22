#include "yuugenmagan_eyes.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
    try{
        if(argc<2)throw std::invalid_argument("Usage: yuugenmagan_lasers_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS2.GRP"));SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding sb(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);std::uint64_t trace=1469598103934665603ull;
        for(unsigned difficulty=0;difficulty<4;++difficulty){
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=300;player_invincible=true;player_is_hit=false;Pellets.native_reset_all();fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);YuugenmaganEyes eyes;eyes.begin_lasers();unsigned ticks=0,emissions=0;
            while(true){++ticks;bool done=eyes.advance_lasers();auto s=eyes.laser_state();
                if(ticks==40)require(s.subphase==1 && s.offset==0,"First sweep becomes active after original 40-frame preparation");
                if(ticks==189)require(s.subphase==2 && s.frame==0 && s.offset==16,"First sweep resets frame and retains original post-reset offset increment");
                if(ticks==378)require(s.subphase==4 && s.frame==0 && s.offset==16,"Second opposite sweep ends at original tick378");
                unsigned expected=0;if(ticks==189 || ticks==378)expected=difficulty>=2 ? 3 : 0;if(ticks==491)expected=difficulty==2 ? 3 : difficulty==3 ? 5 : 0;
                require(Pellets.native_alive_count()==int(expected),"Original rank-specific aimed volleys at sweep completion");emissions+=expected;Pellets.native_reset_all();
                if(argc>=3 && difficulty==1 && ticks==430)dump_bmp(argv[2],*fb);
                for(const auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
                if(done)break;if(ticks>500)throw std::runtime_error("YuugenMagan laser sequence stalled");
            }
            require(ticks==491 && eyes.laser_state().subphase==6 && emissions==(difficulty==2 ? 9u : difficulty==3 ? 11u : 0u) && native_random_seed==1 && !player_is_hit,"Three laser stages complete with original total/cadence and no RNG");auto before=fb->pages;require(eyes.advance_lasers() && fb->pages==before,"Completed sequence does not draw again");
        }
        {YuugenmaganEyes eyes;eyes.begin_lasers();player_left=300;player_invincible=false;player_is_hit=false;for(unsigned i=0;i<117;++i)eyes.advance_lasers();require(player_is_hit,"Actual sweep endpoint hits unprotected player");}
        std::cout<<"YuugenMagan original three-laser sequence, completion volleys and hitbox passed; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

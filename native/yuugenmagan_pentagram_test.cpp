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
        if(argc<2)throw std::invalid_argument("Usage: yuugenmagan_pentagram_test ASSET_DIR [BMP_PREFIX]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS2.GRP"));fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding sb(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);
        rank=RANK_NORMAL;native_random_seed=1;player_left=300;player_invincible=false;player_is_hit=false;YuugenmaganEyes eyes;eyes.begin_pentagram_shrink();std::uint64_t trace=1469598103934665603ull;
        const std::array<YuugenmaganPoint,5> initial{{{316,92},{220,204},{540,156},{92,156},{412,204}}};
        unsigned ticks=0;while(true){
            ++ticks;bool done=eyes.advance_pentagram_shrink();auto state=eyes.pentagram_state();
            require(done==(ticks==221),"Original strict-greater shrink finishes at frame221");
            if(ticks<100)require(state.subphase==0 && state.distance==0,"Pentagram preparation waits to frame100");
            if(ticks==100){require(state.subphase==1,"Pentagram first connects eye corners");for(unsigned i=0;i<5;++i)require(state.points[i].x==initial[i].x && state.points[i].y==initial[i].y,"Original off-center iris corner positions");}
            if(ticks==140)require(state.subphase==2 && state.distance==0,"Shrink movement starts after frame140");
            if(ticks>=141){unsigned steps=(ticks-141)/4+1;require(state.distance==int(steps*8),"Eight-pixel shrink steps every fourth frame");}
            if(ticks==217)require(state.subphase==2 && state.distance==160,"Exact target distance does not finish original strict comparison");
            if(argc>=3 && (ticks==100 || ticks==181 || ticks==221))dump_bmp(std::string(argv[2])+"-"+std::to_string(ticks)+".bmp",*fb);
            for(const auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
            if(done)break;
        }
        auto result=eyes.pentagram_state();const std::array<YuugenmaganPoint,5> final{{{316,176},{276,288},{372,212},{260,212},{356,288}}};
        require(result.subphase==3 && result.distance==168,"Original shrink overshoots to168 rather than clamping160");for(unsigned i=0;i<5;++i)require(result.points[i].x==final[i].x && result.points[i].y==final[i].y,"Original integer shrink factors preserve off-center final shape");
        require(native_random_seed==1 && shots.sound_commands.empty() && !player_is_hit,"Shrink uses no RNG/SE and endpoints remain above player");auto before=fb->pages;require(eyes.advance_pentagram_shrink() && fb->pages==before,"Completed shrink is idempotent");
        std::cout<<"YuugenMagan eye-connected pentagram, shrink cadence and original overshoot passed; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

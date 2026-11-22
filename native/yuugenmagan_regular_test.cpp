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
        if(argc<2)throw std::invalid_argument("Usage: yuugenmagan_regular_test ASSET_DIR [BMP_PREFIX]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS2.GRP"));SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding sb(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);std::uint64_t trace=1469598103934665603ull;
        for(unsigned difficulty=0;difficulty<4;++difficulty)for(bool grow:{false,true}){
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=300;player_invincible=true;player_is_hit=false;Pellets.native_reset_all();fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);YuugenmaganEyes eyes;eyes.begin_regular_pentagram(grow);std::array<bool,10> visited{};unsigned ticks=0,volleys=0;bool saw_slam=false,saw_growth=false;
            while(true){
                auto before=eyes.regular_state();visited.at(before.phase)=true;int next_frame=before.frame+1;const unsigned intervals[]={24,14,10,8};bool firing=next_frame%intervals[difficulty]==0 && before.phase!=6 && before.phase!=9;
                ++ticks;bool done=eyes.advance_regular_pentagram();auto state=eyes.regular_state();
                require(Pellets.native_alive_count()==(firing ? 5 : 0),"Five radial pellets at original rank interval, suppressed only during slam");
                if(firing){++volleys;for(unsigned i=0;i<5;++i){auto& p=Pellets.native_at(i);pixel_t vx,vy;unsigned char angle=iatan2(before.points[i].y-before.y,before.points[i].x-before.x);vector2(vx,vy,subpixel_t(48+difficulty*6),angle);require(p.cur_left.v==before.points[i].x*16 && p.cur_top.v==before.points[i].y*16 && p.velocity.x.v==vx && p.velocity.y.v==vy,"Radial pellets leave pre-update pentagram corners");trace^=std::uint16_t(p.velocity.x.v);trace*=1099511628211ull;trace^=std::uint16_t(p.velocity.y.v);trace*=1099511628211ull;}Pellets.native_reset_all();}
                if(ticks==99)require(state.phase==(grow ? 7 : 0) && state.radius==64 && state.y==189,"Preparation retains original radius and center");
                if(ticks==100)require(state.frame==0 && state.phase==(grow ? 8 : 1) && state.angle==2 && state.radius==(grow ? 65 : 64),"Preparation transition updates spin/growth on same frame zero");
                if(state.phase==6 || state.phase==9){saw_slam=true;require(state.y>=189,"Slam heads towards player bottom");}
                if(state.phase==8)saw_growth=true;
                if(grow && state.phase==9)require(state.radius==115,"Original frame-zero growth produces radius115");
                if(argc>=3 && difficulty==1 && ticks==(grow ? 310u : 350u))dump_bmp(std::string(argv[2])+(grow ? "-grow.bmp" : "-spin.bmp"),*fb);
                for(const auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
                if(done){require(state.frame==0 && state.handoff && state.phase==(grow ? 0 : 7) && state.y>=420,"Slam exits through original phase10 respawn handoff");break;}
                if(ticks>4000)throw std::runtime_error("Pentagram spin/slam stalled");
            }
            const unsigned spin_volleys[]={44,77,110,137},grow_volleys[]={12,21,30,37};require(ticks==(grow ? 532u : 1332u) && volleys==(grow ? grow_volleys[difficulty] : spin_volleys[difficulty]),"Original fixed-target duration and rank-specific radial volley totals");
            require(saw_slam && volleys>0 && native_random_seed==1 && !player_is_hit,"Complete live pattern has radial projectiles, slam and no RNG/invincible damage");
            if(grow)require(visited[7] && visited[8] && visited[9] && saw_growth,"Growth path visits preparation/grow/slam");else for(unsigned i=0;i<=6;++i)require(visited[i],"Spin path visits five alternating spins and slam");auto before=fb->pages;require(eyes.advance_regular_pentagram() && fb->pages==before,"Completed handoff is idempotent");
            std::cout<<"Rank "<<difficulty<<(grow ? " grow" : " spin")<<" ticks="<<ticks<<" radial volleys="<<volleys<<'\n';
        }
        std::cout<<"YuugenMagan original regular pentagram spin/grow/slam and radial fire passed; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

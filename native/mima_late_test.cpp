#include "mima_square_patterns.hpp"
#include "line_backend.hpp"
#include "pellet_backend.hpp"
#include "th01/main/bullet/missile.hpp"
#include "th01/main/bullet/laser_s.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* reason) {if(!ok) throw std::runtime_error(reason);}
int main() {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding binding(shots);
        std::uint64_t trace=1469598103934665603ull;
        for(unsigned difficulty=0;difficulty<4;++difficulty) for(auto attack:{MimaSquareAttack::halfcircle_missiles,MimaSquareAttack::slow_spray,MimaSquareAttack::lasers}) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=304;Pellets.native_reset_all();Missiles.reset();for(auto& laser:shootout_lasers) laser={};shots.sound_commands.clear();
            fb->clear(0,0);fb->clear(1,0);page_access(0);unsigned puts=0,unputs=0;std::array<MimaPoint,4> old{},fresh{};std::vector<bool> casting;
            auto lines=[&](bool restore,const std::array<MimaPoint,4>& points) {render_mima_square(restore,points);if(restore) {old=points;++unputs;}else {fresh=points;++puts;}for(auto p:points) {trace^=std::uint16_t(p.x);trace*=1099511628211ull;trace^=std::uint16_t(p.y);trace*=1099511628211ull;}};
            MimaSquarePattern pattern(attack,256,120,lines,[&](bool cast) {casting.push_back(cast);});unsigned ticks=0;
            do {
                ++ticks;bool done=pattern.advance();
                if(attack==MimaSquareAttack::halfcircle_missiles && ticks==184) {
                    require(Missiles.native_alive_count()==4,"Halfcircle starts four missiles on frame 184");
                    for(unsigned i=0;i<4;++i) {auto m=Missiles.native_at(i);require(m.left==TO_SP(old[i].x) && m.top==TO_SP(old[i].y) && m.velocity_x==(difficulty<2 ? 4 : 5)*16 && m.velocity_y==0,"Halfcircle retains old corners and original decimal-speed truncation");}
                }
                if(attack==MimaSquareAttack::slow_spray && (ticks==184 || ticks==272)) {
                    unsigned index=ticks==184 ? 0 : 44;unsigned angle=ticks==184 ? 116 : 8;
                    subpixel_t speed=32+difficulty*8+difficulty*6;
                    for(unsigned i=0;i<4;++i) {const auto& p=Pellets.native_at(index+i);require(p.cur_left.to_pixel()==old[i].x && p.cur_top.to_pixel()==old[i].y && p.velocity.x.v==polar_x(subpixel_t(0),speed,angle) && p.velocity.y.v==polar_y(subpixel_t(0),speed,angle),"Slow spray fires at old corners and reverses sweep after frame 270");}
                }
                if(attack==MimaSquareAttack::lasers && ticks==181) require(shootout_lasers[1].alive && shootout_lasers[1].origin_left.to_pixel()==fresh[1].x,"First laser uses current rendered corner");
                if(attack==MimaSquareAttack::lasers && ticks==184) {
                    const auto& laser=shootout_lasers[0];require(laser.alive && laser.origin_left.to_pixel()==fresh[0].x && laser.origin_y.to_pixel()==fresh[0].y,"Laser on rotation frame uses fresh corners instead of old ones");
                    require(laser.ray_extend_speed==(difficulty<2 ? 6 : 7) && laser.width_cel==3 && laser.moveout_at_age==20,"Original laser speed truncation, width and moveout delay");
                    require(old[0].x!=fresh[0].x || old[0].y!=fresh[0].y,"Laser old/new corners differ on rotation frame");
                }
                if(done) break;if(ticks>400) throw std::runtime_error("Late Mima pattern stalled");
            } while(true);
            if(attack==MimaSquareAttack::halfcircle_missiles) require(ticks==341 && puts==30 && unputs==31 && Missiles.native_alive_count()==40 && shots.sound_commands.size()==11,"Halfcircle full original timing, forty missiles and sound count");
            if(attack==MimaSquareAttack::slow_spray) require(ticks==371 && puts==34 && unputs==35 && Pellets.native_alive_count()==96 && shots.sound_commands.size()==25,"Slow spray full original timing and ninety-six pellets");
            if(attack==MimaSquareAttack::lasers) {require(ticks==301 && puts==25 && unputs==26 && shots.sound_commands==std::vector<unsigned>{8} && casting==std::vector<bool>{true},"Laser pattern ends at 301 and leaves casting/meteor state as original");for(unsigned i=0;i<10;++i) require(shootout_lasers[i].alive==(i<4),"Repeated spawn calls preserve already-live first four laser slots");}
            require(native_random_seed==1,"Late square attacks do not consume RNG");
        }
        std::cout<<"All-rank Mima halfcircle, slow reverse spray and current-corner lasers passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
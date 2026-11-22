#include "mima_square_patterns.hpp"
#include "line_backend.hpp"
#include "pellet_backend.hpp"
#include "th01/main/bullet/missile.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* reason) {if(!ok) throw std::runtime_error(reason);}
int main() {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding binding(shots);
        rank=RANK_NORMAL;native_pellet_speed=0;native_random_seed=1;player_left=304;
        Pellets.native_reset_all();Missiles.reset();
        unsigned puts=0,unputs=0;std::array<MimaPoint,4> old{},fresh{};std::uint64_t trace=1469598103934665603ull;
        auto lines=[&](bool restore,const std::array<MimaPoint,4>& points) {render_mima_square(restore,points);if(restore) {++unputs;old=points;}else {++puts;fresh=points;}for(auto p:points) {trace^=std::uint16_t(p.x);trace*=1099511628211ull;trace^=std::uint16_t(p.y);trace*=1099511628211ull;}};
        {
            MimaSquarePattern pattern(MimaSquareAttack::pellets,256,120,lines);unsigned ticks=0;
            do {
                ++ticks;bool done=pattern.advance();
                if(ticks==99) require(puts==0 && Pellets.native_alive_count()==0 && shots.sound_commands.empty(),"First pattern waits until frame one hundred");
                if(ticks==100) require(shots.sound_commands==std::vector<unsigned>{8},"Pattern start plays SE eight once");
                if(ticks==184) {
                    require(Pellets.native_alive_count()==4,"First aimed volley starts only after square grows past seal");
                    for(unsigned i=0;i<4;++i) {const auto& pellet=Pellets.native_at(i);require(pellet.cur_left.to_pixel()==old[i].x && pellet.cur_top.to_pixel()==old[i].y,"Pellets spawn at previous unput corners, not freshly rotated ones");}
                    require(old[0].x!=fresh[0].x || old[0].y!=fresh[0].y,"Old and new firing corners differ");
                }
                if(done) break;if(ticks>400) throw std::runtime_error("Pellet pattern stalled");
            } while(true);
            require(ticks==361 && puts==33 && unputs==34 && Pellets.native_alive_count()==92,"Full original aimed/static pattern timing, line events and volley count");
            require(shots.sound_commands.size()==93 && native_random_seed==1,"Pattern emits one start and ninety-two fire SE without RNG");
        }
        puts=unputs=0;shots.sound_commands.clear();Pellets.native_reset_all();Missiles.reset();
        {
            MimaSquarePattern pattern(MimaSquareAttack::missiles,256,120,lines);unsigned ticks=0;
            do {
                ++ticks;if(ticks==224) player_left=100;if(ticks==225) player_left=500;
                bool done=pattern.advance();
                if(ticks==248) {require(Missiles.native_alive_count()==4,"First missile volley at original frame 248");for(unsigned i=0;i<4;++i) {auto m=Missiles.native_at(i);require(m.left==TO_SP(old[i].x) && m.top==TO_SP(old[i].y) && m.velocity_x<0,"Missiles use old corners and target captured at frame 224");require(m.velocity_x%16==0 && m.velocity_y%16==0,"Original missile pattern truncates subpixel vector to pixel velocity");}}
                if(done) break;if(ticks>350) throw std::runtime_error("Missile pattern stalled");
            } while(true);
            require(ticks==321 && puts==28 && unputs==29 && Missiles.native_alive_count()==40,"Full missile square pattern timing and forty spawned missiles");
            require(shots.sound_commands.size()==11 && native_random_seed==1,"Missile pattern emits start plus ten volley SE without RNG");
        }
        puts=unputs=0;shots.sound_commands.clear();Pellets.native_reset_all();Missiles.reset();
        {
            std::vector<bool> casting;std::vector<std::array<MimaPoint,4>> old_squares;
            auto doubled_lines=[&](bool restore,const std::array<MimaPoint,4>& points) {lines(restore,points);if(restore) old_squares.push_back(points);};
            MimaSquarePattern pattern(MimaSquareAttack::two_squares,256,120,doubled_lines,[&](bool cast) {casting.push_back(cast);});
            unsigned ticks=0;
            do {
                ++ticks;old_squares.clear();bool done=pattern.advance();
                if(ticks==49) require(casting.empty(),"Third pattern does not cast before frame fifty");
                if(ticks==50) require(casting==std::vector<bool>{true},"Third pattern renders casting pose on frame fifty");
                if(ticks==184) {
                    require(Pellets.native_alive_count()==8 && old_squares.size()==2,"First dual-square volley creates eight cloud pellets");
                    for(unsigned corner=0;corner<4;++corner) for(unsigned square=0;square<2;++square) {const auto& pellet=Pellets.native_at(corner*2+square);require(pellet.cur_left.to_pixel()==old_squares[square][corner].x && pellet.cur_top.to_pixel()==old_squares[square][corner].y && pellet.cloud_frame==1 && !pellet.moving,"Dual-square pellets interleave old regular/flipped corners in cloud state");}
                }
                require(!Pellets.spawn_with_cloud,"Original cloud-spawn switch is restored after each pattern frame");
                if(done) break;if(ticks>350) throw std::runtime_error("Dual-square pattern stalled");
            } while(true);
            require(ticks==321 && puts==56 && unputs==58,"Dual-square geometry loops follow original timing");
            require(Pellets.native_alive_count()==100 && shots.sound_commands.size()==73,"Cloud pool fills at one hundred while all eighteen volleys still emit sound");
            require(casting==std::vector<bool>({true,false}) && native_random_seed==1,"Pattern completion restores meteor animation without consuming RNG");
            auto sound_count=shots.sound_commands.size();require(pattern.advance() && shots.sound_commands.size()==sound_count,"Finished pattern cannot spawn again");
        }
        fb->clear(0,3);fb->clear(1,5);page_access(0);
        {
            MimaSquarePattern pattern(MimaSquareAttack::pellets,256,120);
            for(unsigned tick=0;tick<104;++tick) pattern.advance();
            bool visible=false;for(int y=140;y<260;++y) for(int x=250;x<390;++x) visible|=fb->pixel(0,x,y)==7;
            require(visible,"Empty callback selects actual native square rasterizer");
        }
        std::cout<<"Original Mima square pellet/missile schedules, old corners and target snapshot passed; line trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
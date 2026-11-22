#include "th01/main/bullet/line.hpp"
#include "th01/main/player/player.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){
    try{
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding sb(shots);player_left=300;player_invincible=false;
        for(int right:{300,301,331,332})for(int bottom:{383,384,385,399}){
            player_is_hit=false;fb->clear(0,0);linebullet_put_patterned_and_hittest(10,64,right,bottom,7,0xe0);
            require(bool(player_is_hit)==(right>300 && right<332 && bottom>384),"Original endpoint-only strict X and lower Y collision bounds");
        }
        player_is_hit=false;linebullet_put_and_hittest(310,390,600,390,7);require(!player_is_hit,"Line crossing player does not hit when endpoint is outside player X");
        player_is_hit=false;linebullet_put_and_hittest(10,64,310,399,7);require(player_is_hit,"Endpoint collision extends upward independently of line origin");
        player_invincible=true;player_is_hit=false;linebullet_put_patterned_and_hittest(10,64,310,399,7,0xe0);require(!player_is_hit,"Invincibility suppresses line-bullet damage");
        fb->clear(0,0);page_access(0);linebullet_put_patterned_and_hittest(100,100,100,104,7,0xe0);
        for(int y=100;y<=104;++y)for(int x=96;x<112;++x)require(fb->pixel(0,x,y)==(x>=100 && x<=102 ? 7 : 0),"Original three-dot vertical laser pattern");
        graph_r_line(120,100,120,104,6);require(fb->pixel(0,120,102)==6 && fb->pixel(0,121,102)==0,"Pattern state resets before ordinary line rendering");
        fb->clear(0,0);fb->clear(1,5);linebullet_put_patterned_and_hittest(310,88,520,399,7,0xe0);linebullet_unput(310,88,520,399);
        require(current_render_page()==0 && fb->pixel(0,304,88)==5,"Original sloppy unput restores background chunks");
        std::uint64_t trace=1469598103934665603ull;
        for(auto end:std::array<MimaPoint,4>{{{0,399},{160,399},{480,399},{639,399}}}){
            fb->clear(0,3);graph_r_line_patterned(316,92,end.x,end.y,7,0xe0);for(auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
        }
        std::cout<<"Original line-bullet patterned rendering, endpoint collision and restore passed; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

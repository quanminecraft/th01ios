#include "kikuri_ripple.hpp"
#include "th01/main/player/player.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        if(argc<2 || argc>3) throw std::invalid_argument("Usage: kikuri_ripple_test ASSET_DIR [BMP_PREFIX]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN")),sprites=read_sprites(asset(archive,"TAMAYEN.PTN"));
        ShotBackend shot{*fb,stg,{}};ShotBackendBinding shot_binding(shot);KikuriRippleBackend ripple{*fb,sprites};KikuriRippleBinding binding(ripple);
        auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS4.GRP"));fb->palette=bg.palette;std::uint64_t trace=1469598103934665603ull;
        for(int left:{304,312,307}) {
            for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);page_access(0);player_invincible=true;player_is_hit=false;shot.sound_commands.clear();
            std::int8_t frame=1;unsigned ticks=0;
            do {
                kikuri_ripple_advance(left,384,frame);++ticks;
                require(!ripple.sloppy && current_render_page()==0 && !player_is_hit,"Ripple resets sloppy flag and leaves page zero selected without invincible collision");
                if(frame==24 && argc==3) dump_bmp(std::string(argv[2])+"-"+std::to_string(left)+".bmp",*fb);
                for(const auto& plane:fb->pages[0]) for(auto byte:plane) {trace^=byte;trace*=1099511628211ull;}
                if(ticks>80) throw std::runtime_error("Ripple stalled");
            } while(frame!=0);
            require(ticks==74 && shot.sound_commands==std::vector<unsigned>({7,7,7}),"Original tear-frame-one ripple completes after seventy-four updates and three SE pulses");
            require(fb->pages[0]==fb->pages[1],"Full ripple lifetime restores background after final empty cels");
        }
        player_invincible=false;player_is_hit=false;player_left=291;
        require(kikuri_ripple_hit_test(307,0) && player_is_hit,"Ripple internal-coordinate left boundary is inclusive");
        player_is_hit=false;player_left=290;require(!kikuri_ripple_hit_test(307,0) && !player_is_hit,"Hitbox retains unaligned internal left instead of rounded sprite position");
        player_left=307;require(kikuri_ripple_hit_test(307,0),"Ripple internal-coordinate right boundary is inclusive");
        player_is_hit=false;player_left=308;require(!kikuri_ripple_hit_test(307,0),"Right neighbor lies outside center ripple");
        player_invincible=true;player_left=291;require(!kikuri_ripple_hit_test(307,0),"Invincibility suppresses ripple hit detection");
        std::cout<<"Original Kikuri ripple timing, aligned/striped rendering, sound and internal hitbox passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
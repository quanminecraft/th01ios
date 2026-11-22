#include "sariel_entrance.hpp"
#include "physics_types.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: sariel_entrance_test ASSET_DIR [BMP]");auto fb=std::make_unique<Framebuffer>();auto low=std::make_unique<Framebuffer>(),high=std::make_unique<Framebuffer>(),battle=std::make_unique<Framebuffer>();
    auto low_grp=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_L.GRP")),high_grp=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_H.GRP")),battle_grp=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A1.GRP"));low->blit(0,0,0,low_grp.images[0],false);high->blit(1,0,0,high_grp.images[0],false);battle->blit(0,0,0,battle_grp.images[0],false);
    unsigned waits=0,delays=0,total_ms=0,resets=0;std::vector<std::string> music,se;unsigned plays=0;SarielEntrance entrance{*fb,argv[1]};entrance.frame_seed=123;
    entrance.reset_stage_objects=[&](){++resets;require(fb->pages[0]==low->pages[0] && fb->pages[1]==high->pages[1],"Original low/high entrance images load before stage reset");};
    entrance.wait=[&](unsigned n){require(n==1,"Original entrance waits one VSync per callback");++waits;if(waits<=800 && waits%2==0){unsigned row=400-waits/2;require(fb->display_scroll_y==int(row),"Original descending scroll register sequence");for(unsigned p=0;p<4;++p)for(unsigned x=0;x<80;++x)require(fb->pages[0][p][row*80+x]==high->pages[1][p][row*80+x],"Each scrolling line copies high page into low page");}if(waits==800)require(fb->pages[0]==high->pages[1],"All four hundred lines copied before diagonal transition");};
    entrance.delay_ms=[&](unsigned ms){require(ms==40,"Original diagonal transition step is forty milliseconds");++delays;total_ms+=ms;if(delays==1){for(unsigned p=0;p<4;++p)for(unsigned y=0;y<8;++y){unsigned mask=y%2 ? 0xaa:0x55;require(fb->pages[0][p][y*80]==((battle->pages[0][p][y*80]&~mask)|(high->pages[1][p][y*80]&mask)),"First diagonal tile uses original interlace mask");}}};
    entrance.music_load=[&](const std::string& name){music.push_back(name);};entrance.se_load=[&](const std::string& name){se.push_back(name);};entrance.music_play=[&](){++plays;};
    for(unsigned run=0;run<2;++run){waits=delays=total_ms=0;fb->text_overlay.assign(640*400,0xffffffffu);native_random_seed=999;entrance.run();require(waits==801 && delays==130 && total_ms==5200,"Original scroll and diagonal transition timings");require(fb->pages[0]==battle->pages[0] && fb->pages[1]==battle->pages[0] && fb->palette==battle_grp.palette,"Both pages and final palette equal battle image");require(fb->display_scroll_y==0 && fb->text_overlay.empty() && entrance.selected_page==0 && entrance.pellet_interlace && native_random_seed==123,"Entrance restores scroll/page, text, interlace and resident RNG seed");}
    require(resets==2 && plays==2 && music==std::vector<std::string>({"TENSI.MDT","TENSI.MDT"}) && se==std::vector<std::string>({"zigoku.mde","zigoku.mde"}),"Original stage reset and music/SE load/play commands repeat");if(argc>2)dump_bmp(argv[2],*fb);
    std::cout<<"Original Sariel scrolling entrance, diagonal masks/timing, music commands, final background and replay passed\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
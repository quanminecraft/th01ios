#include "konngara_entrance.hpp"
#include "framebuffer_dump.hpp"
#include "grp.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        if(argc<2) throw std::invalid_argument("Usage: konngara_entrance_test ASSET_DIR [BMP]");
        auto fb=std::make_unique<Framebuffer>();unsigned frames=0,calls=0,resets=0,plays=0;std::string track;std::vector<int> scroll_trace;bool defeating=false;std::vector<std::uint32_t> flashes;
        KonngaraEntrance entrance{*fb,argv[1],[&]() {++resets;},[&](unsigned n) {frames+=n;++calls;scroll_trace.push_back(fb->display_scroll_y);if(defeating && flashes.size()<9) {require(n==10,"Kuji images hold ten frames");flashes.push_back(fb->palette[0]);if(argc>=3 && flashes.size()==1)dump_bmp(std::string(argv[2])+"-defeat-7.bmp",*fb);}},[&](const std::string& name) {track=name;},[&]() {++plays;}};
        entrance.run();
        require(calls==498 && frames==710,"Original blocking entrance timing");
        require(std::vector<int>(scroll_trace.begin(),scroll_trace.begin()+6)==std::vector<int>({0,32,64,96,128,159}),"Original scroll decelerates at quarter-screen threshold");
        require(entrance.pellet_interlace && resets==1 && plays==1 && track=="ALICE.MDT","Original entrance requests interlace, boss object reset and Alice music");
        require(entrance.selected_page==0 && fb->display_scroll_y==0 && fb->pages[0]==fb->pages[1],"Entrance ends with synchronized pages and no scroll");
        require(entrance.sound_commands==std::vector<unsigned>({9,9,9,9}),"First shake emits original four SE-9 commands");
        for(unsigned i=0;i<7;++i) require(entrance.effects[i].images.size()==1,"Seven slash effect slots are loaded");
        for(unsigned i=0;i<3;++i) require(!entrance.entities[i].images.empty(),"Head/face BOS banks loaded");
        auto scroll=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS7_D1.GRP"));
        require(entrance.stage_palette==scroll.palette,"Preserves original stage-palette snapshot before boss palette load");
        require(fb->palette[9]==0xff0099aau,"Siddham palette completes original white-to-color fade");
        if(argc>=3) dump_bmp(argv[2],*fb);
        auto pages=fb->pages;auto palette=fb->palette;frames=calls=0;entrance.sound_commands.clear();entrance.run();
        require(fb->pages==pages && fb->palette==palette && resets==2 && plays==2,"Fresh entrance reload has no stale visual state");
        std::cout<<"Konngara original entrance passed: waits="<<calls<<" frames="<<frames<<'\n';
        unsigned releases=0,stops=0,fades=0;entrance.release_boss=[&](){++releases;};entrance.music_stop=[&](){++stops;};entrance.music_fade=[&](){++fades;};entrance.score=1234;entrance.sound_commands.clear();frames=calls=0;defeating=true;
        entrance.run_defeat();
        require(calls==337 && frames==418 && entrance.sound_commands==std::vector<unsigned>(44,7),"Original explicit defeat waits and SE schedule");
        require(entrance.game_cleared && entrance.score==51234 && releases==2 && stops==1 && fades==1,"Defeat grants 50000 once, clears game and preserves original duplicate safe frees/music commands");
        require(flashes.size()==9,"All nine Kuji images displayed");for(unsigned i=0;i<9;++i)require(flashes[i]==(i%2 ? 0xffffffffu : 0xff000000u),"Kuji alternates palette-zero black/white");
        require(fb->display_scroll_y==0 && fb->text_overlay.empty(),"Defeat ends with zero scroll and cleared text overlay");for(auto col:fb->palette)require(col==0xff000000u,"Final scroll fades to black");
        for(auto& bank:entrance.entities)require(bank.images.empty(),"Defeat frees entity banks");for(auto& bank:entrance.effects)require(bank.images.empty(),"Defeat frees GRZ slots");
        auto waits=calls;entrance.run_defeat();require(calls==waits && entrance.score==51234 && releases==2,"Completed defeat is idempotent");
        std::cout<<"Konngara original defeat passed: waits="<<calls<<" frames="<<frames<<" SE="<<entrance.sound_commands.size()<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

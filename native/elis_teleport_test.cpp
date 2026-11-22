#include "elis_teleport.hpp"
#include "boss_entity_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: elis_teleport_test ASSET_DIR [BMP]");
    auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto expected=std::make_unique<Framebuffer>();
    auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS5.GRP"));fb->palette=bg.palette;
    for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);
    auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);
    BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);page_access(0);
    std::uint64_t trace=1469598103934665603ull;
    {
        ElisTeleport elis(3);auto girl=elis.girl();auto bat=elis.bat();
        require(girl.left==256 && girl.top==96 && girl.cel==0 && girl.hittable,"Original human base position, still cel and orb hitbox");
        require(bat.left==256 && bat.top==96 && bat.hittable,"Original redundant bat setup position");
        require(entities.slots[0].images.size()==6 && entities.slots[1].images.size()==3 && entities.slots[2].images.size()==3,"Three real human/wave, attack and bat sprite banks");
        std::uint32_t rng=3;
        const int lefts[]={113,420},tops[]={137,80};
        for(unsigned pass=0;pass<2;++pass){
            auto original_girl=elis.girl();elis.begin(lefts[pass],tops[pass]);
            for(unsigned tick=1;tick<=81;++tick){
                bool done=elis.advance();require(done==(tick==81),"Original teleport duration is 81 ticks");girl=elis.girl();
                require(girl.hittable==(tick<20 || tick>=76),"Wave teleport deactivates orb hitbox from tick twenty through seventy-five");
                require(girl.left==(tick<52 ? original_girl.left:lefts[pass]) && girl.top==(tick<52 ? original_girl.top:tops[pass]),"Position changes at original frame fifty-two only");
                unsigned cel=tick<20 ? 0:tick<28 ? 2:tick<36 ? 3:tick<60 ? 4:tick<68 ? 3:tick<76 ? 2:0;
                require(girl.cel==int(cel),"Original wave disappearance/reappearance cel schedule");
                if(tick==44){expected->clear(1,0);expected->blit(1,0,0,bg.images[0],false);require(fb->pages[1]==expected->pages[1],"Old sprite is removed from background page");}
                if(tick==76){expected->clear(1,0);expected->blit(1,0,0,bg.images[0],false);expected->blit(1,lefts[pass]/8*8,tops[pass],entities.slots[0].images[0],true);require(fb->pages[1]==expected->pages[1],"Background cache restores target region and commits still sprite on page one");}
                if(tick%4==0)for(unsigned i=0;i<10;++i)rng=rng*0x015a4e35u+1;
                require(native_random_seed==rng,"Five star positions consume ten RNG samples every fourth frame, even after stars stop drawing");
                for(const auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
                if(argc>=3 && pass==0 && tick==60)dump_bmp(argv[2],*fb);
            }
            require(elis.frame()==0,"Completed teleport resets original phase clock");auto before=fb->pages;require(elis.advance() && before==fb->pages && native_random_seed==rng,"Completed helper is inert until next teleport");
        }
    }
    for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);
    {
        ElisTeleport elis(3);unsigned waits=0,calls=0;auto seed=native_random_seed;
        elis.entrance(archive,[&](unsigned n){waits+=n;++calls;if(argc>=3 && calls==140)dump_bmp(std::string(argv[2])+"-entrance.bmp",*fb);});
        require(waits==799 && calls==259,"Original entrance uses 119 slowing rotation waits and 140 wave ticks");
        auto girl=elis.girl();require(girl.left==256 && girl.top==96 && girl.cel==0 && girl.hittable && elis.frame()==0,"Entrance completes with still girl, active hitbox and reset clock");
        require(native_random_seed==seed,"Sphere/wave entrance consumes no RNG");
        expected->clear(1,0);expected->blit(1,0,0,bg.images[0],false);expected->blit(1,256,96,entities.slots[0].images[0],true);
        require(fb->pages[1]==expected->pages[1],"Entrance commits girl onto clean background page");
        elis.begin(200,100);for(unsigned i=0;i<81;++i)elis.advance();require(elis.girl().left==200 && elis.girl().top==100,"Entrance hands off to live wave teleport");
        for(unsigned cycle=0;cycle<2;++cycle)for(bool to_bat:{true,false}){
            auto before_calls=calls;auto before_waits=waits;auto rng=native_random_seed;shots.sound_commands.clear();elis.begin_transform(to_bat);
            for(unsigned tick=1;tick<=101;++tick){
                require(elis.advance_transform()==(tick==101),"Both original transformations finish at tick 101");
                if(tick>=20 && tick<=100 && tick%4==0){
                    unsigned draws=tick<100 ? 10:0;
                    for(unsigned i=0;i<5;++i)if(tick==20 || tick%16==(i*4)%16)draws+=2;
                    for(unsigned i=0;i<draws;++i)rng=rng*0x015a4e35u+1;
                }
                require(native_random_seed==rng,"Rift cel/color and staggered positions retain original RNG schedule");
                if(tick==22)require(fb->display_scroll_y==396,"Transformation shakes via original wrapped scroll offset");
                if(argc>=3 && cycle==0 && to_bat && tick==60)dump_bmp(std::string(argv[2])+"-transform.bmp",*fb);
            }
            require(calls-before_calls==42 && waits-before_waits==42 && fb->display_scroll_y==0 && elis.frame()==0,"Shake waits and scroll/clock reset follow original boundaries");
            require(shots.sound_commands.size()==21,"Each transformation emits twenty-one original rift sounds");for(auto se:shots.sound_commands)require(se==7,"Original rift sound ID");
            if(to_bat){auto b=elis.bat();require(b.left==240 && b.top==132 && b.cel==0,"Bat centers on the previous girl");expected->clear(1,0);expected->blit(1,0,0,bg.images[0],false);require(fb->pages[1]==expected->pages[1],"Original first bat frame restores girl but does not draw bat yet");}
            else{auto g=elis.girl();require(g.left==200 && g.top==100,"Girl reappears centered on bat");expected->clear(1,0);expected->blit(1,0,0,bg.images[0],false);expected->blit(1,200,100,entities.slots[1].images[1],true);require(fb->pages[1]==expected->pages[1],"Reverse transform commits attack pose into background page");}
        }
        elis.begin_transform(true);for(unsigned i=0;i<101;++i)elis.advance_transform();
        auto start_bat=elis.bat();auto flight_seed=native_random_seed;elis.begin_flight();unsigned flight_ticks=0;bool moved=false;
        while(true){
            ++flight_ticks;bool done=elis.advance_flight();auto b=elis.bat();
            require(b.cel==int((flight_ticks/3)%3),"Original bat cycles three cels at three-frame intervals");
            moved|=b.left!=start_bat.left || b.top!=start_bat.top;
            if(flight_ticks<=2)require(native_random_seed==flight_seed && !moved,"Bat waits two ticks before choosing its random target");
            if(done)break;if(flight_ticks>1000)throw std::runtime_error("Elis bat flight stalled");
        }
        require(moved && flight_ticks>3 && elis.frame()==0,"Original flystep reaches transform boundary and resets clock");
        auto final_bat=elis.bat();elis.begin_transform(false);for(unsigned i=0;i<101;++i)elis.advance_transform();
        auto g=elis.girl();require(g.left==std::clamp(final_bat.left-40,0,512) && g.top==std::max(final_bat.top-32,64),"Reverse transform centers and clamps girl after real bat flight");
        std::cout<<"Elis bat flight ticks="<<flight_ticks<<" target-position="<<final_bat.left<<','<<final_bat.top<<'\n';
    }
    for(unsigned i=0;i<3;++i)require(entities.slots[i].images.empty(),"Controller releases all three sprite banks");
    std::cout<<"Elis original entities, background cache, wave teleport, hitbox timing, repeated targets and star RNG passed; trace "<<std::hex<<trace<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
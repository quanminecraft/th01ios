#include "sariel_body.hpp"
#include "boss_entity_backend.hpp"
#include "hp_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: sariel_body_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A1.GRP"));auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);BossEntityBackend banks{*fb,archive,{}};BossEntityBinding eb(banks);std::uint64_t trace=1469598103934665603ull;
    for(unsigned run=0;run<2;++run){
        fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);
        {
            SarielBody body;auto state=body.state();require(state.hp==18 && state.phase==0 && state.frame==0 && hud_hp_first_white==8 && hud_hp_first_redwhite==2,"Original Sariel HP/phase/HUD thresholds");require(boss_entity_0.cur_left==304 && boss_entity_0.cur_top==144 && !boss_entity_0.hitbox_orb_inactive,"Shield starts at original active orb hitbox");orb_prev_left=320;orb_prev_top=164;require(boss_entity_0.hittest_orb(),"Original shield accepts orb at its center");
            require(boss_anims[0].bos_slot==1 && boss_anims[1].bos_slot==0 && boss_anims[0].left==280 && boss_anims[0].top==192 && boss_anims[1].left==296 && boss_anims[1].top==48,"Original animation instance indices differ from wand/dress bank indices");
            for(unsigned frame=0;frame<=120;++frame){auto before=fb->pages;body.render(frame);auto s=body.state();require(s.shield_cel==int((frame/10)%4) && s.dress_cel==int((frame/15)%4),"Original four-cel shield/dress cadence");if(frame%10!=0 && frame%15!=0)require(before==fb->pages,"Intermediate body frames do not redraw");require(fb->pages[0]==fb->pages[1] && current_render_page()==0,"Both original render helpers commit identical pages and return to zero");for(auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}}
            body.capture_wand_background();auto lowered=fb->pages;
            for(unsigned cycle=0;cycle<2;++cycle){for(unsigned tick=1;tick<=16;++tick){auto before=fb->pages;require(body.raise_wand()==(tick==16),"Wand raise completes at sixteen calls");if(tick==1)require(before==fb->pages,"First raise call does not draw");require(body.state().wand_cel==((tick==16 || (cycle>0 && tick==1)) ? 1:0) && fb->pages[0]==fb->pages[1],"Original wand cels at calls two/sixteen draw on both pages");}auto before=fb->pages;require(!body.raise_wand(true) && before==fb->pages,"Explicit wand restart resets timing without rendering");}
            body.lower_wand();require(fb->pages==lowered && current_render_page()==0,"Lowering restores original 128x96 background on both pages");
            fb->put(0,295,48,3);fb->put(1,424,48,4);body.lower_wand();require(fb->pixel(0,295,48)==3 && fb->pixel(1,424,48)==4,"Lowering does not overwrite adjacent pixels");
            if(argc>2)dump_bmp(argv[2],*fb);
        }
        require(banks.slots[0].images.empty() && banks.animation_slots[0].images.empty() && banks.animation_slots[1].images.empty(),"Sariel body releases all sprite banks");
    }
    std::cout<<"Original Sariel setup, shield/dress cadence, wand raise/restart, orb hitbox and reload passed; trace "<<std::hex<<trace<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
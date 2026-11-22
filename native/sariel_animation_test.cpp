#include "boss_entity_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: sariel_animation_test ASSET_DIR [BMP]");auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);
    boss_entity_0={};boss_entity_0.load("boss6_1.bos",0);boss_entity_0.pos_set(304,144);boss_entity_0.set_image(0);
    const char* names[]={"boss6_2.bos","boss6_3.bos"};int lefts[]={296,280},tops[]={48,192};std::uint64_t trace=1469598103934665603ull;
    for(unsigned bank=0;bank<2;++bank){auto& a=boss_anims[bank];a={};a.load(names[bank],bank);auto& loaded=entities.animation_slots[bank];require(a.vram_w*8==loaded.images[0].width && a.h==loaded.images[0].height && a.bos_image_count==loaded.images.size(),"Sariel animation metadata matches its real BOS bank");
        for(unsigned cel=0;cel<loaded.images.size();++cel)for(unsigned page=0;page<2;++page){
            fb->clear(0,3);fb->clear(1,5);a.left=lefts[bank]+1;a.top=tops[bank];a.bos_image=cel;page_access(page);a.put_8();auto& sprite=loaded.images[cel];
            for(int y=0;y<sprite.height;++y)for(int x=0;x<sprite.width;++x){auto i=y*sprite.width+x;auto expected=(sprite.opaque[i] ? 0:page==0 ? 3:5)|sprite.color[i];require(fb->pixel(page,lefts[bank]+x,tops[bank]+y)==expected,"Animation preserves raw planes, pre-inverted alpha and byte alignment on selected page");trace^=expected;trace*=1099511628211ull;}
            require(fb->pixel(1-page,lefts[bank],tops[bank])==(page==0 ? 5:3),"Animation leaves other page untouched");
        }
        auto before=fb->pages;a.bos_image=a.bos_image_count;a.put_8();require(before==fb->pages,"Unavailable animation cel is ignored");a.bos_image=0;a.top=399;a.left=lefts[bank];page_access(0);a.put_8();require(current_render_page()==0,"Animation bottom clipping preserves selected page");
    }
    auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS6_A1.GRP"));fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);boss_entity_0.put_8(304,144,0);for(unsigned bank=0;bank<2;++bank){auto& a=boss_anims[bank];a.left=lefts[bank];a.top=tops[bank];a.bos_image=0;a.put_8();}
    if(argc>2)dump_bmp(argv[2],*fb);bos_anim_free(0);require(entities.animation_slots[0].images.empty() && !entities.animation_slots[1].images.empty() && !entities.slots[0].images.empty(),"Animation banks release independently of dress and shield");bos_anim_free(1);bos_entity_free(0);
    std::cout<<"Sariel original shield/wand/dress BOS banks, CBossAnim alpha rendering, page selection and cleanup passed; trace "<<std::hex<<trace<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
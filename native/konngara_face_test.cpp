#include "konngara_face.hpp"
#include "boss_entity_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
// Independent BOS mask/color composition, checked against both rendered pages.
static void put(Framebuffer& fb,const Sprite& sprite,int left,int top) {
    for(unsigned page=0;page<2;++page)for(int y=0;y<sprite.height;++y)for(int x=0;x<sprite.width;++x) {auto i=y*sprite.width+x;auto old=fb.pixel(page,left+x,top+y);fb.put(page,left+x,top+y,(sprite.opaque[i] ? 0 : old)|sprite.color[i]);}
}
int main(int argc,char** argv) {
    try {
        if(argc<2)throw std::invalid_argument("Usage: konngara_face_test ASSET_DIR [BMP]");
        auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto expected=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shots{*fb,empty,{}};ShotBackendBinding shot_binding(shots);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding entity_binding(entities);
        auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS8_A1.GRP"));auto head=read_sprites(asset(archive,"BOSS8_1.BOS")),eyes=read_sprites(asset(archive,"BOSS8_E1.BOS")),aim=read_sprites(asset(archive,"BOSS8_E2.BOS"));
        require(head.images.size()==3 && eyes.images.size()==6 && aim.images.size()==3,"Original head/closed/glare/aim sprite banks");
        std::uint64_t trace=1469598103934665603ull;
        for(unsigned dir=0;dir<3;++dir)for(unsigned expr=0;expr<4;++expr) {
            for(unsigned p=0;p<2;++p){fb->blit(p,0,0,bg.images[0],false);expected->blit(p,0,0,bg.images[0],false);}fb->palette=bg.palette;page_access(0);
            {
                KonngaraFace face;face.direction(KonngaraDirection::right);put(*expected,head.images[0],280,80);
                face.direction(static_cast<KonngaraDirection>(dir));if(dir!=0)put(*expected,head.images[dir],280,80);
                face.expression(static_cast<KonngaraExpression>(expr));if(expr==3)put(*expected,aim.images[dir],280,128);else if(expr)put(*expected,eyes.images[(expr-1)*3+dir],280,128);
                require(fb->pages==expected->pages && current_render_page()==0,"All direction/expression combinations match original BOS pixels on both pages");
                auto stable=fb->pages;face.expression(static_cast<KonngaraExpression>(expr));face.direction(static_cast<KonngaraDirection>(dir));require(fb->pages==stable,"Repeated face state does not change image");
                face.allow_direction_change(false);face.tracking(dir==0 ? 0 : 600);require(fb->pages==stable && face.direction()==static_cast<KonngaraDirection>(dir),"Locked direction ignores player tracking");
                face.expression(KonngaraExpression::neutral);if(expr)put(*expected,head.images[dir],280,80);require(fb->pages==expected->pages,"Neutral expression restores head while direction locked");
                face.allow_direction_change(true);
                for(int left:{197,198,396,397}) {auto direction=left<198 ? 1u : left>396 ? 0u : 2u;face.tracking(left);require(face.direction()==static_cast<KonngaraDirection>(direction),"Original strict tracking thresholds");}
                for(auto expression:{KonngaraExpression::glare,KonngaraExpression::aim}) {
                    face.expression(expression);face.direction(KonngaraDirection::left);face.direction(KonngaraDirection::right);
                    auto direct=fb->pages;face.expression(KonngaraExpression::neutral);face.expression(expression);require(fb->pages==direct,"Changing direction reapplies glare/aim overlays");
                }
                if(argc>=3 && dir==0 && expr==2)dump_bmp(argv[2],*fb);
                for(auto& plane:fb->pages[0])for(auto b:plane){trace^=b;trace*=1099511628211ull;}
            }
            require(entities.slots[0].images.empty() && entities.slots[1].images.empty() && entities.slots[2].images.empty(),"Face destructor releases all three BOS banks");
        }
        std::cout<<"Konngara real-BOS head/face matrix, direction lock, thresholds and cleanup passed; trace "<<std::hex<<trace<<'\n';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

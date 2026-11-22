#include "konngara_slash.hpp"
#include "pellet_backend.hpp"
#include "assets.hpp"
#include "grz.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main(int argc,char** argv) {
    try {
        if(argc<2) throw std::invalid_argument("Usage: konngara_slash_test ASSET_DIR [BMP_PREFIX]");
        auto fb=std::make_unique<Framebuffer>();auto archive=load_archive(argv[1]);auto sprites=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,sprites,{}};ShotBackendBinding binding(shots);
        auto background=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS8_A1.GRP"));auto data=read_file(std::filesystem::path(argv[1])/"BOSS8.GRZ");std::array<SpriteSet,7> effects;for(unsigned i=0;i<7;++i) effects[i]=read_grz_image(data,i);
        std::uint64_t trace=1469598103934665603ull;auto mix=[&](unsigned n) {trace^=n;trace*=1099511628211ull;};
        for(unsigned difficulty=0;difficulty<4;++difficulty) for(auto attack:{KonngaraSlashAttack::rain,KonngaraSlashAttack::triangular,KonngaraSlashAttack::aimed}) {
            rank=difficulty;native_pellet_speed=0;native_random_seed=1;player_left=300;player_sliding=false;player_invincible=true;player_is_hit=false;
            Pellets.native_reset_all();shots.sound_commands.clear();fb->palette=background.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,background.images[0],false);page_access(0);
            unsigned ticks=0,emissions=0,centers=0;std::vector<KonngaraExpression> expressions;std::vector<unsigned> image_frames;std::uint32_t seed=1;
            KonngaraSlashPattern pattern(attack,{[&](auto e){expressions.push_back(e);},[&](){++centers;},[&](unsigned image){require(image==image_frames.size(),"Original slash image order");image_frames.push_back(ticks);page_access(1);fb->blit(1,0,0,effects.at(image).images[0],true);page_access(0);fb->blit(0,0,0,effects.at(image).images[0],true);}});
            while(true) {
                ++ticks;bool done=pattern.advance();
                if(ticks==10) require(centers==1 && expressions==std::vector<KonngaraExpression>{attack==KonngaraSlashAttack::rain ? KonngaraExpression::closed : KonngaraExpression::aim},"Slash centers face and changes expression at frame ten");
                require(pattern.face_direction_change_allowed()==(ticks<10 || ticks==201),"Slash locks face through animation then unlocks at completion");
                unsigned count=Pellets.native_alive_count();if(ticks<100 || ticks>=170) require(!count,"Slash only emits during original active windows");
                if(count) {
                    require(count==(attack==KonngaraSlashAttack::aimed ? 1 : 2),"Original slash volley size");
                    auto& first=Pellets.native_at(0);if(emissions==0)require(first.cur_left.v==410*16 && first.cur_top.v==70*16,"First projectile starts at sword center");
                    if(ticks==140)require(first.cur_left.v==432*16 && first.cur_top.v==232*16,"Frame 140 snaps emitter to corner four");
                    if(ticks==150)require(first.cur_left.v==198*16 && first.cur_top.v==198*16,"Frame 150 snaps emitter to corner five");
                    for(unsigned i=0;i<count;++i) {
                        auto& p=Pellets.native_at(i);pixel_t vx,vy;
                        if(attack==KonngaraSlashAttack::rain) {
                            require(p.motion_type==PM_GRAVITY && p.speed.v==1,"Slash rain uses original acceleration");seed=seed*0x015a4e35u+1;
                            vector2(vx,vy,subpixel_t(std::max(16u,difficulty*6)),static_cast<unsigned char>(((seed>>16)&0x7fff)%127));
                        } else if(attack==KonngaraSlashAttack::triangular) {
                            const int speeds[]={32,54,76,90};vector2(vx,vy,subpixel_t(speeds[difficulty]),static_cast<unsigned char>(i ? 0x60 : 0x20));
                        } else {
                            const int speeds[]={64,86,100,114};vector2_between(p.cur_left.to_pixel(),p.cur_top.to_pixel(),pixel_t(player_center_x()-PELLET_W),player_center_y(),vx,vy,subpixel_t(speeds[difficulty]));
                        }
                        require(p.velocity.x.v==vx && p.velocity.y.v==vy,"Original slash angle/aiming and rank-adjusted velocity");
                        mix(ticks);mix(std::uint16_t(p.cur_left.v));mix(std::uint16_t(p.cur_top.v));mix(std::uint16_t(p.velocity.x.v));mix(std::uint16_t(p.velocity.y.v));
                    }
                    emissions+=count;Pellets.native_reset_all();
                }
                if(argc>=3 && difficulty==1 && attack==KonngaraSlashAttack::triangular && ticks==140) dump_bmp(std::string(argv[2])+"-140.bmp",*fb);
                if(done) break;if(ticks>201)throw std::runtime_error("Slash pattern stalled");
            }
            const unsigned rain_totals[]={50,66,100,140};require(ticks==201 && emissions==(attack==KonngaraSlashAttack::rain ? rain_totals[difficulty] : attack==KonngaraSlashAttack::triangular ? 66u : 33u),"Original slash duration and emission total");
            require(image_frames==std::vector<unsigned>({50,60,100,120,140,160,170}) && shots.sound_commands==std::vector<unsigned>{8},"Seven original slash image keyframes and single SE-8");
            require(native_random_seed==seed && !player_is_hit && fb->pages[0]==fb->pages[1] && current_render_page()==0,"RNG, page synchronization and no added sword collision");
            auto expected=std::make_unique<Framebuffer>();expected->blit(0,0,0,background.images[0],false);for(auto& effect:effects) expected->blit(0,0,0,effect.images[0],true);require(expected->pages[0]==fb->pages[0],"Slash effects compose original GRZ images on background");
            for(auto& plane:fb->pages[0])for(auto b:plane)mix(b);
            require(pattern.advance() && image_frames.size()==7,"Completed slash does not render again");
        }
        std::cout<<"Konngara slash rain/triangular/aimed real-GRZ animation and four-rank emissions passed; trace "<<std::hex<<trace<<'\n';
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

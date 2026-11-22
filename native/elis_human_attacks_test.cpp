#include "laser_backend.hpp"
#include "elis_teleport.hpp"
#include "boss_entity_backend.hpp"
#include "pellet_backend.hpp"
#include "missile_backend.hpp"
#include "grp.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try{
    if(argc<2)throw std::invalid_argument("Usage: elis_human_attacks_test ASSET_DIR");
    auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto raw=std::make_unique<Framebuffer>();auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS5.GRP"));
    auto stg=read_sprites(asset(archive,"STG.PTN"));ShotBackend shots{*fb,stg,{}};ShotBackendBinding sb(shots);auto miko=read_sprites(asset(archive,"MIKO.PTN"));PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding pb(player);BossEntityBackend entities{*fb,archive,{}};BossEntityBinding eb(entities);
    std::uint64_t trace=1469598103934665603ull;
    for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned pattern=1;pattern<=2;++pattern){
        fb->palette=bg.palette;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);raw->blit(1,0,0,bg.images[0],false);page_access(0);
        rank=difficulty;native_pellet_speed=0;ElisTeleport elis(3);shots.sound_commands.clear();elis.begin_human_pattern(pattern);unsigned total=0;
        unsigned duration=pattern==1 ? 120:61;
        for(unsigned tick=1;tick<=duration;++tick){
            Pellets.native_reset_all();bool done=elis.advance_human_pattern();require(done==(tick==duration),"Original symmetric/ring pattern completion boundaries");
            int volley=-1;unsigned count=0;
            if(pattern==1){if(tick==50 || tick==60 || tick==70)count=8;if(tick==80)count=20;}
            else{const unsigned times[]={10,16,24,32,40};for(unsigned i=0;i<5;++i)if(tick==times[i])volley=i;if(volley>=0)count=14+difficulty*2;}
            require(Pellets.native_alive_count()==int(count),"Original human volley counts and firing times");
            for(unsigned i=0;i<count;++i){auto& p=Pellets.native_at(i);pixel_t vx,vy;int angle,speed;
                if(pattern==1){const int bases[]={2,3,3,4};int j=i/2;int right=tick==50 ? 64:tick==60 ? 48:tick==70 ? 24:j*6;angle=i%2 ? right:128-right;speed=(bases[difficulty]+(tick==80 ? 2:j))*16+difficulty*6;require(p.cur_left.v==(i%2 ? 320:312)*16 && p.cur_top.v==136*16,"Symmetric stacks retain distinct left/right eye offsets");}
                else{const int speeds[]={48,54,64,72,80};angle=(256/int(count))*i+volley*2;speed=speeds[volley]+difficulty*6;require(p.cur_left.v==316*16 && p.cur_top.v==140*16,"Curved rings retain original central muzzle");}
                vector2(vx,vy,speed,static_cast<unsigned char>(angle));require(p.velocity.x.v==vx && p.velocity.y.v==vy && p.from_group==PG_NONE,"Original mirrored/ring angles and stack velocities");trace^=std::uint16_t(vx);trace*=1099511628211ull;trace^=std::uint16_t(vy);trace*=1099511628211ull;
            }
            require(native_random_seed==3,"Deterministic human stacks/rings consume no RNG");total+=count;
        }
        require(total==(pattern==1 ? 44u:70+difficulty*10),"Whole-pattern projectile totals");
        require(shots.sound_commands.size()==(pattern==1 ? 3u:0u),"Original human firing sound schedule");for(auto sound:shots.sound_commands)require(sound==7,"Original firing SE7");
        if(pattern==2)require(current_render_page()==1 && fb->pages[1]==raw->pages[1],"Ring ending preserves original page-one unblit quirk");
        auto before=fb->pages;require(elis.advance_human_pattern() && before==fb->pages && elis.frame()==0,"Completed pattern is inert");
    }
    MissileBackend missiles{archive};MissileBinding mb(missiles);Missiles.load(PTN_SLOT_BOSS_2);
    for(unsigned difficulty=0;difficulty<4;++difficulty){
        rank=difficulty;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);
        ElisTeleport elis(3);elis.entrance(archive,[](unsigned){});elis.begin_human_pattern(3);shots.sound_commands.clear();unsigned emitted=0;std::uint32_t rng=3;int rx[5]{},ry[5]{};auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};
        for(unsigned tick=1;tick<=171;++tick){
            Missiles.reset();bool fires=tick>60 && tick%3==0;int x=0,y=0;pixel_t vx=0,vy=0;
            if(fires){unsigned slot=random()%5;const unsigned ranges[]={15,21,25,29};unsigned angle=random()%ranges[difficulty]-(ranges[difficulty]-1)/2+64;x=rx[slot];y=ry[slot];vector2(vx,vy,7,static_cast<unsigned char>(angle));}
            if(tick>=60 && tick<=160 && tick%4==0)for(unsigned i=0;i<5;++i){if(tick==60 || tick%16==(i*4)%16){rx[i]=224+random()%192;ry[i]=64+random()%128;}if(tick<160)(void)random();}
            require(elis.advance_human_pattern()==(tick==171),"Rift missile attack completes at 171 after its last launch");
            require(native_random_seed==rng,"Rift positions/cels and missile selection/angle consume original RNG in order");
            unsigned count=0;for(unsigned i=0;i<MISSILE_COUNT;++i)if(Missiles.native_at(i).flag!=0){++count;auto m=Missiles.native_at(i);if(!(m.left==x*16 && m.top==y*16 && m.velocity_x==vx*16 && m.velocity_y==vy*16))throw std::runtime_error("Rift tick "+std::to_string(tick)+" actual "+std::to_string(m.left)+","+std::to_string(m.top)+","+std::to_string(m.velocity_x)+","+std::to_string(m.velocity_y)+" expected "+std::to_string(x*16)+","+std::to_string(y*16)+","+std::to_string(vx*16)+","+std::to_string(vy*16));trace^=std::uint16_t(m.left);trace*=1099511628211ull;trace^=std::uint16_t(m.velocity_x);trace*=1099511628211ull;}
            require(count==unsigned(fires),"Rift missiles fire every third tick after sixty including final tick");emitted+=count;
        }        require(emitted==37 && shots.sound_commands.size()==26 && elis.frame()==0,"Rift attack emits 37 missiles and 26 rift sound commands");
    }
    {
        for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);Missiles.reset();
        ElisTeleport elis(3);elis.entrance(archive,[](unsigned){});elis.begin_human_pattern(3);rank=RANK_NORMAL;player_invincible=true;player_is_hit=false;player_left=300;
        unsigned maximum_live=0;bool saw_explosion=false;
        for(unsigned tick=1;tick<=171;++tick){missiles.frame=tick;Missiles.unput_update_render();elis.advance_human_pattern();unsigned alive=0;for(unsigned i=0;i<MISSILE_COUNT;++i){auto m=Missiles.native_at(i);alive+=m.flag!=MF_FREE;saw_explosion|=m.flag==MF_HIT;}maximum_live=std::max(maximum_live,alive);}
        for(unsigned tick=172;tick<400;++tick){missiles.frame=tick;Missiles.unput_update_render();for(unsigned i=0;i<MISSILE_COUNT;++i)saw_explosion|=Missiles.native_at(i).flag==MF_HIT;}
        for(unsigned i=0;i<MISSILE_COUNT;++i)require(Missiles.native_at(i).flag==MF_FREE,"Sustained missiles drain all slots after pattern");
        require(maximum_live>5 && saw_explosion && !player_is_hit,"Original missile-before-pattern order supports simultaneous live shots, floor explosions and invincible player");
    }    for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned seed:{1u,4u}){
        rank=difficulty;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);for(auto& l:shootout_lasers)l={};
        ElisTeleport elis(seed);elis.begin_human_pattern(4);shots.sound_commands.clear();unsigned emitted=0;std::uint32_t rng=seed;
        for(unsigned tick=1;tick<=170;++tick){
            require(elis.advance_human_pattern()==(tick==170),"Eleven-laser sweep ends on tick 170");if(tick==50)rng=rng*0x015a4e35u+1;require(native_random_seed==rng,"Laser sweep consumes only its direction RNG draw");
            if(tick>=70 && tick%10==0){
                auto& l=shootout_lasers[(tick/10)%10];unsigned direction=(rng>>16)&1;int target=direction ? 640-int(emitted)*64:int(emitted)*64;
                unsigned char angle=iatan2(276,target-316);const int speeds[]={6,6,7,8};
                require(l.alive && l.age==0 && l.origin_left.v==316*256 && l.origin_y.v==124*256,"Laser spawns at original left-eye muzzle and cyclic slot");
                require(l.step_x.v==Cos8(angle) && l.step_y.v==Sin8(angle) && l.ray_extend_speed==speeds[difficulty] && l.ray_moveout_speed==speeds[difficulty],"Original direction, target progression and rank speed truncation");
                require(l.width_cel==3 && l.col==7 && l.moveout_at_age==25 && l.damaging,"Original laser width/color/damaging lifetime");
                trace^=std::uint16_t(l.step_x.v);trace*=1099511628211ull;trace^=std::uint16_t(l.step_y.v);trace*=1099511628211ull;++emitted;
            }
            update_shootout_lasers();
        }
        require(emitted==11 && shots.sound_commands.size()==11,"Sweep emits eleven laser spawns and firing sounds");for(auto se:shots.sound_commands)require(se==6,"Original laser SE6");
        for(unsigned tick=171;tick<400;++tick)update_shootout_lasers();for(auto& l:shootout_lasers)require(!l.alive,"All sweep lasers leave the screen after completion");
    }    for(unsigned difficulty=0;difficulty<4;++difficulty)for(bool lowered:{false,true}){
        rank=difficulty;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);
        ElisTeleport elis(3);if(lowered){elis.begin(256,176);for(unsigned i=0;i<81;++i)elis.advance();}
        native_random_seed=3;std::uint32_t rng=3;player_left=300;elis.begin_human_pattern(5);shots.sound_commands.clear();
        for(unsigned tick=1;tick<=162;++tick){Pellets.native_reset_all();require(elis.advance_human_pattern()==(tick==162),"Circle summon/flash finishes and fires at tick 162");if(tick<162)require(Pellets.native_alive_count()==0 && native_random_seed==3,"Summon/flash neither emits early nor consumes RNG");}
        const pellet_group_t groups[]={PG_1_AIMED,PG_1_RANDOM_NARROW_AIMED,PG_3_SPREAD_WIDE_AIMED,PG_5_SPREAD_NARROW_AIMED};unsigned each=difficulty<2 ? 1:difficulty==2 ? 3:5;unsigned index=0;
        for(unsigned origin=0;origin<32;++origin){int x=polar_x(pixel_t(316),pixel_t(128),origin*8),y=polar_y(pixel_t(lowered ? 224:144),pixel_t(128),origin*8);if(y<56)continue;
            for(unsigned n=0;n<each && index<100;++n){auto& p=Pellets.native_at(index++);require(p.cur_left.v==x*16 && p.cur_top.v==y*16 && p.from_group==groups[difficulty],"Circle volleys retain left-eye X offset, rank groups and clipped origins");
                int offset=0;if(difficulty==1){rng=rng*0x015a4e35u+1;offset=int((rng>>16)&15)-7;}else if(difficulty==2){const int offsets[]={0,9,-9};offset=offsets[n];}else if(difficulty==3){const int offsets[]={0,4,-4,8,-8};offset=offsets[n];}
                pixel_t vx,vy;int speed=std::max(16,4+int(difficulty)*6);unsigned char angle=iatan2(player_center_y()-y,player_center_x()-(difficulty==0 ? 8:4)-x)+offset;vector2(vx,vy,speed,angle);
                require(p.velocity.x.v==vx && p.velocity.y.v==vy,"Circle aimed spread velocity and rank/minimum launch speed");trace^=std::uint16_t(p.cur_left.v);trace*=1099511628211ull;trace^=std::uint16_t(vx);trace*=1099511628211ull;
            }
        }
        const unsigned base_counts[]={23,23,69,100},lower_counts[]={32,32,96,100};unsigned expected=lowered ? lower_counts[difficulty]:base_counts[difficulty];
        require(index==expected && Pellets.native_alive_count()==int(expected),"Original top-edge clipping and hundred-pellet capacity cap");require(native_random_seed==rng,"Only Normal circle volleys consume random aimed-spread angles");require(shots.sound_commands.size()==1 && shots.sound_commands[0]==8,"Original circle summon SE8 only");
    }    for(unsigned difficulty=0;difficulty<4;++difficulty)for(int girl_left:{128,384}){
        rank=difficulty;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);
        ElisTeleport elis(3);elis.entrance(archive,[](unsigned){});elis.begin(girl_left,96);for(unsigned i=0;i<81;++i)elis.advance();
        native_random_seed=3;std::uint32_t rng=3;auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};int x[10]{},y[10]{};unsigned total=0;
        elis.begin_human_pattern(6);shots.sound_commands.clear();
        for(unsigned tick=1;tick<=251;++tick){
            Pellets.native_reset_all();if(tick<=100 && tick%10==0){int i=tick/10;int column=(girl_left<256 ? 608-girl_left:-girl_left)/10;x[i-1]=i*column+girl_left+48;y[i-1]=i*((int(random()%200)+104)/10)+128;}
            bool fires=tick>=110 && tick<=200 && tick%10==0;const unsigned counts[]={5,7,9,13};unsigned count=fires ? counts[difficulty]:0;pixel_t vx[13]{},vy[13]{};
            for(unsigned i=0;i<count;++i){unsigned angle=random()&127;int speed=(random()+24)&63;speed=std::max(16,speed+int(difficulty)*6);vector2(vx[i],vy[i],speed,angle);}
            require(elis.advance_human_pattern()==(tick==251),"Ten-sphere cluster cycle completes at 251 after removal");require(native_random_seed==rng,"Sphere placement and per-pellet angle/speed RNG follow original schedule");require(Pellets.native_alive_count()==int(count),"Original ten sequential cluster bursts and rank counts");
            for(unsigned i=0;i<count;++i){auto& p=Pellets.native_at(i);unsigned sphere=(tick-100)/10-1;require(p.cur_left.v==x[sphere]*16 && p.cur_top.v==y[sphere]*16 && p.velocity.x.v==vx[i] && p.velocity.y.v==vy[i],"Sphere origin and random downward-cluster velocity match original");trace^=std::uint16_t(p.cur_top.v);trace*=1099511628211ull;trace^=std::uint16_t(vx[i]);trace*=1099511628211ull;}
            total+=count;
        }
        const unsigned totals[]={50,70,90,130};require(total==totals[difficulty] && shots.sound_commands.size()==10,"Ten sphere bursts emit original rank totals and sounds");for(auto se:shots.sound_commands)require(se==7,"Original sphere firing SE7");
    }    for(unsigned difficulty=0;difficulty<4;++difficulty){
        rank=difficulty;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);ElisTeleport elis(3);elis.entrance(archive,[](unsigned){});elis.begin_human_pattern(7);shots.sound_commands.clear();
        std::uint32_t rng=3;auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};int x[5]{},y[5]{};unsigned total=0;
        for(unsigned tick=1;tick<=221;++tick){
            Pellets.native_reset_all();const unsigned intervals[]={6,3,2,2};bool fires=(tick>60 && tick<120 && tick%intervals[difficulty]==0)||(tick>=120 && tick<200);int slot=0;pixel_t vx=0,vy=0;
            if(fires){slot=random()%5;unsigned char angle=random()%21+54;if(tick>=120){unsigned char offset=(random()&1) ? int(tick-120):-int(tick-120);angle=static_cast<unsigned char>(angle*(offset/2));}vector2(vx,vy,96+difficulty*6,angle);}
            int px=x[slot],py=y[slot];
            if(tick>=60 && tick<=200 && tick%4==0)for(unsigned i=0;i<5;++i){if(tick==60 || tick%16==(i*4)%16){x[i]=224+random()%192;y[i]=64+random()%128;}if(tick<200)(void)random();}
            require(elis.advance_human_pattern()==(tick==221),"Rift pellet attack ends at 221");require(native_random_seed==rng,"Rift emission and redraw retain original RNG order");require(Pellets.native_alive_count()==int(fires),"First phase uses rank interval; second phase fires every frame");
            if(fires){auto& p=Pellets.native_at(0);require(p.cur_left.v==px*16 && p.cur_top.v==py*16 && p.velocity.x.v==vx && p.velocity.y.v==vy,"Rift pellets preserve pre-redraw muzzle and unsigned-angle multiplication quirk");trace^=std::uint16_t(vx);trace*=1099511628211ull;trace^=std::uint16_t(vy);trace*=1099511628211ull;++total;}
        }
        const unsigned totals[]={89,99,109,109};require(total==totals[difficulty] && shots.sound_commands.size()==36,"Original rift projectile totals and redraw sounds");
    }    for(unsigned difficulty=0;difficulty<4;++difficulty){
        rank=difficulty;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);ElisTeleport elis(3);elis.entrance(archive,[](unsigned){});elis.begin_transform(true);for(unsigned i=0;i<101;++i)elis.advance_transform();elis.begin_transform(false);for(unsigned i=0;i<101;++i)elis.advance_transform();
        player_left=300;player_invincible=false;player_is_hit=false;unsigned delays=0;elis.set_delay([&](unsigned ms){delays+=ms;});native_random_seed=3;std::uint32_t rng=3;auto random=[&](){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;};elis.begin_human_pattern(8);shots.sound_commands.clear();unsigned total=0;
        for(unsigned tick=1;tick<=382;++tick){
            Pellets.native_reset_all();unsigned count=0;int rain_x=0;unsigned rain_angle=64;bool rain=tick>=123 && tick<=321 && tick%(difficulty==0 ? 4:2)==0;
            if(rain){rain_x=random()%632;if(difficulty==3)rain_angle=(random()&63)+32;++count;}
            int circle_frame=int(tick)-121;bool aimed=(tick>=123 && tick<=321 && circle_frame>180 && circle_frame%4==0)||(tick>=322 && circle_frame%8==0);if(aimed)++count;
            require(elis.advance_human_pattern()==(tick==382),"Safety circle and tail complete at 382 without a hit");require(!player_is_hit && delays==0 && native_random_seed==rng,"Safe player survives and rain RNG is exact");require(Pellets.native_alive_count()==int(count),"Safety rain, five late aimed shots and seven tail shots follow original cadence");
            if(rain){auto& p=Pellets.native_at(0);pixel_t vx,vy;vector2(vx,vy,72+difficulty*6,rain_angle);require(p.cur_left.v==rain_x*16 && p.cur_top.v==64*16 && p.velocity.x.v==vx && p.velocity.y.v==vy,"Original top rain spawn and Lunatic angular spread");}
            if(aimed){auto& p=Pellets.native_at(count-1);int x=tick<=321 ? 316:356,y=tick<=321 ? 140:172;require(p.cur_left.v==x*16 && p.cur_top.v==y*16 && p.from_group==PG_1_AIMED,"Tail aimed shot uses stale bat-relative muzzle as original");}
            total+=count;
        }
        require(total==(difficulty==0 ? 62u:111u) && shots.sound_commands.size()==1 && shots.sound_commands[0]==8,"Original safety attack totals and summon sound");
    }
    for(int edge:{-128,104})for(bool invincible:{false,true}){
        for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);ElisTeleport elis;player_left=300;player_invincible=invincible;player_is_hit=false;unsigned delays=0;elis.set_delay([&](unsigned ms){delays+=ms;});elis.begin_human_pattern(8);for(unsigned t=1;t<=122;++t){Pellets.native_reset_all();elis.advance_human_pattern();}player_left=300+edge;elis.advance_human_pattern();require(bool(player_is_hit)==!invincible && delays==(invincible ? 0u:100u),"Inclusive safety edges trigger original 100ms hit delay, except invincible player");
    }
    for(unsigned difficulty=0;difficulty<4;++difficulty){
        rank=difficulty;player_left=300;player_invincible=true;player_is_hit=false;for(unsigned p=0;p<2;++p)fb->blit(p,0,0,bg.images[0],false);page_access(0);for(auto& l:shootout_lasers)l={};ElisTeleport elis(3);elis.begin_human_pattern(9);shots.sound_commands.clear();unsigned total=0,lasers=0;const unsigned intervals[]={40,30,20,15};
        for(unsigned tick=1;tick<=162;++tick){Pellets.native_reset_all();bool done=elis.advance_human_pattern();require(done==(tick==162),"Final combined attack finishes with circle volley at 162");unsigned expected=tick==162 ? 21:tick>=50 && tick%intervals[difficulty]==0 ? 10:0;require(Pellets.native_alive_count()==int(expected),"Original paired five-way wing volleys and clipped final circle");
            if(tick>=50 && tick<162 && tick%(intervals[difficulty]*2)==0){auto& l=shootout_lasers[(tick/10)%10];require(l.alive && l.age==0 && l.origin_left.v==316*256 && l.origin_y.v==124*256 && l.width_cel==7 && l.moveout_at_age==20 && l.ray_extend_speed==6,"Final laser uses original eight-pixel ray, eye Y and fixed truncated speed");++lasers;}
            require(native_random_seed==3,"Final aimed spread/laser attack consumes no RNG");total+=expected;update_shootout_lasers();
        }
        const unsigned totals[]={51,61,81,91},laser_counts[]={2,2,3,4};require(total==totals[difficulty] && lasers==laser_counts[difficulty],"Final attack original rank totals");
    }    std::cout<<"Elis original symmetric stacks/arc and five curved rings across four ranks passed; trace "<<std::hex<<trace<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
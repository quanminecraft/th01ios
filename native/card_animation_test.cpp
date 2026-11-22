#include "card_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
static SpriteSet synthetic() {
	SpriteSet sprites;
	for(unsigned i=0;i<34;++i) sprites.images.push_back({32,32,Bytes(1024,std::uint8_t(i%14+1)),Bytes(1024,1)});
	return sprites;
}
int main(int argc,char** argv) {
	try {
		static_assert(sizeof(subpixel_t)==2 && sizeof(Subpixel)==2 && sizeof(SPPoint)==4);
		SPPoint point;point.set_long(16,32);require(point.x.v==16 && point.y.v==32,"Packed subpixel positive coordinates");
		point.set_long(-16,32);require(point.x.v==-16 && point.y.v==-1,"Retain negative-X sign extension of the original packed expression");
		if(argc>3) throw std::runtime_error("Usage: card_animation_test [ASSET_DIR [SCREENSHOT.bmp]]");
		auto sprites=synthetic();Bytes font_data(0x46800);SpriteSet background;StageLayout first_stage;
		if(argc>=2) {
			auto archive=load_archive(argv[1]);sprites=read_sprites(asset(archive,"STG.PTN"));
			first_stage=read_scene(asset(archive,"STAGE0.DAT"))[0];
			font_data=read_file(std::filesystem::path(argv[1])/"FONT.ROM");
			background=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));
		} else {
			for(unsigned c='0';c<='9';++c) for(unsigned y=0;y<16;++y)
				font_data[0x1800+(0x29-0x21)*96*32+(c-0x20)*32+y]=y==0 || y==15 ? 0x7e : 0x42;
		}
		FontRom font(std::move(font_data));auto fb=std::make_unique<Framebuffer>();
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		CardRenderer render{*fb,sprites,font,{}};CardEvents events;CardBackendBinding binding(events);
		auto reset=[&](unsigned hp) {
			if(argc>=2) {fb->palette=background.palette;fb->blit(1,0,0,background.images[0],false);}
			else fb->clear(1,3);
			fb->pages[0]=fb->pages[1];events={};events.renderer=&render;
			StageLayout stage;stage.cards.push_back({64,96,0,std::uint8_t(hp)});load_cards(stage);capture_card_backgrounds(render);
			score=0;cardcombo_cur=0;cardcombo_max=0;card_flip_cycle=1;rank=RANK_NORMAL;
			stage_cleared=false;player_is_hit=false;test_damage=false;bomb_damaging=false;stage_num=1;
			orb_cur_left=64;orb_cur_top=96;page_access(0);
		};
		std::uint64_t trace=14695981039346656037ull;
		auto tick=[&]() {cards_update_and_render();for(auto& page:fb->pages) for(auto& plane:page) for(auto value:plane) {trace^=value;trace*=1099511628211ull;}};
		reset(0);cards_hittest(0);
		require(cards.flag[0]==CARD_FLIPPING && cards.flip_frame[0]==0,"Hit starts frame-zero flip");
		for(int i=0;i<29;++i) {tick();if(i==17 && argc==3) dump_bmp(argv[2],*fb);}
		require(cards.flag[0]==CARD_FLIPPING && cards.flip_frame[0]==29 && !stage_cleared,"Flip is not complete at frame 29");
		tick();require(cards.flag[0]==CARD_REMOVED && cards.flip_frame[0]==0 && stage_cleared && player_is_hit,"Frame 30 removes the final card and signals completion");
		require(cards_score[0]==0,"Score popup completes before card removal");
		reset(2);
		for(int flip=0;flip<3;++flip) {
			cards_hittest(0);for(int i=0;i<30;++i) tick();
			if(flip<2) require(cards.flag[0]==CARD_ALIVE && cards.hp[0]==1-flip && !stage_cleared,"Multi-HP card returns alive with one fewer extra flip");
		}
		require(cards.flag[0]==CARD_REMOVED && stage_cleared && score==400,"Three-flip card removal and score progression");
		reset(0);rank=RANK_LUNATIC;stage_num=9;cards_hittest(0);
		for(int i=0;i<24;++i) tick();require(events.pellets.empty(),"No revenge pellet before flip keyframe");
		tick();require(events.pellets.size()==1 && events.pellets[0].left==76 && events.pellets[0].top==108 && events.pellets[0].group==PG_1_AIMED && events.pellets[0].speed==56,"Frame 25 aimed revenge pellet request");
		reset(0);rank=RANK_LUNATIC;stage_num=10;cards_hittest(0);
		for(int i=0;i<25;++i) tick();require(events.pellets[0].group==PG_1_RANDOM_NARROW_AIMED,"Later-stage revenge group selection");
		require(trace==(argc>=2 ? 0x364319637b537f55ull : 0xfaa870a69cb00795ull),"Card animation framebuffer trace regression");
		if(argc>=2) {
			reset(0);load_cards(first_stage);capture_card_backgrounds(render);
			require(cards.count==26,"Real first-stage card count");
			for(int i=0;i<cards.count;++i) {
				page_access(1);stageobj_put_bg_and_obj_8(cards.left[i],cards.top[i],CARD_ANIM[cards.hp[i]][0],i);
				page_access(0);stageobj_put_bg_and_obj_8(cards.left[i],cards.top[i],CARD_ANIM[cards.hp[i]][0],i);
			}
			// Controlled mass hit tests the complete real layout's lifecycle;
			// it does not substitute for a real player-driven clearance test.
			test_damage=true;cards_hittest(0);
			for(int i=0;i<30;++i) cards_update_and_render();
			bool removed=true;for(auto flag:cards.flag) removed&=flag==CARD_REMOVED;
			require(removed && stage_cleared && player_is_hit && score==113100 && cardcombo_cur==26,"Real first-stage card lifecycle and completion signal");
		}
		std::cout << "Original card flip/removal, multi-HP, popup and Lunatic revenge timing passed; trace " << std::hex << trace << '\n';
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

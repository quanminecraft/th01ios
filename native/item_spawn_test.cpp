#include "item_backend.hpp"
#include "item_motion.hpp"
#include "item_runtime.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
static int hit_calls=0,collect_calls=0,drop_calls=0;
static bool collect_on_hit=false;
static void hit(int slot) {
	++hit_calls;if(collect_on_hit) items_point[slot].flag=IF_COLLECTED;
}
static void collect(int slot) {
	require(items_point[slot].flag==IF_COLLECTED,"Hit test precedes collected-state dispatch");++collect_calls;
}
static void drop() {require(items_point[0].flag==IF_FREE,"Slot is free before drop callback");++drop_calls;}
int main() {
	try {
		static_assert(sizeof(item_t)==10 && sizeof(item_flag_state_t)==1);
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		for(unsigned i=0;i<34;++i) sprites.images.push_back({32,32,Bytes(1024,2),Bytes(1024,1)});
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,sprites,sprites,sprites,{},{},{}};PlayerBackendBinding player_binding(player);
		CardEvents events;CardBackendBinding card_binding(events);StageItemBinding item_binding(events);
		FontRom font(Bytes(0x46800,255));CardRenderer renderer{*fb,sprites,font,{}};events.renderer=&renderer;
		StageLayout layout;layout.cards={{64,96,0,0},{128,160,0,0},{256,224,0,0}};load_cards(layout);
		reset_item_storage();
		items_point_add(1);
		require(items_point[0].left==128 && items_point[0].top==160 && items_point[0].flag==IF_SPLASH && items_point[0].velocity_y==2 && items_point[0].flag_state.splash_radius==4 && items_point[0].unknown_zero==0,"Original item creation state and source card coordinates");
		for(int i=1;i<10;++i) items_point_add(i%3);
		items_point_add(2);require(items_point[9].left==64,"Full point pool ignores extra drops");
		items_point[3].flag=IF_FREE;items_point_add(2);
		require(items_point[3].left==256 && items_point[3].flag==IF_SPLASH,"Point pool reuses first free slot");
		for(int i=0;i<5;++i) items_bomb_add(i%3);
		require(items_bomb[3].left==64,"Four-slot bomb pool capacity");
		reset_item_storage();score=0;cardcombo_cur=0;cardcombo_max=0;card_flip_cycle=0;
		rank=RANK_NORMAL;test_damage=false;bomb_damaging=false;orb_cur_left=64;orb_cur_top=96;
		cards_hittest(0);
		require(events.items.size()==1 && !events.items[0].bomb && items_point[0].flag==IF_SPLASH && items_point[0].left==64,"Card collision creates a real point item synchronously");
		load_cards(layout);card_flip_cycle=140;orb_cur_left=128;orb_cur_top=160;
		cards_hittest(0);
		require(events.items.size()==2 && events.items[1].bomb && items_bomb[0].flag==IF_SPLASH && items_bomb[0].left==128,"Original drop cycle creates a real bomb item synchronously");
		reset_item_storage();items_point_add(0);fb->clear(0,3);fb->clear(1,3);page_access(0);
		auto& item=items_point[0];
		auto tick=[&]() {item_unput_update_render(0,item.flag,hit,item.left,item.top,item.velocity_y,item.flag_state.splash_radius,PTN_ITEM_POINT,collect,drop);};
		for(int i=0;i<10;++i) tick();
		require(item.flag==IF_SPLASH && item.flag_state.splash_radius==44 && item.top==96,"Splash expands without moving for ten ticks");
		unsigned white=0;for(int y=60;y<160;++y) for(int x=20;x<140;++x) {
			white+=fb->pixel(0,x,y)==V_WHITE;
			require(fb->pixel(1,x,y)==3,"Splash preserves background page");
		}
		require(white>0,"Splash circle produces actual foreground pixels");
		tick();require(item.flag==IF_FALL && item.flag_state.splash_radius==48 && item.top==96,"Eleventh splash tick starts fall without drawing item");
		tick();require(item.top==98 && item.velocity_y==2 && fb->pixel(0,64,98)==2,"Falling item moves two pixels and renders");
		item.top=366;tick();require(item.flag==IF_BOUNCE && item.top==360 && item.velocity_y==-7,"Floor transition performs bounce movement in the same tick");
		unsigned bounce_frames=0;while(item.flag!=IF_FREE && bounce_frames<100) {tick();++bounce_frames;}
		require(item.flag==IF_FREE && drop_calls==1 && item.top>=400,"Bounce eventually releases slot and calls drop once");
		items_point_add(0);item.flag=IF_FALL;auto before=item.top;collect_on_hit=true;tick();
		require(collect_calls==1 && item.top==before,"Collection callback replaces motion immediately after hit test");
		reset_item_storage();native_point_value=0;score=0;player_left=304;
		for(unsigned value: {1000u,2000u,3000u,4000u,5000u,6000u,7000u,8000u,9000u,10000u,20000u,30000u,40000u,50000u,65530u,65530u}) {
			items_point[0]={304,350,0,2,IF_FALL,{0}};auto old_score=score;
			items_point_unput_update_render();
			require(native_point_value==value && score==old_score+int(value) && item.flag==IF_COLLECTED_OVER_CAP && item.flag_state.collect_time==15 && item.top==348,"Original point progression, score and same-frame popup");
		}
		auto final_score=score;items_point_unput_update_render();require(score==final_score,"Collected item cannot score twice");
		for(int i=0;i<14;++i) items_point_unput_update_render();require(item.flag==IF_FREE,"Point popup releases slot after sixteen ticks");
		items_point[0]={280,350,0,2,IF_FALL,{0}};items_point_unput_update_render();
		require(item.flag==IF_FALL,"Item/player hitbox excludes exact left boundary");
		items_point[0]={304,344,0,2,IF_FALL,{0}};items_point_unput_update_render();require(item.flag==IF_FALL,"Item/player hitbox excludes exact top boundary");
		items_point[0]={304,350,0,2,IF_SPLASH,{4}};items_point_unput_update_render();require(item.flag==IF_SPLASH,"Splash cannot be collected");
		items_point[0]={100,399,0,2,IF_BOUNCE,{0}};items_point_unput_update_render();require(item.flag==IF_FREE && native_point_value==0,"Missed point item resets point progression");
		int previous_bombs=-1;player.bombs_hud=[&](int previous) {previous_bombs=previous;};rem_bombs=4;
		items_bomb[0]={304,350,0,2,IF_FALL,{0}};items_bomb_unput_update_render();
		require(rem_bombs==5 && previous_bombs==4 && items_bomb[0].flag==IF_COLLECTED && items_bomb[0].flag_state.collect_time==15,"Bomb item adds bomb and updates HUD before popup");
		for(int i=0;i<15;++i) items_bomb_unput_update_render();require(items_bomb[0].flag==IF_FREE,"Bomb popup cleanup");
		items_bomb[0]={304,399,0,2,IF_FALL,{0}};final_score=score;items_bomb_unput_update_render();
		require(rem_bombs==5 && score==final_score+10000 && items_bomb[0].flag==IF_COLLECTED_OVER_CAP && items_bomb[0].top==366,"Bomb cap awards 10000 and clamps popup to screen");
		require(shots.sound_commands.back()==15,"Collection emits original SE command");
		std::cout << "Original item spawn, splash, fall, bounce, drop and collection dispatch passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

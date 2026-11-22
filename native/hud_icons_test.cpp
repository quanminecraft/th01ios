#include "hud_icons.hpp"
#include "extend_backend.hpp"
#include "item_runtime.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		for(int i=0;i<34;++i) sprites.images.push_back({32,32,Bytes(1024,7),Bytes(1024,1)});
		for(int y=0;y<16;++y) for(int x=16;x<32;++x) sprites.images[PTN_HUD].color[y*32+x]=2;
		ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		PlayerBackend player{shots,sprites,sprites,sprites,{},native_hud_bombs_render,{}};PlayerBackendBinding player_binding(player);
		std::int16_t lives=4;HudIconsBackend icons{*fb,lives,{}};HudIconsBinding icon_binding(icons);
		fb->clear(0,3);fb->clear(1,5);rem_bombs=3;page_access(0);native_hud_lives_init();native_hud_bombs_init();
		require(fb->pixel(0,128,0)==7 && fb->pixel(1,176,0)==7 && fb->pixel(0,128,16)==2 && fb->pixel(1,160,16)==2,"Initial life/bomb sprites copied to both pages");
		lives=2;native_hud_lives_render(4);
		require(fb->pixel(0,160,0)==3 && fb->pixel(1,176,0)==3 && fb->pixel(0,144,0)==7,"Life decrement restores removed icon backgrounds only");
		lives=5;native_hud_lives_render(2);
		require(fb->pixel(0,192,0)==7 && fb->pixel(1,192,0)==7 && fb->pixel(1,208,0)==5,"Life increase snapshots new slots and copies just the HUD row width");
		rem_bombs=1;native_hud_bombs_render(3);
		require(fb->pixel(0,144,16)==3 && fb->pixel(1,160,16)==3,"Bomb decrement restores both pages");
		page_access(1);native_hud_bombs_render(1);require(current_render_page()==1,"Unchanged count preserves accessed page");page_access(0);
		FontRom font(Bytes(0x46800));CardRenderer renderer{*fb,sprites,font,{}};CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);
		reset_item_storage();player_left=304;items_bomb[0]={304,350,0,2,IF_FALL,{0}};items_bomb_unput_update_render();
		require(rem_bombs==2 && fb->pixel(0,144,16)==2 && fb->pixel(1,144,16)==2,"Original item pickup updates actual bomb HUD");
		std::int8_t saved=5;ExtendBackend extend{lives,saved,native_hud_lives_render};ExtendBackendBinding extend_binding(extend);
		score=400000;extend_next=1;native_pellet_speed=0;score_extend_update_and_render();
		require(lives==6 && saved==6 && fb->pixel(0,208,0)==7 && fb->pixel(1,208,0)==7,"Original score extend updates actual life HUD");
  icons.backgrounds.clear();icons.hide_lives=true;lives=0;native_hud_lives_init();
  lives=1;native_hud_lives_render(0);lives=0;native_hud_lives_render(1);
  require(icons.backgrounds.empty(),"Challenge empty-stock HUD never restores an uncaptured fictitious life icon");
		std::cout << "Original HUD icon snapshots, changes, both pages and item/extend integration passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

#include "hud_score.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		Bytes bytes(0x46800);for(int d=0;d<10;++d) bytes[0x1800+(0x23-0x21)*96*32+(0x30+d-0x20)*32]=0x80u>>(d%8);
		FontRom font(std::move(bytes));CardRenderer renderer{*fb,sprites,font,{}};CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);
		std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};hud.hiscore=5000;HudIconsBinding hud_binding(hud);HudScoreBinding score_binding(events);
		fb->clear(0,3);fb->clear(1,5);score=0;cardcombo_cur=0;cardcombo_max=0;page_access(0);native_hud_score_init(true);
		require(hud.backgrounds.size()==18 && fb->pixel(0,256,16)==7 && fb->pixel(1,256,16)==7 && current_render_page()==0,"Score/combo initialization captures original digit backgrounds and copies both rows");
		score=100;cardcombo_cur=1;hud_score_and_cardcombo_render();
		require(events.hud.size()==1 && hud.hiscore==5000 && hud_cardcombo_max==1 && fb->pixel(0,321,16)==7 && fb->pixel(1,321,16)==7,"Score dispatch draws changed digits on both pages and preserves higher hi-score");
		score=99;cardcombo_cur=0;hud_score_and_cardcombo_render();
		require(fb->pixel(0,321,16)==5 && fb->pixel(1,321,16)==5 && hud_cardcombo_max==1,"Decreasing digit restores captured background without losing max combo");
		score=6000;cardcombo_cur=12;hud_score_and_cardcombo_render();
		require(hud.hiscore==6000 && hud_cardcombo_max==12 && fb->pixel(0,310,0)==7 && fb->pixel(1,310,0)==7 && fb->pixel(0,308,0)==5,"New hi-score draws bold changed digits and clears previous glyph");
		cardcombo_cur=3;hud_score_and_cardcombo_render();
		require(hud_cardcombo_max==12 && fb->pixel(0,401,16)==5 && fb->pixel(0,401,0)==7,"Current combo resets independently of displayed maximum");
		fb->put(0,256,16,9);fb->put(1,256,16,9);fwnum_force_rerender=true;hud_score_and_cardcombo_render();fwnum_force_rerender=false;
		require(fb->pixel(0,256,16)==7 && fb->pixel(1,256,16)==7,"Forced current-score redraw restores unchanged digits");
		StageLayout layout;layout.cards={{64,96,0,0}};load_cards(layout);orb_cur_left=64;orb_cur_top=96;test_damage=false;bomb_damaging=false;rank=RANK_NORMAL;card_flip_cycle=0;
		auto old_score=score;auto old_events=events.hud.size();cards_hittest(0);
		require(score>old_score && hud.hiscore==unsigned(score) && events.hud.size()==old_events+1,"Original card collision updates actual score HUD and hi-score synchronously");
		std::cout << "Original score/hi-score/combo initialization, digit backgrounds and card dispatch passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

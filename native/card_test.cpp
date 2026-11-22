#include "card_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		ShotBackend shot_backend{*fb,sprites,{}};ShotBackendBinding shot_binding(shot_backend);
		CardEvents events;CardBackendBinding binding(events);
		auto reset=[&](unsigned count) {
			StageLayout stage;for(unsigned i=0;i<count;++i) stage.cards.push_back({0,64,0,0});load_cards(stage);
			score=0;cardcombo_cur=0;cardcombo_max=0;card_flip_cycle=0;rank=RANK_NORMAL;
			test_damage=false;bomb_damaging=false;bomb_frame=0;native_random_seed=1;events={};shot_backend.sound_commands.clear();
			orb_cur_left=23;orb_cur_top=87;
		};
		reset(3);cards_hittest(0);
		require(cards_score==std::vector<std::uint32_t>({100,120,180}) && score==400 && cardcombo_cur==3 && cardcombo_max==3,"Card scoring order and combo formula");
		require(events.hud.size()==3 && events.hud[1].score==220 && events.items.size()==1 && !events.items[0].bomb,"Ordered HUD/item requests");
		require(shot_backend.sound_commands==std::vector<unsigned>({2,2,2}),"Card SE order");
		cards_hittest(0);require(score==400,"Flipping cards must not score twice");
		reset(1);orb_cur_left=24;cards_hittest(0);require(cards.flag[0]==CARD_ALIVE,"Strict card X boundary");
		orb_cur_left=0;orb_cur_top=88;cards_hittest(0);require(cards.flag[0]==CARD_ALIVE,"Strict card Y boundary");
		reset(3);rank=RANK_LUNATIC;cards_hittest(5);require(score==775,"Lunatic multiplier and scene base score");
		reset(1);cardcombo_cur=40;cards_hittest(0);require(cards_score[0]==25600,"Per-card score cap");
		reset(1);cardcombo_cur=182;cards_hittest(0);require(cards_score[0]==25600,"Original signed 16-bit square truncation");
		reset(1);cardcombo_cur=256;cards_hittest(0);require(cards_score[0]==100,"256 squared wraps to zero before the original unsigned cast");
		reset(1);orb_cur_left=600;test_damage=true;cards_hittest(0);require(cards.flag[0]==CARD_FLIPPING,"Debug damage bypasses geometry");
		reset(3);orb_cur_left=600;bomb_damaging=true;bomb_frame=1;cards_hittest(0);
		require(cards.flag==std::vector<std::int8_t>({CARD_ALIVE,CARD_FLIPPING,CARD_ALIVE}),"Bomb selects frame-modulo card before RNG");
		reset(1);card_flip_cycle=140;cards_hittest(0);require(card_flip_cycle==1 && events.items.size()==1 && events.items[0].bomb,"Flip cycle produces bomb item and wraps");
		reset(1);card_flip_cycle=140;bomb_damaging=true;cards_hittest(0);require(!events.items[0].bomb,"Bomb damage converts bomb-item drop to point item");
		reset(0);bomb_damaging=true;cards_hittest(0);require(score==0 && events.hud.empty(),"Empty card list");
		std::cout << "Original card collision, combo, scoring, drop cycle and bomb-selection tests passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

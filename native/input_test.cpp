#include "input_backend.hpp"
#include <iostream>
#include <stdexcept>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		InputKeys keys;
		auto sense=[&]() {set_input_keys(keys);input_sense(false);};
		auto reset=[&]() {keys={};set_input_keys(keys);end_flag=ES_NONE;mode_test=false;bomb_doubletap_frame=0;player_is_hit=false;input_reset_sense();};
		reset();keys.shot=true;sense();require(input_shot,"Shot press");
		input_shot=false;sense();require(!input_shot,"Held shot must not re-arm after gameplay consumes it");
		keys.shot=false;sense();keys.shot=true;sense();require(input_shot,"Shot re-press");
		reset();keys.strike=true;sense();input_strike=false;sense();require(!input_strike,"Held strike must not re-arm");
		keys.strike=false;sense();keys.strike=true;sense();require(input_strike,"Strike re-press");
		reset();keys.left=true;sense();keys.num_left=true;sense();keys.left=false;sense();
		require(input_lr==INPUT_LEFT,"Numpad/arrow direction merging");
		keys.right=true;sense();require(input_lr==INPUT_LEFT_RIGHT,"Opposite directions remain a combined input");
		keys.num_left=false;sense();require(input_lr==INPUT_RIGHT,"Release one of the opposing directions");
		reset();keys.num_up=true;keys.num_right=true;end_flag=1;sense();require(!input_up && input_lr==INPUT_NONE,"Cutscene excludes numpad controls");
		keys.up=true;sense();require(input_up,"Cutscene still senses arrows");
		reset();keys.escape=true;sense();require(paused,"Escape toggles pause");
		sense();require(paused,"Holding escape must not toggle repeatedly");
		keys.escape=false;sense();keys.escape=true;sense();require(!paused,"Escape re-press toggles pause off");
		reset();keys.escape=true;sense();keys.escape=false;keys.shot=true;keys.enter=true;sense();
		require(input_ok && player_is_hit,"Preserve original paused-shot/Enter player-hit quirk");
		auto tap=[&](bool shot) {keys.shot=shot;keys.strike=!shot;sense();keys.shot=false;keys.strike=false;sense();};
		reset();bomb_doubletap_frame=19;tap(true);tap(false);tap(true);tap(false);
		require(input_bomb,"Double-tap both attacks within the original window");
		reset();tap(true);tap(false);bomb_doubletap_frame=20;tap(true);tap(false);
		require(!input_bomb,"Frame 20 expires and clears the other attack counter");
		reset();keys.left=true;keys.right=true;keys.shot=true;sense();require(input_bomb,"Left+right+shot bomb shortcut");
		keys={};set_input_keys(keys);input_reset_sense();require(!input_bomb && !paused && !input_shot && !input_strike,"Input reset clears latched state");
		std::cout << "Original input edge/consume, direction merge, pause and bomb-combo tests passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

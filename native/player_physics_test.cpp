#include "th01/main/player/player.hpp"
#include "th01/main/player/orb.hpp"
#include <iostream>
static void require(bool value) {if(!value) throw std::runtime_error("Player physics test failed");}
int main() {
	static_assert(sizeof(player_left)==2 && sizeof(player_invincibility_time)==2);
	player_left=2;player_move_and_clamp(-4);require(player_left==0);
	player_left=606;player_move_and_clamp(4);require(player_left==608);
	player_left=300;player_move_and_clamp(4);require(player_left==304);
	player_left=100;orb_cur_left=123;orb_cur_top=353;
	player_is_hit=false;player_invincible=false;orb_player_hittest(OR_NONE);require(player_is_hit);
	player_is_hit=false;orb_cur_left=124;orb_player_hittest(OR_NONE);require(!player_is_hit);
	orb_cur_left=100;orb_cur_top=352;orb_player_hittest(OR_NONE);require(!player_is_hit);
	orb_cur_top=353;player_invincible=true;orb_player_hittest(OR_NONE);require(!player_is_hit);
	player_invincible=false;orb_cur_top=337;orb_cur_left=99;
	orb_player_hittest(1);require(orb_velocity_x==OVX_4_LEFT && orb_force==-13);
	orb_cur_left=101;orb_player_hittest(3);require(orb_velocity_x==OVX_4_RIGHT && orb_force==-12);
	orb_cur_left=139;orb_player_hittest(OR_3_X_8_LEFT);
	require(orb_velocity_x==OVX_8_LEFT && orb_force==-10);
	orb_force=9;orb_cur_left=140;orb_player_hittest(OR_3_X_8_RIGHT);require(orb_force==9);
	// Original centered repel makes two independent RNG calls and can only
	// introduce leftward velocity, including its documented second-call quirk.
	orb_cur_left=100;native_random_seed=1;orb_player_hittest(2);
	require(orb_velocity_x!=OVX_4_RIGHT && native_random_seed!=1 && orb_force==-12);
	std::cout << "Original player clamp, strict hitbox, invincibility and repel tests passed\n";
}

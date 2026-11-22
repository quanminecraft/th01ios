				frame_rand++;
				pellet_speed_raise_cycle = (
					1800 - (rem_lives * 200) - (rem_bombs * 50)
				);
				if((frame_rand % pellet_speed_raise_cycle) == 0) {
					pellet_speed_raise(0.025f);
				}
				input_sense(false);

				if(player_invincibility_time > 1) {
					player_invincibility_time--;
					player_invincible = true;
				} else if(player_invincibility_time == 1) {
					player_invincible = false;
					player_invincibility_time = 0;
				}

				player_unput_update_render();
				items_unput_update_render();
				frame_since_start_of_binary++;
				orb_force_frame++;
				bomb_frame++;
				bomb_doubletap_frame++;

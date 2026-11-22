enum continue_line_t {
	TITLE, YES_SELECTED, NO_SELECTED, YES, NO
};

inline void continue_back_put(
	continue_line_t line, th01_vram_colors_t col, const shiftjis_t str[]
) {
	graph_putsa_fx(
		0, (line * GLYPH_H), (FX_CLEAR_BG | FX_WEIGHT_BOLD | col), str
	);
}

inline void continue_back_2xscale_to_front(
	screen_x_t left_0, screen_y_t top_0, pixel_t w, continue_line_t line
) {
	graph_2xscale_byterect_1_to_0_slow(
		left_0, top_0, 0, (line * GLYPH_H), w, GLYPH_H
	);
}

inline void continue_choice_put(continue_line_t line) {
	continue_back_2xscale_to_front(
		((RES_X / 2) - (GLYPH_FULL_W * 2)),
		((RES_Y / 2) + (((line - YES_SELECTED) % 2) * (GLYPH_H * 2))),
		CONTINUE_CHOICE_W,
		line
	);
}

inline void continue_choice_render(bool yes) {
	continue_choice_put(yes ? YES_SELECTED : YES);
	continue_choice_put(yes ? NO : NO_SELECTED);
}

bool16 continue_menu(void)
{
	bool16 sel;
	unsigned int frame = 0;

	graph_copy_page_to_other(1);
	page_access(1);
	continue_back_put(TITLE, V_WHITE, CONTINUE_TITLE);
	continue_back_put(YES_SELECTED, V_WHITE, CONTINUE_YES_1);
	continue_back_put(NO_SELECTED, V_WHITE, CONTINUE_NO_1);
	continue_back_put(YES, V_GRAY, CONTINUE_YES_2);
	continue_back_put(NO, V_GRAY, CONTINUE_NO_2);

	page_access(0);
	continue_back_2xscale_to_front(
		(RES_X / 4), PLAYFIELD_TOP, CONTINUE_TITLE_W, TITLE
	);

	continue_choice_render(true);
	sel = true;
	input_ok = false;
	paused = false;
	input_shot = false;
	input_reset_sense();

	// If we don't, we quit back to the main menu anyway.
	resident_continue_use();
	if(resident->score_highest < score) {
		resident->score_highest = score;
	}
	resident->score = 0;
	resident->rem_lives = (resident->credit_lives_extra + 2);
	pellet_speed_lower(-2, -5);

	while(1) {
		input_sense(false);
		frame++;
		frame_delay(1);

		if((input_ok == true) || (input_shot == true)) {
			if(sel == true) {
				resident->snd_need_init = false;
				resident->point_value = 0;
				#ifdef TH01_NATIVE_ORB_PHYSICS
				return true; // Host performs the original MAIN reload.
#else
				game_switch_binary(EP_MAIN);
#endif
			} else {
				no_continue:
				player_is_hit = false;
				paused = false;
				mdrv2_bgm_stop();
				return false;
			}
		}
		if(frame > 3000) {
			goto no_continue;
		}
		if(paused == true) {
			player_is_hit = false;
			paused = false;
			return false;
		}
		if(input_up == true) {
			sel = true;
			continue_choice_render(true);
		}
		if(input_down == true) {
			sel = false;
			continue_choice_render(false);
		}
	};
}
// -------------

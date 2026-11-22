/// Main menu
/// ---------
/// Terminology: "Choice" = Top-level menu selection (not amount of values)

static const vc_t COL_ACTIVE = 15;
static const vc_t COL_INACTIVE = 5;
static const int16_t FX = FX_WEIGHT_BLACK;

// Coordinates
// -----------

static const screen_x_t MENU_LEFT = 220;
static const screen_x_t MENU_CENTER_X = 316;
static const screen_y_t MENU_CENTER_Y = 316;
static const pixel_t MENU_W = 176;

static const pixel_t CHOICE_PADDED_H = 20;
static const int CHOICE_COUNT_MAX = 5;

static const int TRACK_COUNT = 15;

// ZUN bloat: Needed to circumvent 16-bit promotion in a single comparison.
inline int8_t track_count(void) {
	return TRACK_COUNT;
}

#define choice_top_0(choice_count) ( \
	(MENU_CENTER_Y - (((choice_count) * CHOICE_PADDED_H) / 2)) \
)

#define choice_top(choice, choice_count) ( \
	(choice_top_0(choice_count) + (choice * CHOICE_PADDED_H)) \
)

#define music_choice_top(choice, choice_count) ( \
	(choice_top_0(choice_count + 1) + (choice * (CHOICE_PADDED_H * 2))) \
)
// -----------

inline void menu_unput(int choice_count) {
	egc_copy_rect_1_to_0_16(
		MENU_LEFT,
		choice_top_0(choice_count),
		MENU_W,
		(choice_count * CHOICE_PADDED_H)
	);
}

#define choice_put(left, top, col, str) { \
	graph_printf_fx(left, top, (col | FX), "%s", str); \
}

#define choice_put_value(left, top, col, str, val) { \
	graph_printf_fx(left, top, (col | FX), "%s%s", str, val); \
}

#define choice_render_if_changed(prev, cur, unput_and_put_func) { \
	if(cur != prev) { \
		unput_and_put_func(prev, COL_INACTIVE); \
		unput_and_put_func(cur, COL_ACTIVE); \
		prev = cur; \
	} \
}

void title_hit_key_put(int frame)
{
	enum {
		LEFT = (((RES_X / 2) - GLYPH_FULL_W) - (shiftjis_w(HIT_KEY) / 2)),
		TOP = choice_top(0, 1),
	};
	if((frame % 70) < 50) {
		graph_putsa_fx(LEFT, TOP, (FX_WEIGHT_BOLD | COL_ACTIVE), HIT_KEY);
	} else {
		egc_copy_rect_1_to_0_16_word_w(LEFT, TOP, shiftjis_w(HIT_KEY), GLYPH_H);
	}
}

void main_choice_unput_and_put(int choice, vc2 col)
{
	const shiftjis_t* CHOICES[MAIN_CHOICE_COUNT] = MAIN_CHOICES;

	screen_x_t left = (MENU_CENTER_X - (MAIN_CHOICE_W / 2));
	screen_y_t top = choice_top(choice, MAIN_CHOICE_COUNT);

	// No unblitting necessary here, as only the colors change.
	graph_putsa_fx(left, top, (col | FX), CHOICES[choice]);
}

void option_choice_unput_and_put(int choice, vc2 col)
{
	const shiftjis_t* CHOICES[OPTION_CHOICE_COUNT] = OPTION_CHOICES;
	const shiftjis_t* RANKS[RANK_COUNT] = RANKS_CAPS_CENTERED;
	const shiftjis_t* MUSIC_MODES[BGM_MODE_COUNT] = BGM_MODES_CENTERED;
	const shiftjis_t* START_LIVES[CFG_CREDIT_LIVES_EXTRA_MAX] = {
		"   3  ",
		"   4  ",
		"   5  ",
		"   6  ",
		"   7  ",
	};

	screen_x_t left = (MENU_CENTER_X - (MENU_W / 2));
	screen_y_t top = choice_top(choice, OPTION_CHOICE_COUNT);

	egc_copy_rect_1_to_0_16(left, top, MENU_W, GLYPH_H);

	if(choice == 0) {
		choice_put_value(left, top, col, CHOICES[choice], RANKS[opts.rank]);
	} else if(choice == 1) {
		choice_put_value(
			left, top, col, CHOICES[choice], MUSIC_MODES[opts.bgm_mode]
		);
	} else if(choice == 2) {
		choice_put_value(
			left,
			top,
			col,
			CHOICES[choice],
			START_LIVES[opts.credit_lives_extra]
		);
	} else if(choice == 3) {
		choice_put(left, top, col, CHOICES[choice]);
	} else if(choice == 4) {
		choice_put(left, top, col, CHOICES[choice]);
	}
}

int8_t music_sel = 0;

void music_choice_unput_and_put(int choice, vc2 col)
{
	const shiftjis_t* CHOICES[MUSIC_CHOICE_COUNT]  = MUSIC_CHOICES;
	const shiftjis_t* TITLES[TRACK_COUNT]  = MUSIC_TITLES;

	screen_x_t left = (MENU_CENTER_X - (MENU_W / 2));
	screen_y_t top = music_choice_top(choice, MUSIC_CHOICE_COUNT);

	egc_copy_rect_1_to_0_16(left, top, MENU_W, GLYPH_H);

	if(choice == 0) {
		// ZUN landmine: That's larger than the menu?
		egc_copy_rect_1_to_0_16(
			left, (top + CHOICE_PADDED_H), (MENU_W + GLYPH_FULL_W), GLYPH_H
		);
		graph_printf_fx(
			left, top, (col | FX), "%s%.2d", CHOICES[choice], music_sel
		);
		choice_put(left, (top + CHOICE_PADDED_H), col, TITLES[music_sel]);
	} else if(choice == 1) {
		choice_put(left, top, col, CHOICES[choice]);
	}
}

void main_update_and_render(void)
{
	OP_LOCAL(int, sel_prev, 99);
	OP_LOCAL(bool16, in_this_menu, false);

	if(!in_this_menu) {
		menu_unput(CHOICE_COUNT_MAX);
		main_choice_unput_and_put(0, COL_INACTIVE);
		main_choice_unput_and_put(1, COL_INACTIVE);
		main_choice_unput_and_put(2, COL_INACTIVE);
		main_choice_unput_and_put(3, COL_INACTIVE);

		main_choice_unput_and_put(menu_sel, COL_ACTIVE);
		in_this_menu = true;
		sel_prev = menu_sel;
	}
	choice_render_if_changed(sel_prev, menu_sel, main_choice_unput_and_put);

	if(input_ok || input_shot) {
		switch(menu_sel) {
		case 0:
			start_game(true);
			break;
		case 1:
			start_game(false);
			break;
		case 2:
			menu_id = MID_OPTION;
			in_this_menu = false;
			break;
		case 3:
			quit = true;
			break;
		}
	}
	if(input_cancel) {
		quit = true;
	}
}

void option_update_and_render(void)
{
	OP_LOCAL(bool16, in_this_menu, false);
	OP_LOCAL(int, sel_prev, 0);
	OP_LOCAL(bool16, left_locked, false);
	OP_LOCAL(bool16, right_locked, false);

	if(!in_this_menu) {
		menu_sel = 0;
		sel_prev = 0;
		in_this_menu = true;
		option_choice_max = (OPTION_CHOICE_COUNT - 1);

		menu_unput(max_macro(MAIN_CHOICE_COUNT, MUSIC_CHOICE_COUNT));
		option_choice_unput_and_put(0, COL_ACTIVE);

		option_choice_unput_and_put(1, COL_INACTIVE);
		option_choice_unput_and_put(2, COL_INACTIVE);
		option_choice_unput_and_put(3, COL_INACTIVE);
		option_choice_unput_and_put(4, COL_INACTIVE);
	}
	choice_render_if_changed(sel_prev, menu_sel, option_choice_unput_and_put);

	on_condition_if_not_locked_2((input_left == true), left_locked, {
		switch(menu_sel) {
		case 0:
			ring_dec_16(opts.rank, RANK_LUNATIC);
			break;
		case 1:
			ring_dec_16(
				OP_BGM_REF, (BGM_MODE_COUNT - 1)
			);
			break;
		case 2:
			ring_dec_16(
				opts.credit_lives_extra, (CFG_CREDIT_LIVES_EXTRA_MAX - 1)
			);
			break;
		}
		option_choice_unput_and_put(menu_sel, COL_ACTIVE);
	});

	on_condition_if_not_locked_2((input_right == true), right_locked, {
		switch(menu_sel) {
		case 0:
			ring_inc(opts.rank, RANK_LUNATIC);
			break;
		case 1:
			ring_inc(OP_BGM_REF, (BGM_MODE_COUNT - 1));
			break;
		case 2:
			ring_inc(opts.credit_lives_extra, (CFG_CREDIT_LIVES_EXTRA_MAX - 1));
			break;
		}
		option_choice_unput_and_put(menu_sel, COL_ACTIVE);
	});

	if(((input_ok || input_shot) && (menu_sel == 4)) || input_cancel) {
		menu_id = MID_UPDATE_BGM_MODE__DELAY__SWITCH_TO_MAIN;
		in_this_menu = false;
		menu_sel = 2; // Option in the main menu
	}
	if((input_ok || input_shot) && (menu_sel == 3)) {
		menu_id = MID_MUSIC;
		in_this_menu = false;
		menu_sel = 0; // Track selection in the Music Test
	}
}

void music_play_selected(void)
{
	const char* FILES[TRACK_COUNT] = MUSIC_FILES;
	mdrv2_bgm_stop();
	mdrv2_bgm_load(FILES[music_sel]);
	mdrv2_bgm_play();
}

void music_update_and_render(void)
{
	OP_LOCAL(bool16, in_this_menu, false);
	OP_LOCAL(int, sel_prev, 0);
	OP_LOCAL(bool16, left_locked, false);
	OP_LOCAL(bool16, right_locked, false);

	if(!in_this_menu) {
		menu_sel = 0;
		sel_prev = 0;
		in_this_menu = true;

		// This is not a functional way of locking these keys...
		input_ok = false;
		input_shot = false;

		option_choice_max = (MUSIC_CHOICE_COUNT - 1);

		menu_unput(CHOICE_COUNT_MAX);
		music_choice_unput_and_put(0, COL_ACTIVE);
		music_choice_unput_and_put(1, COL_INACTIVE);
	}
	choice_render_if_changed(sel_prev, menu_sel, music_choice_unput_and_put);

	on_condition_if_not_locked_2((input_left == true), left_locked, {
		if(menu_sel == 0) {
			ring_dec(music_sel, (TRACK_COUNT - 1));
		}
		music_choice_unput_and_put(menu_sel, COL_ACTIVE);
	});

	on_condition_if_not_locked_2((input_right == true), right_locked, {
		if(menu_sel == 0) {
			ring_inc_ge(music_sel, track_count());
		}
		music_choice_unput_and_put(menu_sel, COL_ACTIVE);
	});

	if(((input_ok || input_shot) && (menu_sel == 1)) || input_cancel) {
		menu_id = MID_DELAY__SWITCH_TO_OPTION;
		in_this_menu = false;
		menu_sel = 3; // Music Test in the option menu
	}
	if((input_ok || input_shot) && (menu_sel == 0)) {
		music_play_selected();
	}
}


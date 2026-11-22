template <class T1, class T2> inline void fwnum_put(
	screen_x_t left,
	screen_y_t top,
	int fx,
	int digits,
	const T1 &val,
	const T2 &val_prev
) {
	graph_putfwnum_fx(
		left, top, fx, digits, val,
		(fwnum_force_rerender == true) ? 0 : val_prev, true
	);
}

#define score_bg(func, digit, top, ptn_id_base) \
	ptn_id_and_quarter_from_i(func, SCORE_LEFT, digit, top, ptn_id_base, digit)

#define score_max_bg(func, digit) \
	score_bg(func, digit, MAX_TOP, PTN_BG_MAX_SCORE)

inline void score_put(screen_y_t top, int fx, const int32_t &prev) {
	fwnum_put(SCORE_LEFT, top, fx, SCORE_DIGITS, score, prev);
}

#define cardcombo_bg(func, digit, top, ptn_id) \
	func(CARDCOMBO_LEFT, digit, top, ptn_id, digit)

#define cardcombo_max_bg(func, digit) \
	cardcombo_bg(func, digit, MAX_TOP, PTN_BG_MAX_CARDCOMBO)

inline void cardcombo_put(screen_y_t top, int fx, const int16_t &prev) {
	fwnum_put(CARDCOMBO_LEFT, top, fx, CARDCOMBO_DIGITS, cardcombo_cur, prev);
}

void hiscore_update_and_render(void)
{
	static int32_t prev = 0;
	int32_t divisor = 1000000; // Must match SCORE_DIGITS!
	uint32_t hiscore = resident_hiscore;

	if(hiscore >= score) {
		return;
	}
	for(int i = 0; i < SCORE_DIGITS; i++) {
		if(digit_changed(score, prev, divisor)) {
			page_access(1);	score_max_bg(bg_put, i);
			page_access(0);	score_max_bg(bg_put, i);
		}
		divisor /= 10;
	}
	page_access(1);	score_put(MAX_TOP, MAX_FX, prev);
	page_access(0);	score_put(MAX_TOP, MAX_FX, prev);

	prev = score;
	resident_hiscore = score;
}

void cardcombo_max_render(void)
{
	static int16_t prev;
	int divisor = 10; // Must match CARDCOMBO_DIGITS!

	for(int i = 0; i < CARDCOMBO_DIGITS; i++) {
		if(digit_changed(cardcombo_cur, prev, divisor)) {
			page_access(1);	cardcombo_max_bg(bg_put, i);
			page_access(0);	cardcombo_max_bg(bg_put, i);
		}
		divisor /= 10;
	}
	page_access(1);	cardcombo_put(MAX_TOP, MAX_FX, prev);
	page_access(0);	cardcombo_put(MAX_TOP, MAX_FX, prev);

	prev = cardcombo_cur;
}

void hud_score_and_cardcombo_render(void)
{
	static score_t score_prev;
	static int16_t cardcombo_prev;

	int digit;
	int page;
	int cardcombo_divisor;
	score_t score_divisor;

	score_divisor = 1000000; // Must match SCORE_DIGITS!
	cardcombo_divisor = 10; // Must match CARDCOMBO_DIGITS!
	for(page = 1; page >= 0; page--) {
		page_access(page);
		score_divisor = 1000000; // Must match SCORE_DIGITS!
		cardcombo_divisor = 10; // Must match CARDCOMBO_DIGITS!

		for(digit = 0; digit < SCORE_DIGITS; digit++) {
			if(digit_changed(score, score_prev, score_divisor)) {
				score_bg(bg_put, digit, CUR_TOP, PTN_BG_CUR_SCORE);
			}
			score_divisor /= 10;
		}
		score_put(CUR_TOP, CUR_FX, score_prev);

		for(digit = 0; digit < CARDCOMBO_DIGITS; digit++) {
			if(digit_changed(cardcombo_cur, cardcombo_prev, cardcombo_divisor)) {
				cardcombo_bg(bg_put, digit, CUR_TOP, PTN_BG_CUR_CARDCOMBO);
			}
			cardcombo_divisor /= 10;
		}
		cardcombo_put(CUR_TOP, CUR_FX, cardcombo_prev);
	}

	score_prev = score;
	cardcombo_prev = cardcombo_cur;
	hiscore_update_and_render();
	if(hud_cardcombo_max < cardcombo_cur) {
		hud_cardcombo_max = cardcombo_cur;
		cardcombo_max_render();
	}
}

#define cardcombo_bg_loop(func, digit, top, ptn_id) \
	for(digit = 0; digit < CARDCOMBO_DIGITS; digit++) { \
		cardcombo_bg(func, digit, top, ptn_id); \
	} \

inline void cardcombo_bg_snap_and_put(screen_y_t top, int fx) {
	graph_putfwnum_fx(CARDCOMBO_LEFT, top, fx, CARDCOMBO_DIGITS, 0, 99, true);
}

#define score_bg_snap_and_put(digit, top, ptn_id, fx, score) \
	page_access(1); \
	for(digit = 0; digit < SCORE_DIGITS; digit++) { \
		score_bg(bg_snap, digit, top, ptn_id); \
	} \
	page_access(0); \
	graph_putfwnum_fx(SCORE_LEFT, top, fx, SCORE_DIGITS, score, 0, true);

// Setting [first_run] to false will only reset the card combo display.
void score_and_cardcombo_bg_snap_and_put(bool16 first_run)
{
	int digit;

	// Spot the difference… :(
	if(first_run) {
		score_bg_snap_and_put(digit, CUR_TOP, PTN_BG_CUR_SCORE, CUR_FX, score);
		page_access(1);
		cardcombo_bg_loop(bg_snap, digit, CUR_TOP, PTN_BG_CUR_CARDCOMBO);
	} else {
		cardcombo_bg_loop(bg_put, digit, CUR_TOP, PTN_BG_CUR_CARDCOMBO);
	}
	page_access(0);
	cardcombo_bg_snap_and_put(CUR_TOP, CUR_FX);

	if(first_run) {
		score_bg_snap_and_put(
			digit, MAX_TOP, PTN_BG_MAX_SCORE, MAX_FX, resident_hiscore
		);
		page_access(1);
		cardcombo_bg_loop(bg_snap, digit, MAX_TOP, PTN_BG_MAX_CARDCOMBO);
		page_access(0);
	} else {
		cardcombo_bg_loop(bg_put, digit, MAX_TOP, PTN_BG_MAX_CARDCOMBO);
	}
	cardcombo_bg_snap_and_put(MAX_TOP, MAX_FX);

	graph_copy_hud_row_0_to_1_8(SCORE_LEFT, MAX_TOP, SCORE_AND_CARDCOMBO_W);
	graph_copy_hud_row_0_to_1_8(SCORE_LEFT, CUR_TOP, SCORE_AND_CARDCOMBO_W);
	hud_cardcombo_max = 0;
}


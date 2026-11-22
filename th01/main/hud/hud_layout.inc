inline void stage_unput(void) {
	ptn_put_quarter_noalpha_8(STAGE_LEFT, STAGE_TOP, PTN_BG_STAGE, 0);
}

inline void stage_num_to(char tmp_str[STAGE_DIGITS + 1]) {
	str_right_aligned_from_uint16(tmp_str, stage_num, STAGE_DIGITS);
}

inline void stage_put(const char str[STAGE_DIGITS + 1]) {
	graph_putsa_fx(STAGE_LEFT, STAGE_TOP, MAX_FX, str);
}

void stage_bg_snap_and_put(void)
{
	char str[STAGE_DIGITS + 1];

	bg_snap(STAGE_LEFT, 0, STAGE_TOP, PTN_BG_STAGE, 0);
	stage_num_to(str);
	stage_put(str);
	graph_copy_hud_row_0_to_1_8(
		STAGE_LEFT, STAGE_TOP, STAGE_DIGITS * GLYPH_HALF_W
	);
}

void rank_put(void)
{
	static const shiftjis_t* RANKS[RANK_COUNT] = RANKS_CAPS;
	graph_putsa_fx(
		(RANK_CENTER_X - (text_extent(RANKS[rank]) / 2)),
		RANK_TOP,
		MAX_FX,
		RANKS[rank]
	);
}

void hud_bg_snap_and_put(void)
{
	char str[STAGE_DIGITS + 1];

	page_access(1); hud_bg_put();
	page_access(0); hud_bg_put();

	if(first_stage_in_scene == true) {
		lives_bg_snap_and_put();
		bombs_bg_snap_and_put();
		stage_bg_snap_and_put();
		timer_bg_snap_and_put();
	}

	page_access(1);
	rank_put();	stage_unput();	stage_num_to(str);	stage_put(str);
	page_access(0);
	rank_put();	stage_unput();	stage_put(str);

	hud_cardcombo_max = 0;
	if(first_stage_in_scene == true) {
		score_and_cardcombo_bg_snap_and_put(true);
	} else {
		score_and_cardcombo_bg_snap_and_put(false);
	}
	timer_put();

	fwnum_force_rerender = true;
	hud_score_and_cardcombo_render();
	fwnum_force_rerender = false;
}

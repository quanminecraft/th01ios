static const main_ptn_slot_t PTN_SLOT_BG_ENT = PTN_SLOT_BOSS_1;
static const main_ptn_slot_t PTN_SLOT_MISSILE = PTN_SLOT_BOSS_2;
// ----

#define bg_func_init(left, top, unnecessary) { \
	ent_attack.pos_cur_set_to(ent_still_or_wave); \
	if(unnecessary == 1) { \
		left = ent_still_or_wave.cur_left; \
		top = ent_still_or_wave.cur_top; \
	} else if(unnecessary == 2) { \
		left = ent_attack.cur_left; \
		top = ent_attack.cur_top; \
	} \
}

void girl_bg_snap(unnecessary_1_or_2_t unnecessary)
{
	int ptn_x;
	int ptn_y;
	screen_x_t left;
	screen_y_t top;
	int image;

	bg_func_init(left, top, unnecessary);

	image = 0;
	ptn_snap_rect_from_1_8(
		left, top, GIRL_W, GIRL_H, PTN_SLOT_BG_ENT, image, ptn_x, ptn_y
	);
}

void girl_bg_put(unnecessary_1_or_2_t unnecessary)
{
	int ptn_x;
	int ptn_y;
	screen_x_t left;
	screen_y_t top;
	int image = 0;

	bg_func_init(left, top, unnecessary);
	ptn_put_rect_noalpha_8(
		left, top, GIRL_W, GIRL_H, PTN_SLOT_BG_ENT, image, ptn_x, ptn_y
	);
}


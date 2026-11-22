#define form_shot_hitbox_w(form) ( \
	(form == F_GIRL) \
		? shot_hitbox_w((GIRL_W / 8) * 5) \
		: shot_hitbox_w(BAT_W) \
)
#define form_shot_hitbox_h(form) ( \
	(form == F_GIRL) \
		? shot_hitbox_h((GIRL_H / 3) * 2) \
		: shot_hitbox_h(( BAT_H * 3) / 2) \
)

inline screen_x_t form_center_x(elis_form_t form) {
	return (form == F_GIRL)
		? ent_still_or_wave.cur_center_x()
		: ent_bat.cur_center_x();
}

inline screen_y_t form_center_y(elis_form_t form) {
	return (form == F_GIRL)
		? ent_still_or_wave.cur_center_y()
		: ent_bat.cur_center_y();
}

inline screen_x_t form_shot_hitbox_left(elis_form_t form) {
	return (form == F_GIRL)
		? (ent_still_or_wave.cur_left + (GIRL_W / 4))
		: ent_bat.cur_left;
}

inline screen_y_t form_shot_hitbox_top(elis_form_t form) {
	return (form == F_GIRL)
		? (ent_still_or_wave.cur_top + (GIRL_H / 3))
		: ent_bat.cur_left; // ZUN quirk: Should be cur_top
}

inline screen_x_t girl_lefteye_x(void) {
	return (ent_still_or_wave.cur_left + 60);
}

inline screen_y_t girl_lefteye_y(void) {
	return (ent_still_or_wave.cur_top + 28);
}

inline screen_x_t girl_wing_left_center_x(void) {
	return (form_center_x(F_GIRL) - 38);
}

inline screen_x_t girl_wing_right_center_x(void) {
	return (form_center_x(F_GIRL) + 40);
}

inline screen_y_t girl_wing_center_y(void) {
	return (form_center_y(F_GIRL) - 4);
}

#define form_fire_group(form, group, speed) { \
	Pellets.add_group( \
		(form_center_x(form) - (PELLET_W / 2)), \
		(form_center_y(form) - (PELLET_H / 2)), \
		group, \
		to_sp(speed) \
	); \
}
// -------------------------


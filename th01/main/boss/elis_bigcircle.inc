static const pixel_t BIGCIRCLE_RADIUS = ((GIRL_W * 2) / 2);

#define bigcircle_sloppy_unput(bigcircle) { \
	shape_circle_sloppy_unput( \
		bigcircle.center_x(), bigcircle.center_y(), BIGCIRCLE_RADIUS, 0x01 \
	); \
}

#define bigcircle_put_arc(bigcircle, angle_start, angle_end, col) { \
	shape_ellipse_arc_put( \
		bigcircle.center_x(), \
		bigcircle.center_y(), \
		BIGCIRCLE_RADIUS, \
		BIGCIRCLE_RADIUS, \
		col, \
		0x01, \
		angle_start, \
		angle_end \
	); \
}

#define bigcircle_put(bigcircle, col) { \
	bigcircle_put_arc(bigcircle, 0x00, 0xFF, col); \
}

#define bigcircle_put_dot(bigcircle, offset_start, offset_end, col) { \
	bigcircle_put_arc( \
		bigcircle, \
		(bigcircle.angle + offset_start), \
		(bigcircle.angle + offset_end), \
		col \
	); \
}

#define bigcircle_is_summon_frame(start_frame) ( \
	(boss_phase_frame >= start_frame) && ((boss_phase_frame % 2) == 0) \
)

#define bigcircle_summon_done(bigcircle) \
	(bigcircle.angle >= (0x100 / 4))

#define bigcircle_summon_update_and_render(bigcircle, q4_offset_end) { \
	bigcircle_put_dot(bigcircle, 0x00, q4_offset_end, V_WHITE); \
	bigcircle_put_dot(bigcircle, 0x40, 0x42, V_WHITE); \
	bigcircle_put_dot(bigcircle, 0x80, 0x82, V_WHITE); \
	bigcircle_put_dot(bigcircle, 0xC0, 0xC2, V_WHITE); \
	\
	bigcircle.angle += 0x02; \
}

// All of the summon animation macros below are forced to be used inside an
// `if` statement, returning a non-zero value when the flash animation is done.
// MODDERS: Just turn them into regular functions.

// Renders a frame of the summon animation.
#define bigcircle_summon(bigcircle, start_frame, q4_offset_end) \
	boss_phase_frame == start_frame) { \
		mdrv2_se_play(8); \
	} \
	bigcircle_summon_update_and_render(bigcircle, q4_offset_end); \
	if(bigcircle_summon_done(bigcircle) /* return value */

// Renders a frame of the summon and flash animation.
#define bigcircle_summon_and_flash(bigcircle, start_frame, q4_offset_end) \
	bigcircle_is_summon_frame(start_frame) && (bigcircle.frame == 0)) { \
		if(bigcircle_summon(bigcircle, start_frame, q4_offset_end)) { \
			bigcircle_sloppy_unput(bigcircle);	/* ZUN bloat */ \
			\
			/* Also looks redundant, but we might have unblitted parts of */ \
			/* the circle during the summon animation… */ \
			bigcircle_put(bigcircle, V_WHITE); \
			\
			bigcircle.frame = 1; \
			ent_unput_and_put_both(ent_still_or_wave, 1, C_STILL); \
		} \
	} else if((bigcircle.frame != 0) && (bigcircle.frame < 40)) { \
		bigcircle.frame++; \
		if((bigcircle.frame % 8) == 0) { \
			bigcircle_sloppy_unput(bigcircle);	/* ZUN bloat */ \
			bigcircle_put(bigcircle, COL_FX); \
		} else if((bigcircle.frame % 8) == 4) { \
			bigcircle_sloppy_unput(bigcircle);	/* ZUN bloat */ \
			bigcircle_put(bigcircle, V_WHITE); \
		} \
	} else if(bigcircle.frame != 0 /* return value */

// Circle around the Star of David
struct StarCircle {
	unsigned char angle;
	int frame;

	screen_x_t center_x(void) { return form_center_x(F_GIRL); }
	screen_y_t center_y(void) { return form_center_y(F_GIRL); }
};
// -----------------


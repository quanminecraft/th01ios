static const int SNAKE_TRAIL_COUNT = 5;

inline screen_x_t snake_target_offset_left(const screen_x_t &to_left) {
	return (to_left + (PLAYER_W / 2) - (DIAMOND_W / 2));
}

#define SNAKE_HOMING_THRESHOLD ( \
	PLAYFIELD_TOP + playfield_fraction_y(5 / 7.0f) - (DIAMOND_H / 2) \
)

template <int SnakeCount> struct Snakes {
	screen_x_t left[SnakeCount][SNAKE_TRAIL_COUNT];
	screen_y_t top[SnakeCount][SNAKE_TRAIL_COUNT];
	pixel_t velocity_x[SnakeCount];
	pixel_t velocity_y[SnakeCount];
	bool16 target_locked[SnakeCount];
	screen_x_t target_left[SnakeCount];

	int count() const {
		return SnakeCount;
	}
};

#define snakes_wobbly_aim(snakes, snake_i, to_left, speed, tmp_angle) \
	tmp_angle = iatan2( \
		(player_center_y() - snakes.top[snake_i][0]), \
		(snake_target_offset_left(to_left) - snakes.left[snake_i][0]) \
	); \
	tmp_angle += ((irand() % 2) == 0) ? +0x28 : -0x28; \
	vector2( \
		(pixel_t far &)snakes.velocity_x[snake_i], \
		(pixel_t far &)snakes.velocity_y[snake_i], \
		speed, \
		tmp_angle \
	);

#define snakes_spawn_and_wobbly_aim( \
	snakes, snake_i, origin_x, origin_y, tmp_i, tmp_angle \
) \
	for(tmp_i = 0; tmp_i < SNAKE_TRAIL_COUNT; tmp_i++) { \
		snakes.left[snake_i][tmp_i] = origin_x; \
		snakes.top[snake_i][tmp_i] = origin_y; \
	} \
	snakes_wobbly_aim(snakes, snake_i, player_left, 6, tmp_angle)

#define snakes_unput_update_render(tmp_i, tmp_j, tmp_angle) \
	for(tmp_i = 0; tmp_i < snakes.count(); tmp_i++) { \
		/* Snake update */ \
		if(snakes.left[tmp_i][0] == -PIXEL_NONE) { \
			continue; \
		} \
		/* The last trail sprite is the only one we have to unblit here. */ \
		/* Since we forward-copy the coordinates for the remaining trail */ \
		/* segments, they're then drawn on top of previously blitted */ \
		/* sprites anyway. Nifty! */ \
		shape8x8_sloppy_unput( \
			snakes.left[tmp_i][SNAKE_TRAIL_COUNT - 1], \
			snakes.top[tmp_i][SNAKE_TRAIL_COUNT - 1] \
		); \
		\
		/* Render…? Before update? */ \
		for(tmp_j = (SNAKE_TRAIL_COUNT - 2); tmp_j >= 1; tmp_j--) { \
			shape8x8_diamond_put( \
				snakes.left[tmp_i][tmp_j], snakes.top[tmp_i][tmp_j], 9 \
			); \
		} \
		shape8x8_diamond_put( \
			snakes.left[tmp_i][0], snakes.top[tmp_i][0], V_WHITE \
		); \
		\
		/* Update */ \
		if(snakes.target_locked[tmp_i] == false) { \
			snakes.target_left[tmp_i] = player_left; \
		} \
		snakes_wobbly_aim(snakes, i, snakes.target_left[tmp_i], 7, angle); \
		if(snakes.top[tmp_i][0] > SNAKE_HOMING_THRESHOLD) { \
			snakes.target_locked[tmp_i] = true; \
		} \
		\
		/* Forward copy */ \
		for(tmp_j = (SNAKE_TRAIL_COUNT - 1); tmp_j >= 1; tmp_j--) { \
			snakes.left[tmp_i][tmp_j] = snakes.left[tmp_i][tmp_j - 1]; \
			snakes.top[tmp_i][tmp_j] = snakes.top[tmp_i][tmp_j - 1]; \
		} \
		\
		snakes.left[tmp_i][0] += snakes.velocity_x[tmp_i]; \
		snakes.top[tmp_i][0] += snakes.velocity_y[tmp_i]; \
		\
		/* Yes, that's a 30×30 hitbox around the player's center point if */ \
		/* the player is not sliding, only leaving out the edges. */ \
		if(overlap_xy_ltrb_lt_gt( \
			snakes.left[tmp_i][0], \
			snakes.top[tmp_i][0], \
			player_left, \
			(player_top + (player_sliding * 10)), \
			(player_right() - DIAMOND_W), \
			(player_bottom() - DIAMOND_H) \
		)) { \
			if(!player_invincible) { \
				player_is_hit = true; \
			} \
		} \
	}

#define snakes_unput_all(snakes, tmp_i, tmp_j) \
	for(tmp_i = 0; tmp_i < snakes.count(); tmp_i++) { \
		for(tmp_j = 0; tmp_j < SNAKE_TRAIL_COUNT; tmp_j++) { \
			shape8x8_sloppy_unput( \
				snakes.left[tmp_i][tmp_j], snakes.top[tmp_i][tmp_j] \
			); \
		} \
	}
// ------


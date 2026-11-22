enum bird_pellet_group_t {
	BPG_AIMED = 0,
	BPG_6_RING = 1,
	BPG_RANDOM_RAIN = 2,
};

static const int BF_UNPUT_UPDATE_RENDER = 0;
static const int BF_RESET = 1000;
static const int BF_FIRE = 1001;

struct CBirds {
	double left[BIRD_COUNT];
	double top[BIRD_COUNT];
	double velocity_x[BIRD_COUNT];
	double velocity_y[BIRD_COUNT];
	char hatch_time[BIRD_COUNT];
	int8_t hatch_frames[BIRD_COUNT];
	char alive[BIRD_COUNT];

	void reset(void);
	void fire(bird_pellet_group_t group);
	void spawn(double left, double top, double velocity_x, double velocity_y);
	void unput_update_render(void);
};

void CBirds::reset(void)
{
	for(int i = 0; i < BIRD_COUNT; i++) {
		alive[i] = false;
	}
}

void CBirds::fire(bird_pellet_group_t group)
{
	for(int i = 0; i < BIRD_COUNT; i++) {
		if(!alive[i] || hatch_time[i]) {
			continue;
		}
		const screen_x_t pellet_left = (
			left[i] + ((BIRD_W / 2) - (PELLET_W / 2))
		);
		const screen_y_t pellet_top = (
			top[i] + ((BIRD_H / 2) - (PELLET_H / 2))
		);
		switch(group) {
		case BPG_AIMED:
			Pellets.add_group(pellet_left, pellet_top, PG_1_AIMED, to_sp(3.0f));
			break;

		case BPG_6_RING:
			{for(int j = 0; j < 6; j++) {
				Pellets.add_single(
					pellet_left, pellet_top, (j * (0x100 / 6)), to_sp(4.0f)
				);
			}}
			break;

		case BPG_RANDOM_RAIN:
			pellets_add_single_rain(
				pellet_left, pellet_top, ((irand() & 0x7F) + 0x80), 4.0f
			);
			break;
		}
	}
}

void CBirds::spawn(
	double left_, double top_, double velocity_x_, double velocity_y_
)
{
	for(int i = 0; i < BIRD_COUNT; i++) {
		if(!alive[i]) {
			alive[i] = true;
			velocity_x[i] = velocity_x_;
			velocity_y[i] = velocity_y_;
			hatch_time[i] = 25;
			left[i] = left_;
			top[i] = top_;
			hatch_frames[i] = hatch_time[i];
			return;
		}
	}
}

void CBirds::unput_update_render(void)
{
	{for(int i = 0; i < BIRD_COUNT; i++) {
		if(!alive[i]) {
			continue;
		}
		if(hatch_time[i] > 0) {
			bird_put_8(left[i], top[i], ((
				(hatch_frames[i] - hatch_time[i]) /
				(hatch_frames[i] / BIRD_HATCH_CELS)
			) + C_HATCH));
			hatch_time[i]--;
		} else {
			// ZUN bug: Shouldn't these be unblitted unconditionally?
			// Because they aren't, each cel of the hatch animation is
			// blitted on top of the previous one...
			grc_sloppy_unput(left[i], top[i]);
		}
	}}
	{for(int i = 0; i < BIRD_COUNT; i++) {
		if(!alive[i] || hatch_time[i]) {
			continue;
		}
		left[i] += velocity_x[i];
		top[i] += velocity_y[i];
		if(!overlap_xy_lrtb_le_ge(
			left[i], top[i], 0, 0, (RES_X - 1), (RES_Y - 1)
		)) {
			grc_sloppy_unput(left[i], top[i]);
			alive[i] = false;
			continue;
		}
		bird_put_8(left[i], top[i], (
			C_FLY + ((boss_phase_frame / 5) % BIRD_FLY_CELS)
		));
		// 14×38 pixels… still unfair given the shape of such a bird.
		if(
			!player_invincible &&
			(left[i] > (player_left - (PLAYER_W / 4))) &&
			(left[i] < (player_left + (PLAYER_W / 4))) &&
			(top[i] > (player_top - (PLAYER_H / 2) - (BIRD_H / 4))) &&
			(top[i] < (player_top + (PLAYER_H / 2)))
		) {
			player_is_hit = true;
			delay(100); // ???
			return;
		}
	}}
}

CBirds Birds;

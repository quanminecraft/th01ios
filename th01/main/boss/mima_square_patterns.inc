static const int SQUARE_POINTS = 4;
static const int SQUARE_INTERVAL = 8;
static const pixel_t SQUARE_RADIUS_PER_FRAME = 1;
static const pixel_t SQUARE_RADIUS_STEP = (
	SQUARE_INTERVAL * SQUARE_RADIUS_PER_FRAME
);

typedef uint8_t square_flag_t;

static const square_flag_t SF_COUNT = 2;
static const square_flag_t SF_NONE = (0 << 0);
static const square_flag_t SF_REGULAR = (1 << 0);
static const square_flag_t SF_FLIPPED = (1 << 1);

struct CSquares {
	unsigned char angle;
	square_flag_t flag;
	pixel_t radius;
	screen_point_t center;
	union {
		struct {
			screen_point_t regular[SQUARE_POINTS];
			screen_point_t flipped[SQUARE_POINTS];
		} c;
		screen_point_t array[SF_COUNT][SQUARE_POINTS];
	} corners;

private:
	void set_corners(screen_point_t* corners, unsigned char angle);
	void set(void);

public:
	void init(square_flag_t flag_) {
		flag = flag_;
		radius = static_cast<pixel_t>(SEAL_RADIUS * 0.4f);
		angle = 0x00;
	}

	void deactivate(void) {
		set_and_unput();
		flag = SF_NONE;
	}

	void set_and_unput(void);
	void set_and_put(void);
};

void CSquares::set_corners(screen_point_t* corners, unsigned char angle)
{
	for(int i = 0; i < SQUARE_POINTS; i++) {
		corners->x = polar_x(center.x, radius, angle);
		corners->y = polar_y(center.y, radius, angle);
		angle += (0x100 / SQUARE_POINTS);
		corners++;
	}
}

void CSquares::set(void)
{
	center.x = ent_still.cur_center_x();
	center.y = ent_still.cur_center_y();
	if(flag & SF_REGULAR) {
		set_corners(corners.c.regular, angle);
	}
	if(flag & SF_FLIPPED) {
		set_corners(corners.c.flipped, (0x00 - angle));
	}
}

void CSquares::set_and_unput(void)
{
	set();
	square_flag_t flag = this->flag;
	for(int i = 0; i < SF_COUNT; i++) {
		if(flag & 1) {
			graph_r_lineloop_unput(corners.array[i], SQUARE_POINTS);
		}
		flag >>= 1;
	}
}

void CSquares::set_and_put(void)
{
	set();
	square_flag_t flag = this->flag;
	for(int i = 0; i < SF_COUNT; i++) {
		if(flag & 1) {
			graph_r_lineloop_put(corners.array[i], SQUARE_POINTS, V_WHITE);
		}
		flag >>= 1;
	}
}

CSquares sq;
// ---------------

#define fire_static_from_corner(angle, sq, corner_x, corner_y, speed) { \
	angle = iatan2((corner_y - sq.center.y), (corner_x - sq.center.x)); \
	Pellets.add_single(corner_x, corner_y, angle, speed); \
}

void pattern_aimed_then_static_pellets_from_square_corners(void)
{
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		sq.init(SF_REGULAR);
		select_subpixel_for_rank(pattern_state.speed, 4.0f, 4.5f, 5.0f, 5.5f);
		mdrv2_se_play(8);
	}
	if((boss_phase_frame % SQUARE_INTERVAL) == 0) {
		sq.set_and_unput();
		sq.angle += ((boss_phase_frame < 260) ? +0x0C : -0x0C);
		if(sq.radius < SEAL_CIRCUMSQUARE_RADIUS) {
			sq.radius += SQUARE_RADIUS_STEP;
		} else if(boss_phase_frame > 280) {
			// Recurring quirk with all of these patterns: They spawn their
			// bullets at the *previous* corner positions, i.e., the ones
			// calculated for the unblitting call, right before blitting the
			// square at the new position.

			for(int i = 0; i < SQUARE_POINTS; i++) {
				unsigned char angle;
				fire_static_from_corner(
					angle,
					sq,
					sq.corners.c.regular[i].x,
					sq.corners.c.regular[i].y,
					pattern_state.speed
				);
				mdrv2_se_play(7);
			}
		} else {
			for(int i = 0; i < SQUARE_POINTS; i++) {
				screen_x_t target_x = (player_center_x() - (PELLET_W / 2));
				screen_x_t target_y = player_center_y();
				unsigned char angle = iatan2(
					(target_y - sq.corners.c.regular[i].y),
					(target_x - sq.corners.c.regular[i].x)
				);
				Pellets.add_single(
					sq.corners.c.regular[i].x,
					sq.corners.c.regular[i].y,
					angle,
					(pattern_state.speed / 2)
				);
				mdrv2_se_play(7);
			}
		}
		sq.set_and_put();
	}
	if(boss_phase_frame > 360) {
		sq.deactivate();
		boss_phase_frame = 0;
	}
}

void pattern_aimed_missiles_from_square_corners(void)
{
	int i;
	Subpixel velocity_x;
	Subpixel velocity_y;
	static screen_x_t target_left;

	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		sq.init(SF_REGULAR);
		select_subpixel_for_rank(pattern_state.speed, 6.0f, 6.5f, 7.0f, 7.375f);
		mdrv2_se_play(8);
	}
	if((boss_phase_frame % SQUARE_INTERVAL) == 0) {
		sq.set_and_unput();
		sq.angle -= 0x0C;
		if(sq.radius < SEAL_RADIUS) {
			sq.radius += SQUARE_RADIUS_STEP;
		} else if(boss_phase_frame == 224) {
			target_left = player_left;
		} else if(boss_phase_frame > 240) {
			// Same corner coordinate quirk as seen in the first pattern.

			vector2_between(
				sq.center.x,
				sq.center.y,
				(target_left + (PLAYER_W / 2) - (MISSILE_W / 2)),
				player_center_y(),
				velocity_x.v,
				velocity_y.v,
				pattern_state.speed
			);
			for(i = 0; i < SQUARE_POINTS; i++) {
				Missiles.add(
					sq.corners.c.regular[i].x,
					sq.corners.c.regular[i].y,
					velocity_x.to_pixel(),
					velocity_y.to_pixel()
				);
			}
			mdrv2_se_play(6);
		}
		sq.set_and_put();
	}
	if(boss_phase_frame > 320) {
		sq.deactivate();
		boss_phase_frame = 0;
	}
}

void pattern_static_pellets_from_corners_of_two_squares(void)
{
	if(boss_phase_frame == 50) {
		mima_put_cast_both();
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		sq.init(SF_REGULAR | SF_FLIPPED);
		select_subpixel_for_rank(pattern_state.speed, 4.0f, 4.5f, 5.0f, 5.5f);
		mdrv2_se_play(8);
	}
	if((boss_phase_frame % SQUARE_INTERVAL) == 0) {
		sq.set_and_unput();
		sq.angle -= 0x06;
		Pellets.spawn_with_cloud = true;

		if(sq.radius < SEAL_CIRCUMSQUARE_RADIUS) {
			sq.radius += SQUARE_RADIUS_STEP;
		} else {
			// Same corner coordinate quirk as seen in the first pattern.

			for(int i = 0; i < SQUARE_POINTS; i++) {
				unsigned char angle;
				fire_static_from_corner(
					angle,
					sq,
					sq.corners.c.regular[i].x,
					sq.corners.c.regular[i].y,
					pattern_state.speed
				);
				fire_static_from_corner(
					angle,
					sq,
					sq.corners.c.flipped[i].x,
					sq.corners.c.flipped[i].y,
					pattern_state.speed
				);
				mdrv2_se_play(7);
			}
		}
		sq.set_and_put();
		Pellets.spawn_with_cloud = false;
	}
	if(boss_phase_frame > 320) {
		sq.deactivate();
		boss_phase_frame = 0;
		meteor_activate();
	}
}


static const int PENTAGRAM_INTERVAL = 4;
static const int PENTAGRAM_POINTS = 5;
static struct {
	// Corners, arranged in counterclockwise order.
	screen_x_t x[PENTAGRAM_POINTS];
	screen_y_t y[PENTAGRAM_POINTS];

	pixel_t radius;
	screen_point_t center;

	// This reversed order ruins a potential PUSH optimization...
	pixel_t velocity_y;
	pixel_t velocity_x;

	void unput(void) {
		linebullet_unput(x[0], y[0], x[2], y[2]);
		linebullet_unput(x[2], y[2], x[4], y[4]);
		linebullet_unput(x[4], y[4], x[1], y[1]);
		linebullet_unput(x[1], y[1], x[3], y[3]);
		linebullet_unput(x[3], y[3], x[0], y[0]);
	}

	void put_and_hittest(void) {
		linebullet_put_and_hittest(x[0], y[0], x[2], y[2], V_WHITE);
		linebullet_put_and_hittest(x[2], y[2], x[4], y[4], V_WHITE);
		linebullet_put_and_hittest(x[4], y[4], x[1], y[1], V_WHITE);
		linebullet_put_and_hittest(x[1], y[1], x[3], y[3], V_WHITE);
		linebullet_put_and_hittest(x[3], y[3], x[0], y[0], V_WHITE);
	}

	void aim(const screen_x_t& target_x, screen_y_t target_y) {
		vector2_between(
			center.x, inhibit_Z3(center.y),
			target_x, target_y,
			velocity_x, velocity_y,
			4
		);
	}

	void move(bool dumb_workaround_for_different_call_positions = true) {
		if(dumb_workaround_for_different_call_positions) {
			center.y += velocity_y;
			center.x += velocity_x;
		}
	}
} pentagram;

// Initial position of the pentagram, with corners in eyes
// -------------------------------------------------------

inline void pentagram_between_eyes_put(
	const CEyeEntity& eye_1, const CEyeEntity& eye_2
) {
	linebullet_put_and_hittest(
		eye_1.offcenter_x(), eye_1.iris_top(),
		eye_2.offcenter_x(), eye_2.iris_top(), V_WHITE
	);
}

inline void pentagram_between_eyes_put(void) {
	pentagram_between_eyes_put(eye_north, eye_southwest);
	pentagram_between_eyes_put(eye_southwest, eye_east);
	pentagram_between_eyes_put(eye_east, eye_west);
	pentagram_between_eyes_put(eye_west, eye_southeast);
	pentagram_between_eyes_put(eye_southeast, eye_north);
}
// -------------------------------------------------------

// Shrink animation
// ----------------

// No weird fraction for once!
static const screen_y_t PENTAGRAM_SHRINK_TARGET_CENTER_Y = PLAYFIELD_CENTER_Y;

static const pixel_t PENTAGRAM_RADIUS_FINAL = 64;
static const unsigned char PENTAGRAM_ANGLE_INITIAL = 0xC0; // (-0x40)

// As the largest distance, we can define all other shrink distances in terms
// of this one. Technically, the radius factor should be
//
// 	cos(PENTAGRAM_ANGLE_INITIAL + (1 × (360° / PENTAGRAM_POINTS)))
//
// for the 1st clockwise point on the pentagram. But since that comes out to
// roughly ≈ 0.95 and thus *almost* 1, it's acceptable to just round up.
static const pixel_t PENTAGRAM_SHRINK_DISTANCE = (
	EYE_LATERAL_CENTER_DISTANCE_X - (PENTAGRAM_RADIUS_FINAL * 1)
);

inline screen_x_t pentagram_shrink_x(eye_flag_t eye, const pixel_t& distance) {
	enum {
		DISTANCE_SOUTH = static_cast<pixel_t>(
			// cos(PENTAGRAM_ANGLE_INITIAL + (2 × (360° / PENTAGRAM_POINTS)))
			EYE_SOUTH_CENTER_DISTANCE_X - (PENTAGRAM_RADIUS_FINAL * 0.587f)
		),
		// Integer arithmetic can never be precise for the distances required
		// here. All these calculations are at least an attempt of deriving
		// those magic shrink factors, even if we have to cheat by "rounding
		// up" in the end.
		DIVISOR_SOUTH = ((PENTAGRAM_SHRINK_DISTANCE / DISTANCE_SOUTH) + 1),
	};

	if(eye == EF_WEST) {
		return (     eye_west.offcenter_x() + distance);
	} else if(eye == EF_EAST) {
		return (     eye_east.offcenter_x() - distance);
	} else if(eye == EF_SOUTHWEST) {
		return (eye_southwest.offcenter_x() + (distance / DIVISOR_SOUTH));
	} else if(eye == EF_SOUTHEAST) {
		return (eye_southeast.offcenter_x() - (distance / DIVISOR_SOUTH));
	}
	return eye_north.offcenter_x();
}

inline screen_y_t pentagram_shrink_y(eye_flag_t eye, const pixel_t& distance) {
	enum {
		TARGET_CENTER_Y = PENTAGRAM_SHRINK_TARGET_CENTER_Y,

		TARGET_NORTH = static_cast<pixel_t>(
			// sin(PENTAGRAM_ANGLE_INITIAL + (1 × (360° / PENTAGRAM_POINTS)))
			TARGET_CENTER_Y - (PENTAGRAM_RADIUS_FINAL * 1)
		),
		TARGET_LATERAL = static_cast<pixel_t>(
			// sin(PENTAGRAM_ANGLE_INITIAL + (2 × (360° / PENTAGRAM_POINTS)))
			TARGET_CENTER_Y + (PENTAGRAM_RADIUS_FINAL * -0.309f)
		),
		TARGET_SOUTH = static_cast<pixel_t>(
			// sin(PENTAGRAM_ANGLE_INITIAL + (3 × (360° / PENTAGRAM_POINTS)))
			TARGET_CENTER_Y + (PENTAGRAM_RADIUS_FINAL * +0.809f)
		),

		DISTANCE_NORTH = (TARGET_NORTH - (EYE_NORTH_TOP + (EYE_H / 2))),
		DISTANCE_LATERAL = (TARGET_LATERAL - (EYE_LATERAL_TOP + (EYE_H / 2))),
		DISTANCE_SOUTH = (TARGET_SOUTH - (EYE_LATERAL_TOP + (EYE_H / 2))),

		// Integer arithmetic can never be precise for the distances required
		// here. All these calculations are at least an attempt of deriving
		// those magic shrink factors, even if we have to cheat by "rounding
		// up" in the end.
		DIVISOR_NORTH = (PENTAGRAM_SHRINK_DISTANCE / DISTANCE_NORTH),
		DIVISOR_LATERAL = ((PENTAGRAM_SHRINK_DISTANCE / DISTANCE_LATERAL) + 1),
		DIVISOR_SOUTH = ((PENTAGRAM_SHRINK_DISTANCE / DISTANCE_SOUTH) + 1),
	};

	if(eye == EF_WEST) {
		return (eye_west.iris_top() + (distance / DIVISOR_LATERAL));
	} else if(eye == EF_EAST) {
		return (eye_east.iris_top() + (distance / DIVISOR_LATERAL));
	} else if(eye == EF_SOUTHWEST) {
		return (eye_southwest.iris_top() + (distance / DIVISOR_SOUTH));
	} else if(eye == EF_SOUTHEAST) {
		return (eye_southeast.iris_top() + (distance / DIVISOR_SOUTH));
	}
	return (eye_north.iris_top() + (distance / DIVISOR_NORTH));
}

void pentagram_shrink_unput(screen_point_t p1, screen_point_t p2)
{
	linebullet_unput(p1.x, p1.y, p2.x, p2.y);
}

void pentagram_shrink_put(screen_point_t p1, screen_point_t p2)
{
	linebullet_put_and_hittest(p1.x, p1.y, p2.x, p2.y, V_WHITE);
}

void pentagram_shrink(
	void (*func)(screen_point_t, screen_point_t), pixel_t distance
)
{
	screen_point_t p[EYE_COUNT];
	p[0].x = pentagram_shrink_x(EF_NORTH, distance);
	p[0].y = pentagram_shrink_y(EF_NORTH, distance);
	p[1].x = pentagram_shrink_x(EF_SOUTHWEST, distance);
	p[1].y = pentagram_shrink_y(EF_SOUTHWEST, distance);
	p[2].x = pentagram_shrink_x(EF_EAST, distance);
	p[2].y = pentagram_shrink_y(EF_EAST, distance);
	p[3].x = pentagram_shrink_x(EF_WEST, distance);
	p[3].y = pentagram_shrink_y(EF_WEST, distance);
	p[4].x = pentagram_shrink_x(EF_SOUTHEAST, distance);
	p[4].y = pentagram_shrink_y(EF_SOUTHEAST, distance);

	int i;
	for(i = 0; i < (EYE_COUNT - 1); i++) {
		func(p[i], p[i + 1]);
	}
	func(p[i], p[0]);
}
// ----------------

// Regular pentagram, with spin and slam phases
// --------------------------------------------

void pentagram_corners_set_regular(unsigned char angle_offset)
{
	for(int i = 0; i < PENTAGRAM_POINTS; i++) {
		pentagram.x[i] = polar_x(
			pentagram.center.x, pentagram.radius, angle_offset
		);
		pentagram.y[i] = polar_y(
			pentagram.center.y, pentagram.radius, angle_offset
		);
		angle_offset += (0x100 / PENTAGRAM_POINTS);
	}
}

// Not "subphases", as these run independent of [boss_phase] – that one does
// in fact loop back at the end of the *_SLAM_INTO_PLAYER_* phases.
enum pentagram_attack_phase_t {
	PAP_PREPARE_1 = 0,
	PAP_SPIN_CLOCKWISE_1 = 1,
	PAP_SPIN_COUNTERCLOCKWISE_1 = 2,
	PAP_SPIN_CLOCKWISE_2 = 3,
	PAP_SPIN_COUNTERCLOCKWISE_2 = 4,
	PAP_SPIN_CLOCKWISE_3 = 5,
	PAP_SLAM_INTO_PLAYER_1 = 6,
	PAP_PREPARE_2 = 7,
	PAP_GROW = 8,
	PAP_SLAM_INTO_PLAYER_2 = 9,

	_pentagram_attack_phase_t_FORCE_INT16 = 0x7FFF
};

#define pentagram_in_slam_phase(phase) ( \
	(phase == PAP_SLAM_INTO_PLAYER_1) || (phase == PAP_SLAM_INTO_PLAYER_2) \
)

#define pentagram_in_clockwise_spin_phase(phase) ( \
	(phase == PAP_SPIN_CLOCKWISE_1) || \
	(phase == PAP_SPIN_CLOCKWISE_2) || \
	(phase == PAP_SPIN_CLOCKWISE_3) \
)

#define pentagram_in_counterclockwise_spin_phase(phase) ( \
	(phase == PAP_SPIN_COUNTERCLOCKWISE_1) || \
	(phase == PAP_SPIN_COUNTERCLOCKWISE_2) \
)

// Also performs collision detection.
void near pentagram_regular_unput_update_render(unsigned char angle_offset)
{
	pentagram.unput();
	pentagram_corners_set_regular(angle_offset);
	pentagram.put_and_hittest();
}

// MODDERS: Merge into the other pentagram structure.
struct Pentagram {
	pentagram_attack_phase_t phase;
	yuugenmagan_int_t angle; // ACTUAL TYPE: unsigned char

	void spin(clock_direction_t dir) {
		if(dir == CLOCKWISE) {
			angle += ((boss_phase_frame / 64) + 0x02);
		} else {
			angle -= ((boss_phase_frame / 64) + 0x02);
		}
	}

	void unput_update_render_regular(void) const {
		pentagram_regular_unput_update_render(angle);
	}
};

// ZUN bug: The pentagram is not rendered in the preparation phases, and can
// thus easily be unblitted by overlapping sprites, most notably player shots
// or HARRY UP pellets.
#define pentagram_prepare_update(pentagram_, phase_id) { \
	if( \
		(boss_phase_frame == SUBPHASE_PREPARE_FRAMES) && \
		(pentagram_.phase == phase_id) \
	) { \
		pentagram_.phase = static_cast<pentagram_attack_phase_t>( \
			phase_id + 1 \
		); \
		boss_phase_frame = 0; \
	} \
}

// The next two are forced to be used inside an `if` statement, returning a
// non-zero value if the phase is done.
#define pentagram_spin_unput_update_render(pentagram_, dir) \
	(boss_phase_frame % PENTAGRAM_INTERVAL) == 0) { \
		pentagram_.spin(dir); \
		pentagram_.unput_update_render_regular(); \
	} \
	if(boss_phase_frame >= SUBPHASE_TIMEOUT_FRAMES


#define pentagram_slam_unput_update_render( \
	pentagram_, homing_threshold_from_bottom, move_first \
) \
	(boss_phase_frame % PENTAGRAM_INTERVAL) == 0) { \
		if(pentagram.center.y < ( \
			PLAYFIELD_BOTTOM - (homing_threshold_from_bottom) \
		)) { \
			pentagram.aim(player_left, player_bottom()); \
		} \
		pentagram.move(move_first); \
		pentagram_.spin(CLOCKWISE); \
		pentagram.move(!move_first); \
		pentagram_.unput_update_render_regular(); \
	} \
	if(pentagram.center.y >= (PLAYFIELD_BOTTOM + ((PLAYFIELD_H / 84) * 5))
// --------------------------------------------
/// ---------


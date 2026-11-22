static const pixel_t EYE_W = 64;
static const pixel_t EYE_H = 48;
static const pixel_t PUPIL_H = 12;
static const pixel_t IRIS_H = 7;

static const pixel_t EYE_LATERAL_CENTER_DISTANCE_X = 224;
static const pixel_t EYE_SOUTH_CENTER_DISTANCE_X = 96;
static const pixel_t EYE_NORTH_LATERAL_DISTANCE_Y = 64;
static const pixel_t EYE_LATERAL_SOUTH_DISTANCE_Y = 48;

// Center of the kimono figure
static const screen_y_t PENTAGRAM_REGULAR_CENTER_Y = 189;

static const screen_x_t EYE_WEST_LEFT = (
	PLAYFIELD_CENTER_X - EYE_LATERAL_CENTER_DISTANCE_X - (EYE_W / 2)
);

static const screen_x_t EYE_EAST_LEFT = (
	PLAYFIELD_CENTER_X + EYE_LATERAL_CENTER_DISTANCE_X - (EYE_W / 2)
);

static const screen_x_t EYE_SOUTHWEST_LEFT = (
	PLAYFIELD_CENTER_X - EYE_SOUTH_CENTER_DISTANCE_X - (EYE_W / 2)
);

static const screen_x_t EYE_SOUTHEAST_LEFT = (
	PLAYFIELD_CENTER_X + EYE_SOUTH_CENTER_DISTANCE_X - (EYE_W / 2)
);

static const screen_x_t EYE_NORTH_LEFT = (PLAYFIELD_CENTER_X - (EYE_W / 2));

static const screen_y_t EYE_NORTH_TOP = PLAYFIELD_TOP;
static const screen_y_t EYE_LATERAL_TOP = (
	EYE_NORTH_TOP + EYE_NORTH_LATERAL_DISTANCE_Y
);
static const screen_y_t EYE_SOUTH_TOP = (
	EYE_LATERAL_TOP + EYE_LATERAL_SOUTH_DISTANCE_Y
);
// -----------

enum yuugenmagan_colors_t {
	COL_YOKOSHIMA = 15, // The big 邪 in the background
};

// Always denotes the last phase that ends with that amount of HP.
enum yuugenmagan_hp_t {
	HP_TOTAL = 16,
	HP_PHASE_1_END = 15,
	HP_PHASE_3_END = 12,
	HP_PHASE_5_END = 10,
	HP_PHASE_7_END = 8,
	HP_PHASE_9_END = 2,
	HP_PHASE_13_END = 0,
};

// Entities
// --------

static const int EYE_COUNT = 5;

enum eye_cel_t {
	C_HIDDEN = 0,
	C_CLOSED = 1,
	C_HALFOPEN = 2,
	C_DOWN = 3,
	C_LEFT = 4,
	C_RIGHT = 5,
	C_AHEAD = 6,
};

// Eye flags
typedef int8_t eye_flag_t;

// Code generation wants 16-bit constants, unfortunately
static const int EF_NONE = 0;
static const int EF_WEST = (1 << 0);
static const int EF_EAST = (1 << 1);
static const int EF_SOUTHWEST = (1 << 2);
static const int EF_SOUTHEAST = (1 << 3);
static const int EF_NORTH = (1 << 4);
static const int EF_RESET = (1 << 5);

struct CEyeEntity : public CBossEntitySized<EYE_W, EYE_H> {
	// Relative pupil and iris coordinates
	// -----------------------------------
	// These only apply to the player-facing cels (C_DOWN, C_LEFT, and C_RIGHT).

	// ZUN quirk: Doesn't really correspond to any precise feature of any eye
	// cel. The best match is the center of the iris on C_DOWN, but even that
	// would be off by 1 pixel – not to mention very wrong for every other cel.
	screen_x_t offcenter_x(void) const {
		return (cur_center_x() - 4);
	}

	// Correct for C_LEFT and C_RIGHT, off by 1 pixel for C_DOWN.
	screen_y_t iris_top(void) const {
		return (cur_center_y() + 4);
	}

	// Correct for C_LEFT and C_RIGHT, off by 1 pixel for C_DOWN.
	screen_y_t iris_center_y(void) const {
		return (iris_top() + (IRIS_H / 2) + 1);
	}

	// Correct for C_LEFT and C_RIGHT, off by 1 pixel for C_DOWN.
	screen_y_t pupil_bottom(void) const {
		return (cur_center_y() + PUPIL_H);
	}
	// -----------------------------------

	void track_player(void) {
		if((cur_center_x() - player_left) > (EYE_W / 2)) {
			set_image(C_LEFT);
		} else if((player_left - cur_center_x()) > (EYE_W / 2)) {
			set_image(C_RIGHT);
		} else {
			set_image(C_DOWN);
		}
	}

	void downwards_laser_put(void) const {
		graph_r_vline(
			offcenter_x(), iris_top(), (PLAYFIELD_BOTTOM - 2), V_WHITE
		);
	}

	void downwards_laser_unput(void) const {
		graph_r_line_unput(
			offcenter_x(), iris_top(), offcenter_x(), PLAYFIELD_BOTTOM
		);
	}

	void fire_from_iris_center(pellet_group_t group, float speed_base) const {
		Pellets.add_group(
			(cur_center_x()  - (PELLET_W / 2)),
			(iris_center_y() - (PELLET_H / 2)),
			group,
			to_sp(speed_base)
		);
	}

	void fire_from_bottom_center(pellet_group_t group, float speed_base) const {
		Pellets.add_group(
			(cur_center_x() - (PELLET_W / 2)),
			(pupil_bottom() - (PELLET_H / 2)),
			group,
			to_sp(speed_base)
		);
	}

	void fire_from_bottom_center(
		bool mirror_angle_horizontally,
		const unsigned char &angle,
		float speed_base,
		pellet_motion_t motion_type = PM_REGULAR,
		float speed_for_motion_fixed = 0.0f
	) const {
		Pellets.add_single(
			(cur_center_x() - (PELLET_W / 2)),
			(pupil_bottom() - (PELLET_H / 2)),
			(mirror_angle_horizontally ? (0x80 - angle) : angle),
			to_sp(speed_base),
			motion_type,
			to_sp(speed_for_motion_fixed)
		);
	}
};


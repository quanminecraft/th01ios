#pragma once
enum item_flag_t {
	IF_FREE = 0,
	IF_SPLASH = 1,
	IF_FALL = 2,
	IF_BOUNCE = 3,
	IF_COLLECTED = 99,
	IF_COLLECTED_OVER_CAP = 100,

	_item_flag_t_FORCE_INT16 = 0x7FFF
};

union item_flag_state_t {
	unsigned char splash_radius;	// During IF_SPLASH. In pixels.
	unsigned char collect_time; 	// During ≥IF_COLLECTED.
};

struct item_t {
	// Also overloaded for the text popup shown when the item is collected.
	screen_x_t left;
	screen_y_t top;
	int16_t unknown_zero;
	pixel_t velocity_y;

	char flag; // item_flag_t
	item_flag_state_t flag_state;
};

static const int ITEM_BOMB_COUNT = 4;
static const int ITEM_POINT_COUNT = 10;


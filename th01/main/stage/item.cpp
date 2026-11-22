#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/item_runtime.hpp"
#define item_point_value native_point_value
#else
#include "th01/resident.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/clamp.hpp"
#include "th01/math/digit.hpp"
#include "th01/math/str_val.hpp"
#include "th01/hardware/grppsafx.h"
#include "th01/snd/mdrv2.h"
#include "th01/main/stage/stageobj.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/hud/hud.hpp"
#include "th01/main/shape.hpp"
#include "th01/main/stage/item.hpp"
#include "th01/shiftjis/item.hpp"
#define item_point_value resident->point_value
#endif

/// Constants
/// ---------

static const pixel_t ITEM_W = PTN_W;
static const pixel_t ITEM_H = PTN_H;

static const unsigned int POINT_CAP = 65530;
static const unsigned int POINT_CAP_DIGITS = digit_count(POINT_CAP);

static const pixel_t BOMB_COLLECT_1_W = shiftjis_w(BOMB_COLLECT_1);
static const pixel_t BOMB_COLLECT_2_W = shiftjis_w(BOMB_COLLECT_2);
static const pixel_t POINT_COLLECT_W = (POINT_CAP_DIGITS * GLYPH_HALF_W);
/// ---------

/// Structures
/// ----------

#include "th01/main/stage/item_state.hpp"

item_t items_bomb[ITEM_BOMB_COUNT];
item_t items_point[ITEM_POINT_COUNT];
/// ----------

/// Function types
/// --------------

// Called every frame to check collision between the player and an item in the
// given [slot]. Should set its [flag] to IF_COLLECTED or IF_COLLECTED_OVER_CAP
// if they did collide.
typedef void hittest_func_t(int slot);

// Called while an item slot's [flag] is ≥ IF_COLLECTED. Therefore, also
// responsible to eventually set a different flag.
typedef void collect_update_and_render_func_t(int slot);

// Called a single time if an uncollected item fell below the playfield, after
// its slot is reset to IF_FREE.
typedef void drop_func_t(void);
/// --------------

/// Helper functions
/// ----------------

#define item_is_visible(item) ( \
	(item.flag > IF_SPLASH) && (item.flag < IF_COLLECTED) \
)

// Note that this hitbox is actually perfectly symmetrical around the
// player's center point, even though it might not look like it.
#define item_player_hittest(item) ( \
	(item.left > (player_left - (PLAYER_W - (ITEM_W / 4)))) && \
	(item.left < (player_left + (PLAYER_W - (ITEM_W / 4)))) && \
	(item.top  > (player_top  - (PLAYER_H - (ITEM_H / 4)))) && \
	item_is_visible(item) \
)

#define item_collect_init(item) { \
	item.velocity_y = -2; \
	item.flag_state.collect_time = 16; \
	item.top = clamp_max_2_ge(item.top, (PLAYFIELD_BOTTOM - ITEM_H)); \
}

inline screen_x_t point_collect_left(item_t* slots, const int i) {
	return clamp_max_2(slots[i].left, (PLAYFIELD_RIGHT - POINT_COLLECT_W));
}

inline screen_x_t bomb_collect_2_left(item_t* slots, const int i) {
	// Line 2 is centered relative to line 1.
	enum {
		X_OFFSET = ((BOMB_COLLECT_2_W - BOMB_COLLECT_1_W) / 2)
	};
	static_assert(sizeof(BOMB_COLLECT_1) == sizeof(BOMB_COLLECT_CAP));

	return clamp_max_2(
		clamp_min_2((slots[i].left - X_OFFSET), PLAYFIELD_LEFT),
		(PLAYFIELD_RIGHT - BOMB_COLLECT_2_W)
	);
}
/// ----------------

#include "th01/main/stage/item_spawn.inc"
void bomb_hittest(int slot)
{
	if(!item_player_hittest(items_bomb[slot])) {
		return;
	}
	ptn_sloppy_unput_16(items_bomb[slot].left, items_bomb[slot].top);

	item_collect_init(items_bomb[slot]);

	if(rem_bombs < BOMBS_MAX) {
		rem_bombs++;
		hud_bombs_put(rem_bombs - 1);
		items_bomb[slot].flag = IF_COLLECTED;
	} else {
		score += 10000;
		hud_score_and_cardcombo_render();
		items_bomb[slot].flag = IF_COLLECTED_OVER_CAP;
	}
	mdrv2_se_play(15);
}

void items_bomb_render(void)
{
	for(int i = 0; i < ITEM_BOMB_COUNT; i++) {
		if(item_is_visible(items_bomb[i])) {
			ptn_put_8(items_bomb[i].left, items_bomb[i].top, PTN_ITEM_BOMB);
		}
	}
}

void items_bomb_reset(void)
{
	items_bomb[0].flag = IF_FREE;
	items_bomb[1].flag = IF_FREE;
	items_bomb[2].flag = IF_FREE;
	items_bomb[3].flag = IF_FREE;
}

// ZUN bloat: Just pass an item_t structure?
#ifndef TH01_NATIVE_ORB_PHYSICS
#include "th01/main/stage/item_motion.inc"
#endif

void bomb_collect_update_and_render(int slot)
{
	#define item items_bomb[slot]

	static_assert(sizeof(BOMB_COLLECT_1) == sizeof(BOMB_COLLECT_CAP));
	egc_copy_rect_1_to_0_16(item.left, item.top, BOMB_COLLECT_1_W, GLYPH_H);
	if(item.flag == IF_COLLECTED) {
		egc_copy_rect_1_to_0_16(
			bomb_collect_2_left(items_bomb, slot),
			(item.top + GLYPH_H),
			BOMB_COLLECT_2_W,
			GLYPH_H
		);
	}

	item.top += item.velocity_y;
	item.flag_state.collect_time--;
	if(item.flag_state.collect_time == 0) {
		item.flag = IF_FREE;
		return;
	}
	if(item.flag == IF_COLLECTED) {
		graph_putsa_fx(item.left, item.top, V_WHITE, BOMB_COLLECT_1);
		graph_putsa_fx(
			bomb_collect_2_left(items_bomb, slot),
			(item.top + GLYPH_H),
			V_WHITE,
			BOMB_COLLECT_2
		);
	} else /* item.flag == IF_COLLECTED_OVER_CAP */ {
		graph_putsa_fx(item.left, item.top, V_WHITE, BOMB_COLLECT_CAP);
	}

	#undef item
}

void bomb_drop(void)
{
}

void items_bomb_unput_update_render(void)
{
	for(int i = 0; i < ITEM_BOMB_COUNT; i++) {
		if(items_bomb[i].flag != IF_FREE) {
			item_unput_update_render(
				i,
				items_bomb[i].flag,
				bomb_hittest,
				items_bomb[i].left,
				items_bomb[i].top,
				items_bomb[i].velocity_y,
				items_bomb[i].flag_state.splash_radius,
				PTN_ITEM_BOMB,
				bomb_collect_update_and_render,
				bomb_drop
			);
		}
	}
}


void point_hittest(int slot)
{
	if(!item_player_hittest(items_point[slot])) {
		return;
	}
	ptn_sloppy_unput_16(items_point[slot].left, items_point[slot].top);

	item_collect_init(items_point[slot]);

	// Well… that's one way to prevent a unsigned 16-bit integer overflow.
	if(item_point_value < 59999) {
		item_point_value += (item_point_value < 10000) ? 1000 : 10000;
	}
	if(item_point_value >= 60000) {
		item_point_value = POINT_CAP;
	}
	score += item_point_value;
	hud_score_and_cardcombo_render();
	items_point[slot].flag = IF_COLLECTED_OVER_CAP;

	mdrv2_se_play(15);
}

void items_point_render(void)
{
	for(int i = 0; i < ITEM_POINT_COUNT; i++) {
		if(item_is_visible(items_point[i])) {
			ptn_put_8(items_point[i].left, items_point[i].top, PTN_ITEM_POINT);
		}
	}
}

void items_point_reset(void)
{
	for(int i = 0; i < ITEM_POINT_COUNT; i++) {
		items_point[i].flag = IF_FREE;
	}
}

void point_drop(void)
{
	item_point_value = 0;
}

void point_collect_update_and_render(int slot)
{
	#define item items_point[slot]
	#define left point_collect_left(items_point, slot)
	char str[POINT_CAP_DIGITS + 1];

	egc_copy_rect_1_to_0_16_word_w(left, item.top, POINT_COLLECT_W, GLYPH_H);

	item.top += item.velocity_y;
	item.flag_state.collect_time--;
	if(item.flag_state.collect_time == 0) {
		item.flag = IF_FREE;
		return;
	}

	str_right_aligned_from_uint16(str, item_point_value, POINT_CAP_DIGITS);
	graph_putsa_fx(left, item.top, V_WHITE, str);

	#undef left
	#undef item
}

void items_point_unput_update_render(void)
{
	for(int i = 0; i < ITEM_POINT_COUNT; i++) {
		if(items_point[i].flag != IF_FREE) {
			item_unput_update_render(
				i,
				items_point[i].flag,
				point_hittest,
				items_point[i].left,
				items_point[i].top,
				items_point[i].velocity_y,
				items_point[i].flag_state.splash_radius,
				PTN_ITEM_POINT,
				point_collect_update_and_render,
				point_drop
			);
		}
	}
}

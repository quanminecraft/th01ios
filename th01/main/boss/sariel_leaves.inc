void near pattern_swaying_leaves(sariel_frame_t &frame, int spawn_interval_or_reset)
{
	enum {
		LEAF_COUNT = 30,
	};

	enum leaf_flag_t {
		LF_FREE = 0,
		LF_SPARK = 1,
		LF_SPLASH = 2,

		// Separate state because it assigns the velocity for the next one.
		LF_SPLASH_DONE = (LF_SPLASH + LEAFSPLASH_CELS),

		LF_LEAF,

		_leaf_flag_t_FORCE_INT16 = 0x7FFF,
	};

	static leaf_flag_t flag[LEAF_COUNT] = { LF_FREE };
	static DecimalSubpixel left[LEAF_COUNT];
	static DecimalSubpixel top[LEAF_COUNT];
	static DecimalSubpixel velocity_x[LEAF_COUNT];
	static DecimalSubpixel velocity_y[LEAF_COUNT];

#ifdef TH01_NATIVE_ORB_PHYSICS
	native_leaf_bind(flag, left, top, velocity_x, velocity_y);
#endif

	#define leaf_on_screen(i) \
		overlap_xy_rltb_lt_ge( \
			left[i].v, \
			top[i].v, \
			to_dsp(0.0f), \
			to_dsp(0.0f), \
			to_dsp(RES_X - LEAF_W), \
			to_dsp(RES_Y - LEAF_H) \
		)

	#define leaf_put(tmp_vo, tmp_first_bit, i, sprite) { \
		if(leaf_on_screen(i)) { \
			tmp_vo = vram_offset_divmul( \
				left[i].to_pixel(), top[i].to_pixel() \
			); \
			tmp_first_bit = (left[i].to_pixel() % BYTE_DOTS); \
			grcg_put_8x8_mono(tmp_vo, tmp_first_bit, sprite[0].row, V_WHITE); \
		} \
	}

	// Code generation needs them to be separate, unfortunately...
#ifdef TH01_NATIVE_ORB_PHYSICS
	#include "native/leaf_sprites.inc"
#else
	#include "th01/sprites/leaf_s.csp"
	#include "th01/sprites/leaf_l.csp"
	#include "th01/sprites/leaf_r.csp"
#endif

	int i;
	vram_offset_t vo;
	int first_bit;

	if(spawn_interval_or_reset == 999) {
		for(i = 0; i < LEAF_COUNT; i++) {
			flag[i] = LF_FREE;
		}
		return;
	}
	if((frame % spawn_interval_or_reset) == 0) {
		for(i = 0; i < LEAF_COUNT; i++) {
			if(flag[i] != LF_FREE) {
				continue;
			}
			left[i].v = (
				to_dsp(PLAYFIELD_LEFT) + (irand() % to_dsp(PLAYFIELD_W))
			);
			top[i].v = (
				to_dsp(FACE_CENTER_Y) +
				(irand() % to_dsp((PLAYFIELD_H * 25) / 84))
			);
			vector2_between(
				left[i].to_pixel(),
				top[i].to_pixel(),
				playfield_rand_x(),
				PLAYFIELD_TOP,
				velocity_x[i].v,
				velocity_y[i].v,
				to_dsp(4.0f)
			);
			flag[i] = LF_SPARK;
			break;
		}
	}

	if((frame % 2) != 0) {
		return;
	}

	// Unblit and update
	for(i = 0; i < LEAF_COUNT; i++) {
		if(flag[i] == LF_FREE) {
			continue;
		}
		if(flag[i] == LF_SPARK) {
			if(leaf_on_screen(i)) {
				egc_copy_rect_1_to_0_16_word_w(
					left[i].to_pixel(), top[i].to_pixel(), LEAF_W, LEAF_H
				);
			}
			left[i].v += velocity_x[i].v;
			top[i].v += velocity_y[i].v;
			if(top[i].v < to_dsp(PLAYFIELD_TOP)) {
				flag[i] = LF_SPLASH;
			}
		} else if(flag[i] <= LF_SPLASH_DONE) {
			if(leaf_on_screen(i)) {
				grc_sloppy_unput(left[i].to_pixel(), top[i].to_pixel());
			}
			if(flag[i] == LF_SPLASH_DONE) {
				flag[i] = LF_LEAF;
				velocity_y[i].v = (to_dsp(0.3f) + (irand() % to_dsp(0.2f)));
				velocity_x[i].v = to_dsp(0.2f);
			}
		} else if(flag[i] == LF_LEAF) {
			if(leaf_on_screen(i)) {
				egc_copy_rect_1_to_0_16_word_w(
					left[i].to_pixel(), top[i].to_pixel(), LEAF_W, LEAF_H
				);
			}
			left[i].v += velocity_x[i].v;
			top[i].v += velocity_y[i].v;
			velocity_y[i].v--;
			if(velocity_y[i].v < to_dsp(-0.1f)) {
				velocity_y[i].v = (to_dsp(3.0f) + (irand() % to_dsp(2.0f)));
				velocity_x[i].v = (velocity_x[i].v < to_dsp(0.0f))
					? to_dsp(+2.0f)
					: to_dsp(-2.0f);
			}
			if(top[i].to_pixel() > PLAYFIELD_BOTTOM) {
				flag[i] = LF_FREE;
			}
		}
	}

	// Render and detect player collisions
	for(i = 0; i < LEAF_COUNT; i++) {
		if(flag[i] == LF_FREE) {
			continue;
		}
		if(flag[i] == LF_SPARK) {
			leaf_put(vo, first_bit, i, sSPARK);
		} else if(flag[i] <= LF_SPLASH_DONE) {
			if(leaf_on_screen(i)) {
				// ZUN bug: Another missing conversion to screen pixels,
				// resulting in the entire leaf splash animation never being
				// actually rendered, and becoming effectively unused.
				grc_put_8(
					left[i].v, // should be pixels
					top[i].v, // should be pixels
					GRC_SLOT_LEAFSPLASH,
					(flag[i] - LF_SPLASH),
					V_WHITE
				);
			}
			if((frame % 4) == 0) {
#ifdef TH01_NATIVE_ORB_PHYSICS
				flag[i] = static_cast<leaf_flag_t>(flag[i] + 1);
#else
				static_cast<int>(flag[i])++;
#endif
			}
		} else if(flag[i] == LF_LEAF) {
			leaf_put(vo, first_bit, i, ((velocity_x[i].v < to_dsp(0.0f))
				? sLEAF_LEFT
				: sLEAF_RIGHT
			));
			if(
				(top[i].v > to_dsp(player_top)) &&
				(top[i].v < to_dsp(player_bottom() - LEAF_H)) &&
				(left[i].to_pixel() > player_left) &&
				(left[i].to_pixel() < (player_left + PLAYER_W - LEAF_W)) &&
				!player_invincible
			) {
				player_is_hit = true;
			}
		}
	}

	#undef leaf_put
	#undef leaf_on_screen
}

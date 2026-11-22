static const pixel_t GIRL_W = 128;
static const pixel_t GIRL_H = 96;
static const pixel_t BAT_W = 48;
static const pixel_t BAT_H = 32;

static const screen_x_t BASE_CENTER_X = PLAYFIELD_CENTER_X;
static const screen_y_t BASE_CENTER_Y = (
	PLAYFIELD_TOP + ((PLAYFIELD_H / 21) * 5)
);

static const screen_x_t BASE_LEFT = (BASE_CENTER_X - (GIRL_W / 2));
static const screen_y_t BASE_TOP = (BASE_CENTER_Y - (GIRL_H / 2));
// -----------

enum elis_colors_t {
	COL_FX = 4,
	COL_ENTRANCE_SPHERE = 8,
};

// Always denotes the last phase that ends with that amount of HP.
enum elis_hp_t {
	HP_TOTAL = 14,
	HP_PHASE_1_END = 10,
	HP_PHASE_3_END = 6,
	HP_PHASE_5_END = 0,
};

// Entities
// --------

enum elis_form_t {
	F_GIRL = 0,
	F_BAT = 1,

	_elis_form_t_FORCE_INT16 = 0x7FFF,
};

static const int BAT_CELS = 3;
static const int BAT_FRAMES_PER_CEL = 3;
static const int BAT_CYCLE_FRAMES = (BAT_CELS * BAT_FRAMES_PER_CEL);
static const int BAT_SPEED_DIVISOR = 3;

enum elis_entity_cel_t {
	// ENT_STILL_OR_WAVE
	C_STILL = 0,
	C_HAND = 1,
	C_WAVE_1 = 2,
	C_WAVE_2 = 3,
	C_WAVE_3 = 4,
	C_WAVE_4 = 5, // (unused)

	// ENT_ATTACK
	C_PREPARE = 0,
	C_ATTACK_1 = 1,
	C_ATTACK_2 = 2,

	// ENT_BAT
	C_BAT = 0,
	C_BAT_last = (C_BAT + BAT_CELS - 1),
};

struct CGirlEntity : public CBossEntitySized<GIRL_W, GIRL_H> {
	pixel_t surround_area_offset_y(pixel_t area_h) const {
		return -(area_h - h_static()); // not centered
	}

	void pos_cur_set_to(CBossEntity& other) {
		pos_cur_set(other.cur_left, other.cur_top);
	}
};

struct CBatEntity : public CBossEntitySized<BAT_W, BAT_H> {
	pixel_t surround_area_offset_y(pixel_t area_h) const {
		return -((area_h - h_static()) / 2); // centered
	}

	void pos_cur_set_to(CGirlEntity& girl) {
		pos_cur_set(
			(girl.cur_left + ((girl.w_static() - w_static()) / 2)),
			(girl.cur_top  + ((girl.h_static() - h_static()) / 2))
		);
	}
};

#ifdef TH01_NATIVE_ORB_PHYSICS
static CGirlEntity native_elis_girl{},native_elis_attack{};
static CBatEntity native_elis_bat{};
#define ent_still_or_wave native_elis_girl
#define ent_attack native_elis_attack
#define ent_bat native_elis_bat
#else
#define ent_still_or_wave \
	reinterpret_cast<CGirlEntity &>(boss_entity_0)

#define ent_attack \
	reinterpret_cast<CGirlEntity &>(boss_entity_1)

#define ent_bat \
	reinterpret_cast<CBatEntity &>(boss_entity_2)

#endif
// 1 for [ent_still_or_wave], 2 for [ent_attack]. Not actually needed anywhere
// it's used, so it doesn't warrant subclassing those two just to solve that
// distinction via pseudo-polymorphism.
typedef int unnecessary_1_or_2_t;

inline void elis_ent_load(void) {
	ent_still_or_wave.load("boss5.bos", 0);
	ent_attack.load("boss5_2.bos", 1);
	ent_bat.load("boss5_3.bos", 2);
}

inline void elis_ent_free(void) {
	bos_entity_free(0);
	bos_entity_free(1);
	bos_entity_free(2);
}

// It's needed in the functions below...
void girl_bg_put(unnecessary_1_or_2_t unnecessary);

// [unput_0] should theoretically always be `true`. Making sure that both VRAM
// pages are identical avoids visual glitches from blitting cels with different
// transparent areas on top of each other (see: Mima's third arm)… but you can
// always just *assume* rather than ensure, right? :zunpet:
inline void ent_unput_and_put_both(
	CBossEntity& ent,
	unnecessary_1_or_2_t unnecessary,
	elis_entity_cel_t cel,
	bool unput_0 = true
) {
	page_access(1);
	girl_bg_put(unnecessary);
	ent.unlock_put_image_lock_8(cel);
	page_access(0);
	if(unput_0) {
		girl_bg_put(unnecessary);
	}
	ent.unlock_put_image_lock_8(cel);
}

inline void ent_put_both(CBossEntity& ent, elis_entity_cel_t cel) {
	page_access(1); ent.unlock_put_image_lock_8(cel);
	page_access(0); ent.unlock_put_image_lock_8(cel);
}

#define ent_attack_render() { \
	if((boss_phase_frame % 8) == 0) { \
		ent_unput_and_put_both(ent_attack, 2, C_ATTACK_1); \
	} else if((boss_phase_frame % 8) == 4) { \
		ent_put_both(ent_attack, C_ATTACK_2); \
	} \
}
// --------


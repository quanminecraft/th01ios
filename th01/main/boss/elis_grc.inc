static const pixel_t SPHERE_W = 32;
static const pixel_t SPHERE_H = 32;

enum elis_grc_cel_t {
	RIFT_CELS = 2,

	C_EMPTY = 0,
	C_SPHERE_SMALL,
	C_SPHERE_MEDIUM,
	C_SPHERE_LARGE,
	C_RIFT,
	C_RIFT_last = (C_RIFT + RIFT_CELS - 1),
};

#define elis_grc_sloppy_unput(entities, i) { \
	grc_sloppy_unput(entities.left[i], entities.top[i]); \
}

#define elis_grc_put(entities, i, cel, col) { \
	grc_put_8(entities.left[i], entities.top[i], GRC_SLOT_BOSS_1, cel, col); \
}

#define sphere_put(left, top, cel) { \
	grc_put_8(left, top, GRC_SLOT_BOSS_1, cel, COL_ENTRANCE_SPHERE); \
}

#define sphere_unput(left, top) { \
	grc_sloppy_unput(left, top) \
}


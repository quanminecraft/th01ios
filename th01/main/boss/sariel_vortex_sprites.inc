static const pixel_t VORTEX_W = 32;
static const pixel_t VORTEX_H = 32;
static const pixel_t DEBRIS_W = 32;
static const pixel_t DEBRIS_H = 32;

static const int VORTEX_COUNT = 2;

enum vortex_or_debris_cel_t {
	VORTEX_CELS = 3,
	DEBRIS_CELS = 3,

	C_VORTEX = 0,
	C_VORTEX_last = (C_VORTEX + VORTEX_CELS - 1),
	C_DEBRIS,
	C_DEBRIS_last = (C_DEBRIS + DEBRIS_CELS - 1),

	_vortex_or_debris_cel_t_FORCE_INT16 = 0x7FFF
};

inline vortex_or_debris_cel_t debris_cel_next(vortex_or_debris_cel_t &cur) {
	return (cur < C_DEBRIS_last)
		? static_cast<vortex_or_debris_cel_t>(cur + 1)
		: C_DEBRIS;
}

#define vortex_or_debris_put_8(left, top, cel) \
	grc_put_8(left, top, GRC_SLOT_VORTEX_DEBRIS, cel, V_WHITE);
// ----------------------------------------

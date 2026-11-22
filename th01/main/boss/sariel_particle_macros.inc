static const pixel_t PARTICLE2X2_W = 2;
static const pixel_t PARTICLE2X2_H = 2;
static const int PARTICLE2X2_COUNT = 30;
static const dots8_t sPARTICLE2X2 = 0xC0; // (**      )

#define particle2x2_linear_vram_offset(vo, first_bit, left, top) { \
	vo = vram_offset_divmul(left, top); \
	first_bit = (static_cast<screen_x_t>(left) % BYTE_DOTS); \
}

// Better.
#define particle2x2_snap(dots, vo, first_bit) { \
	dots[0] = grcg_chunk_8(vo); \
	dots[1] = grcg_chunk_8(vo + ROW_SIZE); \
	dots[0] &= (sPARTICLE2X2 >> first_bit); \
	dots[1] &= (sPARTICLE2X2 >> first_bit); \
}

// Worse.
#define particle2x2_snap_2(dots, vo, first_bit) { \
	dots[0] = grcg_chunk(vo, 8); \
	/* Parentheses omitted for code generation reasons */ \
	dots[1] = grcg_chunk(vo + ROW_SIZE, 8); \
	dots[0] &= (sPARTICLE2X2 >> first_bit); \
	dots[1] &= (sPARTICLE2X2 >> first_bit); \
}

#define particle2x2_snap_left_right(dots_left, dots_right, vo, first_bit) { \
	particle2x2_snap_2(dots_left, vo, first_bit); \
	if(first_bit == ((BYTE_DOTS - PARTICLE2X2_W) + 1)) { \
		/* Parentheses omitted for code generation reasons */ \
		dots_right[0] = grcg_chunk(vo + 1, 8); \
		dots_right[1] = grcg_chunk(vo + 1 + ROW_SIZE, 8); \
		/* We only care about the leftmost dot, if anything */ \
		dots_right[0] &= 0x80; \
		dots_right[1] &= 0x80; \
	} \
}

#define particle2x2_put(vo, first_bit, dots) { \
	grcg_put_emptyopt(vo, dots[0], 8); \
	/* Parentheses omitted for code generation reasons */ \
	grcg_put_emptyopt(vo + ROW_SIZE, dots[1], 8); \
}

#define particle2x2_put_left_right(vo, first_bit, dots_left, dots_right) { \
	particle2x2_put(vo, first_bit, dots_left); \
	if(first_bit == ((BYTE_DOTS - PARTICLE2X2_W) + 1)) { \
		particle2x2_put(vo + 1, first_bit, dots_right); \
	} \
}
// -------------

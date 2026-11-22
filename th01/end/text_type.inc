static const unsigned int TYPE_DELAY = 3;
static const vc_t COL_TYPE = 15;
static const int16_t TYPE_FX = (COL_TYPE | FX_WEIGHT_NORMAL);

void graph_type_ank_n(
	screen_x_t left, vram_y_t top, int len, const sshiftjis_t *str
)
{
	for(int i = 0; i < len; i++) {
		graph_printf_fx(
			left + (i * GLYPH_HALF_W), top, TYPE_FX, "%c", str[i]
		);
		frame_delay(TYPE_DELAY);
	}
}

void graph_type_kanji_n(
	screen_x_t left, vram_y_t top, int len, const sshiftjis_kanji_t *str
)
{
	for(int i = 0; i < len; i++) {
		graph_printf_fx(
			left + (i * GLYPH_FULL_W), top, TYPE_FX,
			"%c%c", str[(2 * i) + 0], str[(2 * i) + 1]
		);
		frame_delay(TYPE_DELAY);
	}
}

#define graph_type_ank(left, top, str) { \
	graph_type_ank_n(left, top, (sizeof(str) - 1), str); \
}
#define graph_type_kanji(left, top, str) { \
	graph_type_kanji_n(left, top, ((sizeof(str) - 1) / 2), str); \
}

// > rendering text to VRAM, where it wouldn't be limited to the byte grid
// > still aligning it to the byte grid
inline void pic_caption_type_n(int line, size_t len, const char str[]) {
	graph_type_ank_n(
		(((RES_X / 2) - ((len * GLYPH_HALF_W) / 2) + BYTE_MASK) & ~BYTE_MASK),
		(CUTSCENE_PIC_TOP + CUTSCENE_PIC_H + (((line * 2) + 1) * GLYPH_H)),
		len,
		str
	);
}

// MODDERS: Remove the [incorrect_extra_w] parameter.
#define pic_caption_type_2(line_1, line_2, incorrect_extra_w) { \
	/* ZUN landmine: This accesses one extra byte for the "STAGE 5 BOSS" \
	 * string. Which, luckily, happens to be the null terminator, and doesn't \
	 * have any visible effect. */ \
	pic_caption_type_n(0, (sizeof(line_1) - 1 + incorrect_extra_w), line_1); \
	\
	pic_caption_type_n(1, (sizeof(line_2) - 1 + incorrect_extra_w), line_2); \
}
// -----------


static const int FRAME_COUNT = (
	(RES_X / INTERLEAVE_W) + (RES_Y / INTERLEAVE_H)
);

void pagetrans_diagonal_8x8_with_palette(
	unsigned int step_ms, Palette4 &pal_cur, const Palette4 &pal_target
)
{
	int comp;
	svc2 col;
	screen_x_t from_topleft_left = 0;
	vram_y_t from_topleft_top = 0;
	screen_x_t from_bottomright_left = (RES_X - INTERLEAVE_W);
	vram_y_t from_bottomright_top = (RES_Y - INTERLEAVE_H);
	screen_x_t tip_left;
	vram_y_t tip_top;

	for(int frame = 0; frame < FRAME_COUNT; frame++) {
		tip_left = from_topleft_left;
		tip_top = from_topleft_top;
		while((from_topleft_left >= 0) && (from_topleft_top <= (RES_Y - 1))) {
			graph_interleave_pages_8x8_8(
				from_topleft_left, from_topleft_top, 1
			);
			from_topleft_left -= INTERLEAVE_W;
			from_topleft_top += INTERLEAVE_H;
		}

		from_topleft_left = (tip_left < (RES_X - INTERLEAVE_W))
			? (tip_left + INTERLEAVE_W)
			: tip_left;
		from_topleft_top = (tip_left < (RES_X - INTERLEAVE_W))
			? 0
			: (tip_top + INTERLEAVE_H);

		if((frame % (FRAME_COUNT / COLOR_COUNT)) == 0) {
			// Not identical to the ramping loop in z_palette_fade_from()!
			palette_foreach(col, comp, {
				if(pal_cur[col].v[comp] != pal_target[col].v[comp]) {
					pal_cur.colors[col].v[comp] +=
						(pal_cur[col].v[comp] > pal_target[col].v[comp])
						? -1
						:  1;
				}
			});
			z_palette_set_all_show(pal_cur);
		}

		tip_left = from_bottomright_left;
		tip_top = from_bottomright_top;
		while(
			(from_bottomright_left >= 0) &&
			(from_bottomright_top <= (RES_Y - 1))
		) {
			graph_interleave_pages_8x8_8(
				from_bottomright_left, from_bottomright_top, 2
			);
			from_bottomright_left -= INTERLEAVE_W;
			from_bottomright_top += INTERLEAVE_H;
		}

		from_bottomright_left = (tip_top > 0)
			? (RES_X - INTERLEAVE_W)
			: (tip_left - INTERLEAVE_W);
		from_bottomright_top = (tip_top > 0)
			? (tip_top - INTERLEAVE_H)
			: tip_top;

		delay(step_ms);
	}
}

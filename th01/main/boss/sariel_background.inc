void near bg_transition(int image_id_new)
{
	// Terminology:
	// • Gust: The full animation drawn across the whole width of VRAM
	// • Cell: Area drawn in a single loop iteration, randomly shifted
	// • Stripe: 8×1 pixels drawn in the same color, followed by an 8-pixel gap

	enum {
		VELOCITY_X = 32,
		ROW_SPACING = 4,
		STRIPES_PER_CELL = 5,
		STRIPE_PADDED_VRAM_W = 2,
	};

	static screen_x_t cell_x;
	static vram_y_t cell_y;
	static vram_offset_t cell_vo;
	static vc2 stripe_col_base;
	static int gust_id;

	int row;
	vram_word_amount_t stripe_id;
	vram_byte_amount_t cell_offset_right;

	struct {
		// Storing 8 dots would have been enough.
		dots16_t put[STRIPES_PER_CELL];
		dots16_t unput[STRIPES_PER_CELL];

		// Yeah, unsafe, but this code is hardware-dependent anyway :P
		dots16_t& operator [](int stripe_id) {
#ifdef TH01_NATIVE_ORB_PHYSICS
			return stripe_id < STRIPES_PER_CELL ? put[stripe_id] : unput[stripe_id - STRIPES_PER_CELL];
#else
			return put[stripe_id];
#endif
		}
	} cell_mask;

	#define stripe_vram_offset(id_static, id_dynamic) \
		/* The parentheses around the whole expression are omitted for */ \
		/* code generation reasons, since this macro is used as an operand */ \
		/* for pointer arithmetic. */ \
		cell_vo + cell_offset_right - (id_static * STRIPE_PADDED_VRAM_W) - ( \
			id_dynamic * STRIPE_PADDED_VRAM_W \
		)

	#define stripe_y(id_static, id_dynamic) \
		((stripe_vram_offset(id_static, id_dynamic)) / ROW_SIZE)

	#define stripe_vram(id_static, id_dynamic) \
		grcg_chunk(stripe_vram_offset(id_static, id_dynamic), 16)

	#define on_same_y_as_cell(id_static, id_dynamic) ( \
		/* Note how (cell_vo / ROW_SIZE) == cell_y */ \
		((cell_vo / ROW_SIZE) == stripe_y(id_static, id_dynamic)) && \
		(stripe_vram_offset(id_static, id_dynamic) >= 0) \
	)

	if(boss_phase_frame == 20) {
		gust_id = 0;
	}
	if(boss_phase_frame < 50) {
		return;
	} else if(boss_phase_frame == 50) {
		cell_x = 0;
		cell_y = 0;
		stripe_col_base = 2;
		cell_vo = vram_offset_shift(0, 0);
		mdrv2_se_play(10);
	}

	for(row = 0; row < (RES_Y / ROW_SPACING); row++) {
		grcg_setcolor_tcr(COL_AIR);
		cell_offset_right = (irand() % 8);

		page_access(1);
		for(stripe_id = 0; stripe_id < (STRIPES_PER_CELL * 2); stripe_id++) {
			if(stripe_y(0, 0) != stripe_y(0, stripe_id)) {
				break;
			}
			if(on_same_y_as_cell(0, stripe_id)) {
				cell_mask[stripe_id] = stripe_vram(0, stripe_id);
			}
		}

		page_access(0);
		for(stripe_id = 0; stripe_id < STRIPES_PER_CELL; stripe_id++) {
			if(stripe_y(0, 0) != stripe_y(0, stripe_id)) {
				break;
			}
			if(on_same_y_as_cell(0, stripe_id)) {
				grcg_setcolor_rmw(
					stripe_col_base + ((stripe_id > (STRIPES_PER_CELL / 2))
						? ((STRIPES_PER_CELL - 1) - stripe_id)
						: stripe_id
				));
				stripe_vram(0, stripe_id) = (cell_mask.put[stripe_id] & 0x00FF);
			}
		}

		grcg_setcolor_rmw(COL_AIR);
		for(stripe_id = 0; stripe_id < (gust_id + 2); stripe_id++) {
			if(stripe_y(STRIPES_PER_CELL, stripe_id) != stripe_y(0, 0)) {
				break;
			}
			if(on_same_y_as_cell(STRIPES_PER_CELL, stripe_id)) {
				stripe_vram(STRIPES_PER_CELL, stripe_id) = (
					cell_mask.unput[stripe_id] & 0x00FF
				);
			}
		}

		cell_y += ROW_SPACING;
		cell_vo = vram_offset_shift(cell_x, cell_y);
		// Yeah, this could have just been done unconditionally outside the
		// loop.
		if(cell_vo >= PLANE_SIZE) {
			cell_y = 0;
			cell_x += VELOCITY_X;
			cell_vo = vram_offset_shift(cell_x, cell_y);
		}
		grcg_off_func();
	}

	if(cell_x > (RES_X - VELOCITY_X)) {
		gust_id++;
		boss_phase_frame = 49;
		if(gust_id >= 3) {
			z_vsync_wait_and_scrollup(0);

			page_access(1);
			grp_put(BG_IMAGES[image_id_new], GPF_PALETTE_SHOW);
			graph_copy_page_to_other(1);
			stage_palette_set(z_Palettes);

			page_access(0);
			hud_rerender();
			hud_hp_rerender(boss_hp);
			boss_phase_frame = 0;
		}
	}

	#undef on_same_y_as_cell
	#undef stripe_vram
	#undef stripe_y
	#undef stripe_vram_offset
}

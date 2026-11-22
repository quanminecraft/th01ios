void whiteline_put(screen_y_t y)
{
	vram_offset_t vram_offset = vram_offset_shift(0, y);
	vram_dword_amount_t x;

	grcg_setcolor_rmw(15);
	x = 0;
	while(x < (ROW_SIZE / sizeof(dots32_t))) {
		grcg_put(vram_offset, 0xFFFFFFFF, 32);
		x++;
		vram_offset += static_cast<vram_offset_t>(sizeof(dots32_t));
	}
	grcg_off_func();
}

void whitelines_animate(void)
{
	unsigned int i = 0;
	screen_y_t y1 = 0;
	screen_y_t y2 = 0;
	bool drawn_at[RES_Y] = { false };

	while(i++ < (RES_Y / 4)) {
		egc_copy_rect_1_to_0_16(0, y1, RES_X, 1);
		egc_copy_rect_1_to_0_16(0, y2, RES_X, 1);

		do {
			y1 = (irand() % RES_Y);
		} while(drawn_at[y1]);
		drawn_at[y1] = true;

		do {
			y2 = (irand() % RES_Y);
		} while(drawn_at[y2]);
		drawn_at[y2] = true;

		whiteline_put(y1);
		whiteline_put(y2);
		frame_delay(1);
	}
	graph_copy_page_to_other(1);
	page_access(0);
}

void title_init(void)
{
	mdrv2_bgm_load("reimu.mdt");
	mdrv2_bgm_play();
	page_access(1);
	grp_put("REIIDEN2.grp", GPF_PALETTE_SHOW);
	z_palette_black();
	graph_copy_page_to_other(1);
	page_access(1);
	grp_put("REIIDEN3.grp", GPF_PALETTE_KEEP);
	page_access(0);
	z_palette_black_in();
	frame_delay(100);

	whitelines_animate();
}

void title_window_put(void)
{
	graph_copy_page_to_other(1);
	page_access(0);
	grp_put("op_win.grp", GPF_COLORKEY);
	graph_copy_page_to_other(0);
	page_access(0);
}


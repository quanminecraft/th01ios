void sariel_entrance(int8_t)
{
	pellet_interlace = true;

	text_fillca(' ', (TX_BLACK | TX_REVERSE));

	/*  page_access(0);  */ grp_put("boss6_l.grp", GPF_PALETTE_SHOW);
	/**/page_access(1)/**/; grp_put("boss6_h.grp", GPF_PALETTE_SHOW);

	page_access(0);
	stageobjs_init_and_render(BOSS_STAGE);
	mdrv2_bgm_load("TENSI.MDT");
	mdrv2_se_load(SE_FN); // ZUN bloat: Already done in main()
	mdrv2_bgm_play();

	text_fillca(' ', TX_WHITE);

	// Scroll
	// ------

	for(pixel_t line_on_top = (RES_Y - 1); line_on_top >= 0; line_on_top--) {
		EGCCopy egc;
		z_vsync_wait_and_scrollup(line_on_top);
		egc.rect_interpage(0, line_on_top, RES_X, 1, 1);
		frame_delay(1);
	}
	z_vsync_wait_and_scrollup(0);
	// ------

	page_access(1);
	grp_put(BG_IMAGES[0], GPF_PALETTE_SHOW);
	page_access(0);
	pagetrans_diagonal_8x8(40);

	random_seed = frame_rand;
}


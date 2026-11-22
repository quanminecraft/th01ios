void konngara_load_and_entrance(int8_t)
{
	int i;
	int j;
	int in_quarter;
	svc2 ramp_col;
	int ramp_comp;
	int scroll_frame;

	pellet_interlace = true;

	text_fillca(' ', (TX_BLACK | TX_REVERSE));

	// page_access(0);
	grp_put(SCROLL_BG_FN, GPF_PALETTE_SHOW);

	// ZUN bug: On its own, this call at this position in the code would just
	// be redundant as the stage palette is never used here. After all, you
	// can't enter the Pause menu or bomb during a blocking animation function.
	// However, it's also the only call to stage_palette_set() in all of
	// Konngara's code, and ends up setting a palette that doesn't exactly
	// match the one used for the rest of the fight. Most notably, this is why
	// entering the Pause menu or bombing changes color #10 from red to green,
	// which is only corrected by boss_hit_update_and_render()'s periodic reset
	// of the hardware palette to the [boss_palette].
	stage_palette_set(z_Palettes);

	stageobjs_init_and_render(BOSS_STAGE);

	page_access(1);
	grp_put("boss8_a1.grp", GPF_PALETTE_SHOW);
	// The stage_palette_set() call should have been here.

	page_access(0);
	mdrv2_bgm_load("ALICE.MDT");
	mdrv2_se_load(SE_FN); // ZUN bloat: Already done in main()
	mdrv2_bgm_play();

	z_palette_set_black(j, i);

	text_fillca(' ', TX_WHITE);
	konngara_ent_load();

	// Decelerating scroll
	// -------------------

	#define quarters_remaining	i
	#define line_on_top       	j

	line_on_top = 0;
	quarters_remaining = 32; // Should be divisible by 4.
	in_quarter = 0;
	scroll_frame = 0;

	do {
		z_vsync_wait_and_scrollup(line_on_top);
		line_on_top += quarters_remaining;
		if((in_quarter == 0) && (line_on_top > ((RES_Y / 4) * 1))) {
			in_quarter++;
			quarters_remaining--;
		}
		if((in_quarter == 1) && (line_on_top > ((RES_Y / 4) * 2))) {
			in_quarter++;
			quarters_remaining--;
		}
		if((in_quarter == 2) && (line_on_top > ((RES_Y / 4) * 3))) {
			in_quarter++;
			quarters_remaining--;
		}
		if((in_quarter == 3) && (line_on_top > ((RES_Y / 4) * 4))) {
			in_quarter = 0;
			quarters_remaining--;
			line_on_top -= RES_Y;
		}
		if(quarters_remaining <= 0) {
			break;
		}
		if((scroll_frame % 8) == 0) {
			z_palette_black_in_step_to(ramp_col, ramp_comp, grp_palette)
		}
		scroll_frame++;
		frame_delay(1);
	} while(1);

	#undef line_on_top
	#undef quarters_remaining
	// -------------------

	z_vsync_wait_and_scrollup(0);
	grz_load_single(0, GRZ_FN, 0);
	grz_load_single(1, GRZ_FN, 1);
	grz_load_single(2, GRZ_FN, 2);
	grz_load_single(3, GRZ_FN, 3);
	grz_load_single(4, GRZ_FN, 4);
	grz_load_single(5, GRZ_FN, 5);
	grz_load_single(6, GRZ_FN, 6);

	frame_delay(40);

	// Shaking and panning
	// -------------------

	enum {
		MAGNITUDE = 16,
	};
	#define frame      	i
	#define line_on_top	j

	// Shake (below)
	for(frame = 0; frame < 32; frame++) {
		z_vsync_wait_and_scrollup(
			(RES_Y + MAGNITUDE) - ((frame % 2) * (MAGNITUDE * 2))
		);
		if((frame % 8) == 0) {
			mdrv2_se_play(9);
		}
		frame_delay(1);
	}

	// "Pan" up to Konngara
	for(line_on_top = RES_Y; line_on_top >= 0; line_on_top -= (MAGNITUDE * 2)) {
		EGCCopy egc;
		z_vsync_wait_and_scrollup(line_on_top);
		egc.rect_interpage(0, line_on_top, RES_X, (MAGNITUDE * 2), 1);
		frame_delay(1);
	}

	// Shake
	for(frame = 0; frame < 32; frame++) {
		z_vsync_wait_and_scrollup(
			(RES_Y + MAGNITUDE) - ((frame % 2) * MAGNITUDE)
		);
		frame_delay(1);
	}

	#undef line_on_top
	#undef frame
	// -------------------

	frame_delay(30);

	// Flashing Siddhaṃ seed syllables
	// -------------------------------

	siddham_col_white();
	grp_put("boss8_d1.grp", GPF_COLORKEY);
	grp_put("boss8_d2.grp", GPF_COLORKEY);
	grp_put("boss8_d3.grp", GPF_COLORKEY);
	grp_put("boss8_d4.grp", GPF_COLORKEY);

	for(j = 0; j < RGB4::Range; j++) {
		siddham_col_white_in_step();
		frame_delay(10);
	}
	graph_copy_page_to_other(0);
	page_access(0);
	// -------------------------------
}


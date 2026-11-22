printf("\x1B*"); // Clear text RAM
		konngara_free();
		z_graph_clear();
		mdrv2_bgm_stop();

		kuji_put(BLACK, 7);
		kuji_put(WHITE, 8);
		kuji_put(BLACK, 9);
		kuji_put(WHITE, 10);
		kuji_put(BLACK, 11);
		kuji_put(WHITE, 12);
		kuji_put(BLACK, 13);
		kuji_put(WHITE, 14);
		kuji_put(BLACK, 15);

		boss_phase_frame = 0;
		while(1) {
			enum {
				MAGNITUDE = -8
			};

			boss_phase_frame++;
			if((boss_phase_frame % 4) == 0) {
				z_palette_black_out_step(j, i);
				mdrv2_se_play(7);
			}
			z_vsync_wait_and_scrollup(
				(RES_Y - MAGNITUDE) - ((boss_phase_frame % 2) * (MAGNITUDE * 2))
			);
			if(boss_phase_frame > 64) {
				break;
			}
			frame_delay(1);
		}

		// Blit the scroll background, while covering the blit with ridiculously
		// slow text RAM clears to black and back to transparency.
		//
		// "Graph mode" (as opposed to "kanji mode") disables Shift-JIS decoding
		// inside NEC's IO.SYS. This allows new half-width glyphs at the
		// Shift-JIS lead byte code points, 0x81-0x9F and 0xE0-0xFF, to be
		// accessed via regular INT 29h text output, and consequently, printf().
		// Had to reverse-engineer that, only to find out that it has exactly
		// zero effect when printing spaces...
		// See https://github.com/joncampbell123/dosbox-x/pull/2547 for a
		// reimplementation of the original functionality into DOSBox-X.
		// --------------------------------------------------------------------

		printf("\x1B)3"); // Enter graph mode
		text_fill_black(j, i);

		grp_put(SCROLL_BG_FN, GPF_PALETTE_SHOW);
		z_palette_set_black(j, i);

		printf("\x1B)0"); // Back to regular kanji mode
		text_clear_sloppy(j, i);
		// --------------------------------------------------------------------

		// Final scroll
		// ------------

		#define scroll_speed	i
		#define line_on_top 	j

		line_on_top = 0;
		scroll_speed = 32;
		scroll_frame = 0;

		while(1) {
			z_vsync_wait_and_scrollup(line_on_top);
			line_on_top += scroll_speed;
			if(line_on_top > RES_Y) {
				line_on_top -= RES_Y;
			}
			if(scroll_frame < 150) {
				if((scroll_frame % 8) == 0) {
					z_palette_black_in_step_to(col, comp, grp_palette);
					mdrv2_se_play(7);
				}
			} else if((scroll_frame % 8) == 0) {
				if(scroll_frame == 192) {
					// We aren't playing anything, though?
					mdrv2_bgm_fade_out_nonblock();
				}
				z_palette_black_out_step_2(col, comp);
				if(z_Palettes[V_WHITE].c.r == 0) {
					break;
				}
				if(scroll_frame < 220) {
					mdrv2_se_play(7);
				}
			}
			scroll_frame++;
			frame_delay(1);
		}

		#undef line_on_top
		#undef scroll_speed
		// ------------

		z_vsync_wait_and_scrollup(0);

		// Jigoku clear! Grant 50,000 score points in the fanciest way
		for(j = 0; j < 5; j++) {
			score += 10000;
		}
		konngara_free(); // Yes, we already did this above :zunpet:
		game_cleared = true;
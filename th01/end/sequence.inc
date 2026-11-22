inline void end_done(void) {
	mdrv2_bgm_fade_out_nonblock();
	grp_palette_black_out(10);

	z_graph_clear();
	mdrv2_bgm_stop();
	mdrv2_bgm_load("st1.mdt");
	mdrv2_bgm_play();
}

void end_and_verdict_and_regist_animate(void)
{
	enum {
		WHITE_OUT_STEP = 5,
		WHITE_OUT_INTERVAL = 16,
	};

	int i;

	mdrv2_bgm_load("iris.mdt");
	mdrv2_bgm_play();
	grp_palette_settone(0);
	end_pics_load_palette_show("ED1A.grp");

	end_pic_show(0);
	grp_palette_black_in(6);
	frame_delay(100);

	grp_palette_white_out(5);
	frame_delay(100);

	// Closing barn door
	end_pic_show(1);
	grp_palette_settone(100);
	frame_delay(13);
	end_pic_show_and_delay(2, 13);
	end_pic_show_and_delay(3, 50);

	// Seal
	end_pics_load_palette_show("ED1B.grp");
	end_pic_show_and_delay(0, 10);
	end_pic_show_and_delay(1, 10);
	end_pic_show_and_delay(2, 100);
	grp_palette_black_out(6);

	page_access(0);
	z_graph_clear(); // ZUN bloat
	grp_palette_settone(100);

	// Reimu sweeping
	end_pics_load_palette_show("ED1C.GRP");
	for(i = 0; i < 6; i++) {
		end_pic_show_and_delay(0, 20);
		end_pic_show_and_delay(1, 20);
	}
	end_pic_show_and_delay(0, 40);
	end_pic_show_and_delay(2, 40);

	// Reimu looking at the Orb
	end_pics_load_palette_show("ED1D.GRP");
	end_pic_show_and_delay(3, 70);
	for(i = 0; i < 100; i++) {
		#define blink_at(first_frame, frame) { \
			if(frame == (first_frame + 0)) { end_pic_show(1); } \
			if(frame == (first_frame + 4)) { end_pic_show(2); } \
			if(frame == (first_frame + 8)) { end_pic_show(0); } \
		}

		if(i == 0) { end_pic_show(0); }
		blink_at(20, i);
		blink_at(50, i);
		blink_at(90, i);

		frame_delay(2);

		#undef blink_at
	}
	end_pic_show_and_delay(3, 60);
	end_pic_show_and_delay(0, 20);
	end_pic_show(3);
	grp_palette_settone(105);

	// Orb hovering and sparkling
	end_pics_load_palette_show("ED1E.GRP");
	end_pic_show(0);
	for(i = 0; i < ((130 - 110) / WHITE_OUT_STEP); i++) {
		grp_palette_settone(110 + (i * WHITE_OUT_STEP));
		frame_delay(WHITE_OUT_INTERVAL);
	}
	for(i = 0; i < ((205 - 130) / WHITE_OUT_STEP); i++) {
		end_pic_show_and_delay(1, (WHITE_OUT_INTERVAL / 2));
		end_pic_show_and_delay(2, (WHITE_OUT_INTERVAL / 2));
		grp_palette_settone(130 + (i * WHITE_OUT_STEP));
	}

	if(continues_total == 0) {
		end_good();
		boss_slides_animate();
	} else {
		end_bad();
		end_done();
	}
	verdict_animate_and_regist_menu();
}

void near shake_then_boom(int shake_duration, int boom_duration)
{
	int i;

	// <= instead of <? Let's better not treat the parameters as "frames" then.
	for(i = 0; i <= shake_duration; i++) {
		if((i & 3) == 0) { graph_scrollup(8); }
		/*       */ else { graph_scrollup(0); }

		// Redundant, would have been enough to show it once outside the loop,
		// and keep just the delay here.
		end_pic_show_and_delay(1, 1);

		if(i & 1) { grp_palette_settone(170); }
		/**/ else { grp_palette_settone(100); }
	}

	for(i = 0; i <= boom_duration; i++) {
		if((i & 3) == 0) { graph_scrollup(8); }
		/*       */ else { graph_scrollup(0); }

		end_pic_show_and_delay(1, 2);

		if(i & 1) {
			// ZUN bug: No delay after rendering this image. Unlike the
			// z_vsync_wait_and_scrollup() function used in REIIDEN.EXE,
			// master.lib's graph_scrollup() doesn't wait for VSync, causing
			// this image to immediately be overwritten with pic #1 on the next
			// iteration of the loop...
			//
			// ... or so you would think. Turns out that EGC inter-page copies,
			// as used by end_pic_show(), are *really* slow. Combined with the
			// usual slowness of PC-98 VRAM, you'll still end up seeing at
			// least parts of the "boom"/"ドカーン" image even on faster PC-98
			// systems before it's fully overwritten on the next iteration.
			//
			// Technically this could be considered a quirk, as the resulting
			// frame drops affect the length of the ending. On the other hand,
			// we explicitly judge observability in terms of an infinitely fast
			// PC-98. Such a system would consequently never show this image
			// that ZUN clearly intended to show, thus turning this into the
			// exact definition of a bug.
			end_pic_show(2);
		} else {
			// And why are we re-showing the same pic here? Redundant,
			// obviously – there are much better ways of intentionally burning
			// CPU cycles.
			end_pic_show(1);
		}
	}
}

void near end_bad(void)
{
	end_pics_load_palette_show("ed2a.grp");
	grp_palette_settone(200); // (redundant, we white in anyway)
	end_pic_white_in_and_delay(0, 5, 120);

	if(end_flag == ES_JIGOKU) {
		shake_then_boom(64, 16);
		shake_then_boom(32, 32);
		graph_scrollup(0);
		frame_delay(50);

		end_pic_show_and_delay(3, 100);
		pic_caption_type_2(END_BAD_LINE_1, END_BAD_LINE_2_JIGOKU, 0);
	} else {
		enum {
			WHITE_OUT_STEP = 5,
		};

		int i;

		end_pics_load_palette_show("ed4a.grp");

		for(i = 0; i < 20; i++) {
			end_pic_show_and_delay(0, 3);
			end_pic_show_and_delay(1, 3);
		}
		for(i = 0; i < ((200 - 100) / WHITE_OUT_STEP); i++) {
			end_pic_show_and_delay(0, 3);
			end_pic_show_and_delay(1, 3);
			grp_palette_settone(100 + (i * WHITE_OUT_STEP));
		}
		end_pic_white_in_and_delay(2, 5, 200);
		end_pic_show_and_delay(3, 100);
		pic_caption_type_2(END_BAD_LINE_1, END_BAD_LINE_2_MAKAI, 0);
	}
	frame_delay(FINAL_DELAY_FRAMES);
}

void near end_good(void)
{
	enum {
		WHITE_IN_STEP = 5,
		WHITE_OUT_STEP = 10,
	};

	int i;

	end_pics_load_palette_show("ed3a.grp");

	grp_palette_settone(200);
	for(i = 0; i < ((200 - 100) / WHITE_IN_STEP); i++) {
		end_pic_show_and_delay(0, 8);
		end_pic_show_and_delay(1, 8);
		grp_palette_settone(200 - (i * WHITE_IN_STEP));
	}
	grp_palette_settone(100);

	for(i = 0; i < 10; i++) {
		end_pic_show_and_delay(0, 8);
		end_pic_show_and_delay(1, 8);
	}

	for(i = 0; i < ((200 - 100) / WHITE_OUT_STEP); i++) {
		end_pic_show_and_delay((i & 1), 8);
		grp_palette_settone(100 + (i * WHITE_OUT_STEP));
	}
	grp_palette_settone(200);

	if(end_flag == ES_MAKAI) {
		end_good_makai();
	} else {
		end_good_jigoku();
	}
}

void near end_good_jigoku(void)
{
	end_pics_load_palette_show("ed3b.grp");
	end_pic_white_in_and_delay(0, 4, 250);
	end_pic_show_and_delay(1, 200);
	end_pic_show_and_delay(2, 150);
	end_pic_show_and_delay(3, 150);
	pic_caption_type_2(END_GOOD_LINE_1, END_GOOD_LINE_2_JIGOKU, 0);
	frame_delay(FINAL_DELAY_FRAMES);
}

void near end_good_makai(void)
{
	end_pics_load_palette_show("ed5a.grp");
	end_pic_white_in_and_delay(0, 4, 300);

	for(int i = 0; i < 15; i++) {
		end_pic_show_and_delay(1, 8);
		end_pic_show_and_delay(2, 8);
	}

	end_pics_load_palette_show("ed5b.grp");
	end_pic_show_and_delay(0, 150);
	end_pic_show_and_delay(1, 100);
	end_pic_show_and_delay(2, 100);
	end_pic_show_and_delay(3, 100);

	end_pics_load_palette_show("ed5c.grp");
	end_pic_show_and_delay(0, 100);
	end_pic_show_and_delay(1, 150);
	end_pic_show_and_delay(2, 150);
	pic_caption_type_2(END_GOOD_LINE_1, END_GOOD_LINE_2_MAKAI, 0);
	frame_delay(FINAL_DELAY_FRAMES);
}
/// -------

/// Boss slideshow
/// --------------

static const int BOSS_SLIDE_DELAY = 150;
static const int BOSS_TEXT_DELAY = 50;

void near boss_slide_next(int quarter)
{
	frame_delay(BOSS_SLIDE_DELAY);
	grp_palette_black_out(5);
	z_graph_clear();
	end_pic_show(quarter);
	grp_palette_black_in(5);
	frame_delay(BOSS_TEXT_DELAY);
}

void near boss_slides_animate(void)
{
	// MODDERS: Move to end_and_verdict_and_regist_animate() for cleanliness.
	end_done();

	if(end_flag == ES_MAKAI) {
		end_pics_load_palette_show("endb_a.grp");
	} else {
		end_pics_load_palette_show("endb_b.grp");
	}
	end_pic_show(0);
	grp_palette_settone(100);
	frame_delay(BOSS_TEXT_DELAY);

	// ZUN calculated with one extra romaji? Was this character originally
	// called "ShinGyoku" after all?!
	pic_caption_type_2(SLIDES_TITLE_5, SLIDES_BOSS_5, 1);

	boss_slide_next(1);
	if(end_flag == ES_MAKAI) {
		pic_caption_type_2(SLIDES_TITLE_10, SLIDES_BOSS_10_MAKAI, 0);
	} else {
		pic_caption_type_2(SLIDES_TITLE_10, SLIDES_BOSS_10_JIGOKU, 0);
	}

	boss_slide_next(2);
	if(end_flag == ES_MAKAI) {
		pic_caption_type_2(SLIDES_TITLE_15, SLIDES_BOSS_15_MAKAI, 0);
	} else {
		pic_caption_type_2(SLIDES_TITLE_15, SLIDES_BOSS_15_JIGOKU, 0);
	}

	boss_slide_next(3);
	if(end_flag == ES_MAKAI) {
		pic_caption_type_2(SLIDES_TITLE_20, SLIDES_BOSS_20_MAKAI, 0);
	} else {
		pic_caption_type_2(SLIDES_TITLE_20, SLIDES_BOSS_20_JIGOKU, 0);
	}

	frame_delay(BOSS_SLIDE_DELAY);
}
/// --------------


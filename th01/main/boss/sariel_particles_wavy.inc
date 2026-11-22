void near particles2x2_wavy_unput_update_render()
{
	// Also indicates whether a particle is alive.
	static svc_t col[PARTICLE2X2_COUNT] = { 0 };

	static screen_x_t left[PARTICLE2X2_COUNT];
	static vram_y_t top[PARTICLE2X2_COUNT];
	static pixel_t velocity_y[PARTICLE2X2_COUNT];
	static int age[PARTICLE2X2_COUNT];

#ifdef TH01_NATIVE_ORB_PHYSICS
	native_particle_bind(1, col, left, top, velocity_y, age);
	if(native_particle_reset) { std::fill_n(col, PARTICLE2X2_COUNT, 0); return; }
#endif

	int i;
	int first_bit;
	vram_offset_t vo;
	screen_x_t wave_left;
	DotRect<dots8_t, PARTICLE2X2_H> dots;

	if((boss_phase_frame % 7) == 0) {
		for(i = 0; i < PARTICLE2X2_COUNT; i++) {
			if(col[i] != 0) {
				continue;
			}
			left[i] = (irand() % RES_X);
			top[i] = RES_Y;
			velocity_y[i] = -1;
			age[i] = 0;
			col[i] = COL_PARTICLE2X2;
			break;
		}
	}
	if((boss_phase_frame % 2) != 0) {
		return;
	}
	for(i = 0; i < PARTICLE2X2_COUNT; i++) {
		if(col[i] == 0) {
			continue;
		}

		grcg_setcolor_tcr(COL_AIR);

		wave_left = polar_y(left[i], 16, age[i]);
		vo = vram_offset_shift(wave_left, top[i]);
		first_bit = (wave_left & BYTE_MASK);

		// Unblit
		page_access(1);	particle2x2_snap(dots, vo, first_bit);
		grcg_setcolor_rmw(COL_AIR);
		page_access(0);	particle2x2_put(vo, first_bit, dots);

		// Update
		top[i] += velocity_y[i];
		age[i]++;

		// Recalculate VRAM offset and clip
		wave_left = polar_y(left[i], 16, age[i]);
		vo = vram_offset_shift(wave_left, top[i]);
		first_bit = (wave_left & BYTE_MASK);
		if(age[i] >= 100) {
			col[i] = 0;
			continue;
		}

		// Render
		grcg_setcolor_tcr(COL_AIR);
		page_access(1);	particle2x2_snap(dots, vo, first_bit);
		grcg_setcolor_rmw(col[i]);
		page_access(0);	particle2x2_put(vo, first_bit, dots);
	}
	grcg_off_func();
}

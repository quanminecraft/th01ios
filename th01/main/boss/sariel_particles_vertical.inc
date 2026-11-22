void near particles2x2_vertical_unput_update_render(bool16 from_bottom)
{
	// Also indicates whether a particle is alive.
	static svc_t col[PARTICLE2X2_COUNT] = { 0 };

	static double left[PARTICLE2X2_COUNT];
	static double top[PARTICLE2X2_COUNT];
	static double velocity_y[PARTICLE2X2_COUNT];

#ifdef TH01_NATIVE_ORB_PHYSICS
	native_particle_bind(0, col, left, top, velocity_y, nullptr);
	if(native_particle_reset) { std::fill_n(col, PARTICLE2X2_COUNT, 0); return; }
#endif

	int i;
	vram_offset_t vo;
	int first_bit;
	DotRect<dots8_t, PARTICLE2X2_H> dots;

	if((boss_phase_frame % 7) == 0) {
		for(i = 0; i < PARTICLE2X2_COUNT; i++) {
			if(col[i] != 0) {
				continue;
			}
			left[i] = (irand() % RES_X);
			top[i] = ((from_bottom == false) ? 0 : (RES_Y - 1 - PARTICLE2X2_H));
			velocity_y[i] = ((from_bottom == false)
				? (( irand() % 15) + 2)
				: ((-irand() % 15) - 8)
			);
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

		particle2x2_linear_vram_offset(vo, first_bit, left[i], top[i]);

		// Lazy trick to avoid having to touch two adjacent VRAM bytes? Why
		// though, you've got a 16-bit CPU. And why not just shift it to the
		// next VRAM byte (= 1 pixel to the right) rather than 7 pixels to the
		// left? And why is this done every frame?
		if(first_bit == ((BYTE_DOTS - PARTICLE2X2_W) + 1)) {
			first_bit = 0;
		}

		// Unblit
		page_access(1);	particle2x2_snap_2(dots, vo, first_bit);
		grcg_setcolor_rmw(COL_AIR);
		page_access(0);	particle2x2_put(vo, first_bit, dots);

		// Update
		top[i] += velocity_y[i];

		// Recalculate VRAM offset and clip
		vo = vram_offset_divmul(left[i], top[i]);
		if((vo >= (((RES_Y - PARTICLE2X2_H) + 1) * ROW_SIZE) || (vo < 0))) {
			col[i] = 0;
			continue;
		}

		// Render
		grcg_setcolor_tcr(COL_AIR);
		page_access(1);	particle2x2_snap_2(dots, vo, first_bit);
		grcg_setcolor_rmw(col[i]);
		page_access(0);	particle2x2_put(vo, first_bit, dots);
	}
	grcg_off_func();
}

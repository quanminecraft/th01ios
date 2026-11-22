void near particles2x2_horizontal_unput_update_render(int frame)
{
	// Also indicates whether a particle is alive.
	static svc_t col[PARTICLE2X2_COUNT] = { 0 };

	static double left[PARTICLE2X2_COUNT];
	static double top[PARTICLE2X2_COUNT];
	static double velocity_x[PARTICLE2X2_COUNT];

#ifdef TH01_NATIVE_ORB_PHYSICS
	native_particle_bind(2, col, left, top, velocity_x, nullptr);
	if(native_particle_reset) { std::fill_n(col, PARTICLE2X2_COUNT, 0); return; }
#endif

	int i;
	int first_bit;
	vram_offset_t vo;
	DotRect<dots8_t, PARTICLE2X2_H> dots_right;
	DotRect<dots8_t, PARTICLE2X2_H> dots_left;

	if((frame % 7) == 0) {
		for(i = 0; i < PARTICLE2X2_COUNT; i++) {
			if(col[i] != 0) {
				continue;
			}
			left[i] = (irand() % RES_X);
			top[i] = (irand() % RES_Y);
			velocity_x[i] = ((irand() % 2) == 0) ? -6 : 6;
			col[i] = COL_PARTICLE2X2;
			break;
		}
	}
	if((frame % 2) != 0) {
		return;
	}
	for(i = 0; i < PARTICLE2X2_COUNT; i++) {
		if(col[i] == 0) {
			continue;
		}

		grcg_setcolor_tcr(COL_AIR);

		particle2x2_linear_vram_offset(vo, first_bit, left[i], top[i]);

		// Unblit
		page_access(1);
		particle2x2_snap_left_right(dots_left, dots_right, vo, first_bit);
		grcg_setcolor_rmw(COL_AIR);
		page_access(0);
		particle2x2_put_left_right(vo, first_bit, dots_left, dots_right);

		// Update
		left[i] += velocity_x[i];
		velocity_x[i] += (velocity_x[i] < 0) ? 0.1 : -0.1;

		// Recalculate VRAM offset and clip
		particle2x2_linear_vram_offset(vo, first_bit, left[i], top[i]);
		if((left[i] >= RES_X) || (left[i] < 0) || (abs(velocity_x[i]) < 0.5)) {
			col[i] = 0;
			continue;
		}

		// Render
		grcg_setcolor_tcr(COL_AIR);
		page_access(1);
		particle2x2_snap_left_right(dots_left, dots_right, vo, first_bit);

		grcg_setcolor_rmw(col[i]);
		page_access(0);
		particle2x2_put_left_right(vo, first_bit, dots_left, dots_right);
	}
	grcg_off_func();
}

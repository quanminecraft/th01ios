void near totle_pagetrans_animate(int)
{
	upixel_t y;
	unsigned int rows_active = 0;
	uvram_byte_amount_t* row_transfer_offset;
	bool16* row_active;
	unsigned int rows_done = 0;
	vc2 col;
	unsigned int comp;
	unsigned int rows_done_prev = 1;

	#ifdef TH01_NATIVE_ORB_PHYSICS
	std::array<uvram_byte_amount_t,RES_Y> native_offsets{};
	std::array<bool16,RES_Y> native_active{};
	row_transfer_offset=native_offsets.data();row_active=native_active.data();
#else
	row_transfer_offset = new uvram_byte_amount_t[RES_Y];
	row_active = new bool16[RES_Y];
#endif
	for(y = 0; y < RES_Y; y++) {
		row_transfer_offset[y] = 0;
		row_active[y] = false;
	}

	#ifndef TH01_NATIVE_ORB_PHYSICS
	egc_on();
	egc_setup_copy();
#endif

	while(1) {
		for(y = 0; y < RES_Y; y++) {
			if(!row_active[y]) {
				continue;
			}
			egc_pagetrans_rowshift_alternating_32(y, row_transfer_offset[y]);
			row_transfer_offset[y] += ROWSHIFT_CHUNK_SIZE;
			if(row_transfer_offset[y] >= ROW_SIZE) {
				row_active[y] = false;
				rows_done++;
			}
		}
		frame_delay(2);

		if(rows_done >= RES_Y) {
			break;
		}
		if(rows_active < RES_Y) {
			row_active[rows_active + 0] = true;
			row_active[rows_active + 1] = true;
			row_active[rows_active + 2] = true;
			row_active[rows_active + 3] = true;
			row_active[rows_active + 4] = true;
			row_active[rows_active + 5] = true;
			row_active[rows_active + 6] = true;
			row_active[rows_active + 7] = true;
		}
		rows_active += 8;

		if((rows_done != rows_done_prev) && ((rows_active & 0xF) == 0)) {
			z_palette_step_to(col, comp, grp_palette);
		}
		rows_done_prev = rows_done;
	}

	#ifndef TH01_NATIVE_ORB_PHYSICS
	delete[] row_transfer_offset;
	delete[] row_active;
	egc_off();
#endif
}


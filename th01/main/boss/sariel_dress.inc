void near dress_render_both(void)
{
	enum {
		CELS = 4,
		FRAMES_PER_CEL = 15,
	};
	if((boss_phase_frame % FRAMES_PER_CEL) != 0) {
		return;
	}
	int cel = ((boss_phase_frame % (FRAMES_PER_CEL * CELS)) / FRAMES_PER_CEL);
	anm_dress.bos_image = cel;
	page_access(1);	anm_dress.put_8();
	page_access(0);	anm_dress.put_8();
}


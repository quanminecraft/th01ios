void near shield_render_both(void)
{
	enum {
		CELS = 4,
		FRAMES_PER_CEL = 10,
	};
	if((boss_phase_frame % FRAMES_PER_CEL) != 0) {
		return;
	}
	ent_shield.set_image(
		(boss_phase_frame % (FRAMES_PER_CEL * CELS)) / FRAMES_PER_CEL
	);
	page_access(1);	ent_shield.unlock_put_lock_8();
	page_access(0);	ent_shield.unlock_put_lock_8();
}


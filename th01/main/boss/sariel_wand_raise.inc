bool16 near wand_render_raise_both(bool16 restart = false)
{
	static int frame = 0;

	if(restart == true) {
		frame = 0;
	} else if (frame == 0) {
		frame = 1;
	} else {
		frame++;
		if(frame == 2) {
			anm_wand.bos_image = 0;
			page_access(1);	anm_wand.put_8();
			page_access(0);	anm_wand.put_8();
		} else if(frame == 16) {
			anm_wand.bos_image = 1;
			page_access(1);	anm_wand.put_8();
			page_access(0);	anm_wand.put_8();
			frame = 0;
			return true;
		}
	}
	return false;
}


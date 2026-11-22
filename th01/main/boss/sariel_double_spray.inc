void near pattern_curved_spray_leftright_twice(sariel_frame_t &frame)
{
	static CurvedSpray spray;

	if(frame < 80) {
		return;
	} else if(frame == 80) {
		spray.init();
	}
	if((spray.subpattern_id % 2) == 0) {
		spray.fire_and_advance(X_RIGHT);
		if(spray.target.x >= PLAYFIELD_RIGHT) {
			spray.subpattern_id++;
			spray.reset(PLAYFIELD_RIGHT);
		}
	} else {
		spray.fire_and_advance(X_LEFT);
		if(spray.target.x < PLAYFIELD_LEFT) {
			spray.subpattern_id++;
			spray.reset_2(PLAYFIELD_LEFT);
			if(spray.subpattern_id == 4) {
				frame = 0;
			}
		}
	}
}

struct CurvedSpray {
	screen_point_t target;
	Subpixel speed;
	unsigned char angle; // should be local
	int subpattern_id; // controlled by the pattern

	void init(void) {
		target.x = PLAYFIELD_LEFT;
		target.y = PLAYFIELD_BOTTOM;
		speed.set(7.0f);
		subpattern_id = 0;

		// Divisor = number of pellets effectively fired per direction.
		select_for_rank(pattern_state.velocity_x,
			(PLAYFIELD_W / 20),
			(PLAYFIELD_W / 26.66),
			(PLAYFIELD_W / 35.55),
			(PLAYFIELD_W / 40)
		);
	}

	void fire_and_advance(x_direction_t dir) {
		angle = iatan2((target.y - SEAL_CENTER_Y), (target.x - SEAL_CENTER_X));
		Pellets.add_single(SEAL_CENTER_X, SEAL_CENTER_Y, angle, speed);
		speed -= 0.25f;
		if(dir == X_RIGHT) {
			target.x += pattern_state.velocity_x;
		} else {
			target.x -= pattern_state.velocity_x;
		}
	}

	void reset(screen_x_t new_target) {
		speed.set(7.0f);
		target.x = new_target;
	}

	void reset_2(screen_x_t new_target) {
		target.x = new_target;
		speed.set(7.0f);
	}
};

void near pattern_curved_spray_leftright_once(sariel_frame_t &frame)
{
	static CurvedSpray spray;

	if(frame < 50) {
		return;
	} else if(frame == 50) {
		dottedcircle_unput_update_render(
			SEAL_CENTER_X, SEAL_CENTER_Y, 1, 4, 16, V_WHITE, 32, 160
		);
	}

	// The duration can be calculated as:
	//
	//	target_radius = √((RES_X - SEAL_CENTER_X)² + (RES_Y - SEAL_CENTER_Y)²)
	//	frames = (((target_radius - radius_initial) / radius_step) * interval)
	//
	// Good luck doing that at compile time, but given the variables defined
	// here, the result does come out as 88.38. So, close enough.
	dottedcircle_unput_update_render(
		SEAL_CENTER_X, SEAL_CENTER_Y, ((frame - 50) + 1), 4, 16, V_WHITE, 32, 90
	);

	if(frame < 100) {
		return;
	} else if(frame == 100) {
		spray.init();
	}

	if(spray.subpattern_id == 0) {
		spray.fire_and_advance(X_RIGHT);
		if(spray.target.x >= PLAYFIELD_RIGHT) {
			spray.subpattern_id = 1;
			spray.reset(PLAYFIELD_RIGHT);
		}
	} else {
		spray.fire_and_advance(X_LEFT);
		if(spray.target.x < PLAYFIELD_LEFT) {
			frame = 0;
		}
	}
}

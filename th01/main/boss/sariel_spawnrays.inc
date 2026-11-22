#include "th01/main/boss/sariel_ray.inc"

// Two symmetric spawn rays with debris
// ------------------------------------

template <
	screen_x_t OriginX, screen_y_t OriginY, pixel_t DebrisOffsetX
> struct SymmetricSpawnraysWithDebris {
	screen_x_t target_l_x;	// Left ray target, used to calculate the right one.
	screen_y_t target_y;

	screen_x_t target_x(x_direction_t ray_id) const {
		return (ray_id == X_LEFT)
			? target_l_x
			: ((PLAYFIELD_RIGHT - 1) - target_l_x);
	}

	screen_x_t debris_left(x_direction_t ray_id, pixel_t offset = 0) const {
		return (ray_id == X_LEFT)
			? ((target_l_x - (DEBRIS_W - DebrisOffsetX)) - offset)
			: (((PLAYFIELD_RIGHT - DebrisOffsetX) - target_l_x) + offset);
	}

	// Almost a correct unblitting call, except for the top coordinate...
	void debris_sloppy_unput(
		x_direction_t ray_id, pixel_t offset = 0, pixel_t additional_h = 0
	) const {
		egc_copy_rect_1_to_0_16(
			debris_left(ray_id),
			((target_y - (DEBRIS_H / 4)) + offset),
			DEBRIS_W,
			(DEBRIS_H + additional_h)
		);
	}

	// Worse.
	void unput_and_put(
		x_direction_t ray_id,
		const vortex_or_debris_cel_t &debris_cel,
		pixel_t velocity_x
	) const {
		// ZUN bug: Due to egc_copy_rect_1_to_0_16()'s lack of horizontal
		// clamping, an initially negative X coordinate for the left debris
		// sprite, and an extra sloppy 48-pixel width, this unblitting call
		// will wrap around to the other side of the screen for *both* the left
		// and right sprites, unblitting most of the other sprite as well.
		// It will take until
		//
		// 	(target_l_x - (DEBRIS_W / 4) - (velocity_x * 2)) >= 32
		//
		// for both the left and right sprite coordinates to have diverged
		// enough for these wraparounds to stop.
		grc_sloppy_unput(
			debris_left(ray_id, (velocity_x * 2)), (target_y - DEBRIS_H)
		);

		// Also, why only the top half?
		vortex_or_debris_put_8(
			debris_left(ray_id), (target_y - (DEBRIS_H / 2)), debris_cel
		);
		spawnray_unput_and_put(
			OriginX, OriginY, target_x(ray_id), target_y, V_WHITE
		);
	}

	// Better.
	void unput_and_put(
		x_direction_t ray_id, const vortex_or_debris_cel_t &debris_cel
	) const {
		debris_sloppy_unput(ray_id);
		spawnray_unput_and_put(
			OriginX, OriginY, target_x(ray_id), target_y, V_WHITE
		);
		vortex_or_debris_put_8(
			debris_left(ray_id), (target_y - (DEBRIS_H / 2)), debris_cel
		);
	}

	void ray_unput_right(pixel_t velocity_x) const {
		// spawnray_unput_and_put() could have done this as well, no need to
		// change APIs...
		// Also, [target_l_x] was updated after the last time the right ray was
		// rendered, so we have to undo this movement here.
		graph_r_line_unput(
			OriginX, OriginY, (target_x(X_RIGHT) + velocity_x), target_y
		);
	}

	void unput_broken(pixel_t velocity_y) const {
		debris_sloppy_unput(X_RIGHT, -velocity_y, velocity_y);
		debris_sloppy_unput(X_LEFT, -velocity_y, (velocity_y + 10)); // ???

		// ZUN bug: Should of course be [velocity_y] rather than 4. These calls
		// therefore unblit lines at angles that completely differ from the
		// intended spawn rays. It doesn't matter for the left one, which
		// actually isn't visible when this code gets to run, but the right one
		// does indeed stay visible after this function. You only barely notice
		// it because the one pattern that calls this function then spawns
		// pellets on top of both rays, and it's *their* sloppy unblitting code
		// that ends up gradually unblitting the right ray over the next few
		// frames, as a side effect.
		// Again, spawnray_unput_and_put() would have just done the correct
		// thing.
		graph_r_line_unput(OriginX, OriginY, target_x(X_LEFT),  (target_y + 4));
		graph_r_line_unput(OriginX, OriginY, target_x(X_RIGHT), (target_y + 4));
	}

	void fire_random_to_bottom(
		unsigned char &angle, x_direction_t from_dir, subpixel_t speed
	) const {
		angle = iatan2(
			(PLAYFIELD_BOTTOM - target_y), (playfield_rand_x() - target_l_x)
		);
		if(from_dir == X_LEFT) {
			Pellets.add_single(inhibit_Z3(target_l_x), target_y, angle, speed);
		} else {
			Pellets.add_single(target_x(from_dir), target_y, angle, speed);
		}
	}
};
// ------------------------------------

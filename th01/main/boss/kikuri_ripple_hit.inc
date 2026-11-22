bool16 near tear_ripple_hittest(screen_x_t left, pixel_t extra_w)
{
	if(player_invincible != true) {
		// Translation: 8 pixels in Reimu's center vs. 10 pixels in the ripple
		// sprite's center.
		//
		// ZUN bug: … except that ripples are rendered at byte-aligned X
		// positions. This collision detection uses the internal X coordinate
		// though, which can be (and due to the deterministic nature of this
		// part of the fight, regularly is) anywhere within that byte. This
		// ends up shifting this otherwise logical hitbox up to 7 pixels to the
		// right, compared to where you would expect it based on how the
		// sprites appear on screen. (Same as Mima's flame pillars.)
		//
		// (The ideal fix would be to introduce unaligned rendering for these
		// sprites, rather than byte-aligning the coordinates here. The latter
		// would fork the game, obviously.)
		if(
			(player_left >= (left - ((PLAYER_W / 4) + (RIPPLE_W / 2)))) &&
			(player_left <= (left + extra_w))
		) {
			player_is_hit = true;
			return true;
		}
	}
	return false;
}


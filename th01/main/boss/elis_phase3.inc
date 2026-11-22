int phase_3(int id)
{
	#define star_of_david_then(pattern_cur, id, func) { \
		if(pattern_cur == 0) { \
			pattern_cur = star_of_david() /* != SP_STAR_OF_DAVID */; \
			return id; \
		} \
		return (pattern_cur = func()); \
	}

	static int pattern_cur = CHOOSE_NEW;

	if(id == 99) {
		pattern_cur = CHOOSE_NEW;
		return CHOOSE_NEW;
	}

	switch(id) {
	case CHOOSE_NEW:
		// Note that this includes `CHOOSE_NEW`. Due to how phase_frame_common()
		// switches between patterns and teleporting, this adds a 25% chance of
		// Elis skipping an attack cycle and teleporting again.
		return (irand() % 4);
	case 1: /* return */ star_of_david_then(pattern_cur, 1,
		pattern_curved_5_stack_rings
	);
	case 2: /* return */ star_of_david_then(pattern_cur, 2,
		pattern_clusters_from_spheres
	);
	case 3: /* return */ star_of_david_then(pattern_cur, 3,
		pattern_random_from_rifts
	);
	}
	return CHOOSE_NEW;

	#undef star_of_david_then
}


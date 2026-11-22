#define clamp_and_add_x10_to_score_bonus(val, max, unnecessary_i) \
	/* Sneaky! That's how we can pretend this is an actual function that */ \
	/* returns a value. */ \
	val; \
	if(val > max) { \
		val = max; \
	} \
	for(unnecessary_i = 0; unnecessary_i < 10; unnecessary_i++) { \
		score_bonus += val; \
	}

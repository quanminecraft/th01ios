inline int select_for_rank(
	int for_easy, int for_normal, int for_hard, int for_lunatic
) {
	return (
		(rank == RANK_EASY) ? for_easy :
		(rank == RANK_NORMAL) ? for_normal :
		(rank == RANK_HARD) ? for_hard :
		(rank == RANK_LUNATIC) ? for_lunatic :
		for_normal
	);
}

static int pattern_interval;
// --------


#pragma once
enum pellet_group_t {
	// Not a valid pellet type, due to a potential ZUN bug in
	// group_velocity_set()!
	PG_NONE = 0,

	PG_1 = 1,
	PG_1_AIMED = 12,

	PG_2_SPREAD_WIDE,
	PG_2_SPREAD_NARROW,
	PG_3_SPREAD_WIDE,
	PG_3_SPREAD_NARROW,
	PG_4_SPREAD_WIDE,
	PG_4_SPREAD_NARROW,
	PG_5_SPREAD_WIDE,
	PG_5_SPREAD_NARROW,

	// Aimed versions of the n-way spreads above. Expected to have enum values
	// >= this one! Solved more elegantly than in TH02...
	PG_AIMED_SPREADS,

	PG_2_SPREAD_WIDE_AIMED = PG_AIMED_SPREADS,
	PG_2_SPREAD_NARROW_AIMED,
	PG_3_SPREAD_WIDE_AIMED,
	PG_3_SPREAD_NARROW_AIMED,
	PG_4_SPREAD_WIDE_AIMED,
	PG_4_SPREAD_NARROW_AIMED,
	PG_5_SPREAD_WIDE_AIMED,
	PG_5_SPREAD_NARROW_AIMED,

	// -11.25 deg to +11.25 deg, around the player
	PG_1_RANDOM_NARROW_AIMED = 29,
	// -45 deg to +45 deg, facing down
	PG_1_RANDOM_WIDE = 30,

	_pellet_group_t_FORCE_INT16 = 0x7FFF
};

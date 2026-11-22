void elis_setup(void)
{
	ent_still_or_wave.pos_set(BASE_LEFT, BASE_TOP);

	// ZUN bloat: These two are redundant, as they're synced with
	// [ent_still_or_wave] before first use anyway.
	ent_attack.pos_set(BASE_LEFT, BASE_TOP);
	ent_bat.pos_set(BASE_LEFT, BASE_TOP);

	ent_still_or_wave.hitbox_orb_set(
		((GIRL_W / 8) * 1), -4,
		((GIRL_W / 8) * 7), ((GIRL_H / 6) * 5)
	);
	// Note that [ent_attack] doesn't receive a hitbox!
	ent_bat.hitbox_orb_set(-8, -8, (BAT_W + 8), (BAT_H + 8));

	boss_phase = 0;
	boss_phase_frame = 0;

	// Same HP and phase settings as Kikuri.
	boss_hp = HP_TOTAL;
	hud_hp_first_white = HP_PHASE_1_END;
	hud_hp_first_redwhite = HP_PHASE_3_END;

	random_seed = frame_rand;
}


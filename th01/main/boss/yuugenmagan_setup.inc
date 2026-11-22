void yuugenmagan_setup(void)
{
	eye_west     .set_image(C_HIDDEN);
	eye_east     .set_image(C_HIDDEN);
	eye_southwest.set_image(C_HIDDEN);
	eye_southeast.set_image(C_HIDDEN);
	eye_north    .set_image(C_HIDDEN);

	// These exactly correspond to the yellow boxes in BOSS2.GRP.
	eye_west     .pos_set(     EYE_WEST_LEFT, EYE_LATERAL_TOP);
	eye_east     .pos_set(     EYE_EAST_LEFT, EYE_LATERAL_TOP);
	eye_southwest.pos_set(EYE_SOUTHWEST_LEFT, EYE_SOUTH_TOP);
	eye_southeast.pos_set(EYE_SOUTHEAST_LEFT, EYE_SOUTH_TOP);
	eye_north    .pos_set(    EYE_NORTH_LEFT, EYE_NORTH_TOP);

	eye_west     .hitbox_orb_set(-4, -4, (EYE_W + 4), EYE_H);
	eye_east     .hitbox_orb_set(-4, -4, (EYE_W + 4), EYE_H);
	eye_southwest.hitbox_orb_set(-4, -4, (EYE_W + 4), EYE_H);
	eye_southeast.hitbox_orb_set(-4, -4, (EYE_W + 4), EYE_H);
	eye_north    .hitbox_orb_set(-4, -4, (EYE_W + 4), EYE_H);
	eye_west     .hitbox_orb_deactivate();
	eye_east     .hitbox_orb_deactivate();
	eye_southwest.hitbox_orb_deactivate();
	eye_southeast.hitbox_orb_deactivate();
	eye_north    .hitbox_orb_deactivate();

	boss_phase_frame = 0;
	boss_phase = 0;
	boss_hp = HP_TOTAL;
	hud_hp_first_white = HP_PHASE_3_END;
	hud_hp_first_redwhite = HP_PHASE_7_END;
}


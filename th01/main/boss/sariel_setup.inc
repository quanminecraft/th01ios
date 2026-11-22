void sariel_setup(void)
{
	ent_shield.pos_set(SHIELD_LEFT, SHIELD_TOP, 0, RES_X, 0, RES_Y);
	ent_shield.hitbox_orb_set(-16, -16, 64, 64);
	boss_hp = 18;
	hud_hp_first_white = 8;
	hud_hp_first_redwhite = 2;
	boss_phase = 0;
	boss_phase_frame = 0;
	ent_shield.hitbox_orb_inactive = false;
	anm_dress.left = DRESS_LEFT;
	anm_dress.top = DRESS_TOP;
	anm_wand.left = WAND_LEFT;
	anm_wand.top = WAND_TOP;
}


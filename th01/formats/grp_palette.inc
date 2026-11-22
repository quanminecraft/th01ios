void grp_palette_settone(int tone)
{
	if(tone < 0) {
		tone = 0;
	} else if(tone > 200) {
		tone = 200;
	}
	for(vc_t col = 1; col < COLOR_COUNT; col++) {
		for(int comp = 0; comp < COMPONENT_COUNT; comp++) {
			int blend;
			if(tone > 100) {
				blend = (RGB4::max() - grp_palette[col].v[comp]);
				blend *= (tone - 100);
				blend /= 100;
				z_Palettes[col].v[comp] = (grp_palette[col].v[comp] + blend);
			} else {
				blend = grp_palette[col].v[comp];
				blend *= (100 - tone);
				blend /= 100;
				z_Palettes[col].v[comp] = (grp_palette[col].v[comp] - blend);
			}
		}
	}
	grp_palette_tone = tone;
	z_palette_set_all_show(z_Palettes);
}

void grp_palette_inout(
	int tone_start, int speed, unsigned int frame_delay_per_step
)
{
	int tone = tone_start;
	for(int i = 0; i < (100 / TONE_STEP_PER_FRAME); i++) {
		tone += (TONE_STEP_PER_FRAME * speed);
		grp_palette_settone(tone);
		frame_delay(frame_delay_per_step);
	}
}

void grp_palette_black_out(unsigned int frame_delay_per_step)
{
	grp_palette_inout(100, -1, frame_delay_per_step);
}

void grp_palette_black_in(unsigned int frame_delay_per_step)
{
	grp_palette_inout(  0, +1, frame_delay_per_step);
}

void grp_palette_white_out(unsigned int frame_delay_per_step)
{
	grp_palette_inout(100, +1, frame_delay_per_step);
}

void grp_palette_white_in(unsigned int frame_delay_per_step)
{
	grp_palette_inout(200, -1, frame_delay_per_step);
}

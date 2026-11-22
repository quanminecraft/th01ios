#ifndef TH01_NATIVE_ORB_PHYSICS
#define yuugenmagan_component(color, index) (color).v[index]
#endif
void eyes_toggle_and_yokoshima_recolor(
	eye_flag_t eyes_to_close,
	eye_flag_t eyes_to_open,
	int yokoshima_comp_dec,
	int yokoshima_comp_inc,
	yuugenmagan_int_t& frame
)
{
	#define eye_should_be_closed(bit) ( \
		eyes_to_close & bit \
	)

	#define eye_should_be_opened(bit) ( \
		eyes_to_open & bit \
	)

	#define eye_is_toggled(bit) ( \
		((eyes_to_close & bit) == bit) || ((eyes_to_open & bit) == bit) \
	)

	eyes_foreach_if(eye_is_toggled, locked_unput_and_put_8);
	frame++;

	// ZUN quirk: That's a color change on every frame *except* the 10th one...
	if((frame % 10) != 0) {
		if(yokoshima_comp_dec < COMPONENT_COUNT) {
			// This clamping condition is actually rather important to keep
			// [stage_palette] from running into negative numbers.
			if(yuugenmagan_component(z_Palettes[COL_YOKOSHIMA], yokoshima_comp_dec) > 0) {
				yuugenmagan_component(z_Palettes[COL_YOKOSHIMA], yokoshima_comp_dec)--; // ZUN bloat
				yuugenmagan_component(stage_palette[COL_YOKOSHIMA], yokoshima_comp_dec)--;
			}
		}
		if(yokoshima_comp_inc < COMPONENT_COUNT) {
			// ZUN quirk: No equivalent clamping condition here. The final
			// color in [z_Palettes] is clamped to 0xF, but there's nothing to
			// prevent the [stage_palette] version from running past 0xF.
			// If a component that was previously incremented like this is then
			// decremented in subsequent recoloring operations, this overflow
			// manifests itself as a noticeable delay: The component will spend
			// most of the recoloring time above 0xF, and only fall back into
			// the valid color range at the *end* of the recoloring phase. If
			// the previously component additionally started out with a non-0x0
			// value (which is the case for the blue component in the original
			// game), decrementing it for a constant amount of frames can only
			// ever reach that brighter initial value again.
			yuugenmagan_component(z_Palettes[COL_YOKOSHIMA], yokoshima_comp_inc)++; // ZUN bloat
			yuugenmagan_component(stage_palette[COL_YOKOSHIMA], yokoshima_comp_inc)++;
		}

		// ZUN bloat: z_palette_set_show(COL_YOKOSHIMA) is enough.
		z_palette_set_all_show(stage_palette);
	}

	// Matches the animation in phase_0_eye_open().
	if(frame == ((EYE_OPENING_FRAMES / 3) * 1)) {
		eyes_set_image(eyes_to_close, C_HALFOPEN);
		eyes_set_image(eyes_to_open, C_CLOSED);
		eyes_foreach_if(eye_should_be_opened, hitbox_orb_activate);
	} else if(frame == ((EYE_OPENING_FRAMES / 3) * 2)) {
		eyes_set_image(eyes_to_close, C_CLOSED);
		eyes_set_image(eyes_to_open, C_HALFOPEN);
		eyes_foreach_if(eye_should_be_closed, hitbox_orb_deactivate);
	} else if(frame == ((EYE_OPENING_FRAMES / 3) * 3)) {
		eyes_set_image(eyes_to_close, C_HIDDEN);
		eyes_set_image(eyes_to_open, C_AHEAD);
	}

	#undef eye_should_be_closed
	#undef eye_should_be_opened
	#undef eye_is_toggled
}

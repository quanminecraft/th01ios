enum menu_id_t {
	MID_MAIN,
	MID_OPTION,
	MID_MUSIC,
	MID_UPDATE_BGM_MODE__DELAY__SWITCH_TO_MAIN,
	MID_DELAY__SWITCH_TO_OPTION,
};

static const int MAIN_CHOICE_COUNT = 4;
static const int OPTION_CHOICE_COUNT = 5;
static const int MUSIC_CHOICE_COUNT = 2;

// ZUN bloat: Worse versions of the same macros in `clamp.hpp`, replacing `++`
// and `--` with `+= 1` and `-= 1`.
#define ring_inc_one(val, ring_end) \
	(val) += 1; \
	if((val) > (ring_end)) { \
		(val) = 0; \
	}

#define ring_dec_one(val, ring_end) \
	(val) -= 1; \
	if((val) < 0) { \
		(val) = ring_end; \
	}

#define input_update_bool(var, cur_sensed) \
	if(cur_sensed) input_func_bool(var)

#define input_onchange_ring(prev_slot, cur_sensed, ring_func) \
	if(cur_sensed) { \
		if(!input_prev[prev_slot]) { \
			ring_func; \
			input_prev[prev_slot] = true; \
		} \
	} else { \
		input_prev[prev_slot] = false; \
	}

int8_t menu_sel = 0;
int8_t input_left = false; // ACTUAL TYPE: bool
bool input_cancel = false;

int8_t menu_id = MID_MAIN; // ACTUAL TYPE: menu_id_t

int8_t input_right = false; // ACTUAL TYPE: bool
bool quit = false;
int8_t unused_con_arg_0 = 0;
int8_t unused_con_arg_2 = 0;
int32_t unused_con_arg_1 = 0;

inline void keygroup_sense(REGS& out, REGS& in, char group_id) {
	in.h.ah = 0x04;
	in.h.al = group_id;
	int86(0x18, &in, &out);
}

inline void ok_shot_cancel_sense(REGS& out, REGS& in) {
	keygroup_sense(out, in, 3);
	input_update_bool(input_ok, (out.h.ah & K3_RETURN));

	keygroup_sense(out, in, 5);
	input_update_bool(input_shot, (out.h.ah & K5_Z));

	keygroup_sense(out, in, 0);
	input_update_bool(input_cancel, (out.h.ah & K0_ESC));
}

void main_input_sense(void)
{
	REGS in;
	REGS out1, out2;
	OP_INPUT_PREV;

	keygroup_sense(out1, in, 7);
	keygroup_sense(out2, in, 8);

	input_onchange_ring(0,
		(out1.h.ah & K7_ARROW_UP) || (out2.h.ah & K8_NUM_8),
		ring_dec_one(menu_sel, (MAIN_CHOICE_COUNT - 1))
	);

	keygroup_sense(out2, in, 9);

	input_onchange_ring(1,
		(out1.h.ah & K7_ARROW_DOWN) || (out2.h.ah & K9_NUM_2),
		ring_inc_one(menu_sel, (MAIN_CHOICE_COUNT - 1))
	);

	ok_shot_cancel_sense(out1, in);
}

uint8_t option_choice_max = 3;

void option_input_sense(void)
{
	REGS in;
	REGS out1, out2;
	OP_INPUT_PREV;

	keygroup_sense(out1, in, 7);
	keygroup_sense(out2, in, 8);

	input_onchange_ring(0,
		(out1.h.ah & K7_ARROW_UP) || (out2.h.ah & K8_NUM_8),
		ring_dec_one(menu_sel, option_choice_max)
	);
	input_update_bool(
		input_left, (out1.h.ah & K7_ARROW_LEFT) || (out2.h.ah & K8_NUM_4)
	);

	keygroup_sense(out2, in, 9);

	input_onchange_ring(1,
		(out1.h.ah & K7_ARROW_DOWN) || (out2.h.ah & K9_NUM_2),
		ring_inc_one(menu_sel, option_choice_max)
	);
	input_update_bool(
		input_right, (out1.h.ah & K7_ARROW_RIGHT) || (out2.h.ah & K9_NUM_6)
	);

	ok_shot_cancel_sense(out1, in);
}
/// -----


#include <stddef.h>
#include "platform/x86real/pc98/page.hpp"
#include "libs/master.lib/pc98_gfx.hpp"
#include "th01/rank.h"
#include "th01/resident.hpp"
#include "th01/hardware/egc.h"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/input.hpp"
#include "th01/hardware/graph.h"
#include "th01/hardware/grppsafx.h"
#include "th01/hardware/grp_text.hpp"
#include "th01/hardware/grppsafx.h"
#include "th01/hardware/palette.h"
#include "th01/snd/mdrv2.h"
#include "th01/formats/grp.h"
#include "th01/formats/scoredat.hpp"
#include "th01/hiscore/regist.hpp"
#include "th01/end/end.hpp"
#include "th01/end/pic.hpp"
#include "th01/shiftjis/end.hpp"
#include "th01/shiftjis/scoredat.hpp"
#include "th01/shiftjis/title.hpp"

// Pictures
// --------

static const pixel_t PIC_W = (RES_X / 2);
static const pixel_t PIC_H = (RES_Y / 2);
static const screen_x_t PIC_LEFT   = ((RES_X / 2) - (PIC_W / 2));
static const screen_y_t PIC_TOP    = ((RES_Y / 2) - (PIC_H / 2));
static const screen_y_t PIC_RIGHT  = (PIC_LEFT + PIC_W);
static const screen_y_t PIC_BOTTOM = (PIC_TOP + PIC_H);

static const pixel_t PIC_VRAM_W = (PIC_W / BYTE_DOTS);

void end_pics_load_palette_show(const char *fn)
{
	page_access(1);
	grp_put(fn, GPF_PALETTE_SHOW);
}

#include "th01/end/pic_show.inc"

#define end_pic_show_and_delay(quarter, delay_frames) { \
	end_pic_show(quarter); \
	frame_delay(delay_frames); \
}

#define end_pic_white_in_and_delay(quarter, white_in_speed, delay_frames) { \
	end_pic_show(quarter); \
	grp_palette_white_in(white_in_speed); \
	frame_delay(delay_frames); \
}
// --------

// Text typing
// -----------
// Types [len] half- (ank) or full-width (kanji) characters of [str] onto the
// given position in VRAM, with a frame delay between each character.

#include "th01/end/text_type.inc"

/// Endings
/// -------

static const int FINAL_DELAY_FRAMES = 300;

// Function ordering fails
// -----------------------

// Resets all end-related data in the resident structure. Always returns true.
bool16 end_resident_clear(void);

void near end_good(void);
void near end_bad(void);
void near end_good_makai(void);
void near end_good_jigoku(void);

// Shows the route-specific boss slideshow.
void near boss_slides_animate(void);

// Shows the verdict screen, then calls regist_menu() with the score and the
// cleared route.
void verdict_animate_and_regist_menu(void);
// -----------------------

#include "th01/end/sequence.inc"

/// Verdict screen
/// --------------

#include "th01/end/verdict.inc"

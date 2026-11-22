#pragma option -zCPTN_GRP_GRZ

#include "libs/master.lib/master.hpp"
#include "libs/piloadc/piloadm.hpp"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/palette.h"
#include "th01/formats/grp.h"
#include "x86real.h"

// The same size that master.lib uses in graph_pi_load_pack(), with no
// explanation for it given in either master.lib or PILOADC.DOC...
#define GRP_BUFFER_SIZE 0x4000

// Palette
// -------

static const int TONE_STEP_PER_FRAME = 5;

Palette4 grp_palette;
static int grp_palette_tone = 100;

#include "th01/formats/grp_palette.inc"
// -------

int grp_put(const char *fn, grp_put_flag_t flag)
{
	static int8_t* grp_buf;
	int option = 0;
	int ret;
	const int8_t *grp_buf_aligned;

	grp_buf = new int8_t[GRP_BUFFER_SIZE];
	if(flag & GPF_COLORKEY) {
		option = PILOAD_OPT_COLORKEY(15);
	}
	if(!file_ropen(fn)) {
		ret = FileNotFound;
		goto err;
	}
	ret = PiBlitL(grp_buf, GRP_BUFFER_SIZE, 0, 0, option, file_read);
	file_close();

	// Pulling the palette out of the buffer this way might look like reliance
	// on implementation details, but it is, in fact, officially sanctioned
	// by PILOADC.DOC. Slightly ugly how we have also to replicate PiLoad's
	// paragraph rounding of the address, though...
	grp_buf_aligned = reinterpret_cast<const char *>(MK_FP(
		FP_SEG(grp_buf), ((FP_OFF(grp_buf) + 15) & ~15)
	));
	grp_palette = *reinterpret_cast<const Palette4 *>(grp_buf_aligned + 0x100);

	if(flag & GPF_PALETTE_SHOW) {
		if(grp_palette_tone != 100) {
			grp_palette_settone(grp_palette_tone);
		} else {
			z_palette_set_all_show(grp_palette);
		}
	}
err:
	delete[] grp_buf;
	return ret;
}

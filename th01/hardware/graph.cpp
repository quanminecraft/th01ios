#include <dos.h>
#include <mem.h>
#include <mbctype.h>
#include <mbstring.h>
#include "platform/x86real/pc98/page.hpp"
#include "th01/v_colors.hpp"
#include "th01/math/clamp.hpp"
#include "th01/hardware/egc.h"
#include "th01/hardware/grcg.hpp"
#include "th01/hardware/vsync.hpp"
#include "th01/hardware/graph.h"
#include "th01/hardware/palette.h"

// Never read from, so it's supposedly only there for debugging purposes?
static screen_point_t graph_r_last_line_end;

/// VRAM plane "structures"
/// -----------------------
#define Planes_declare(var) \
	dots8_t far *var##_B = reinterpret_cast<dots8_t __seg *>(SEG_PLANE_B); \
	dots8_t far *var##_R = reinterpret_cast<dots8_t __seg *>(SEG_PLANE_R); \
	dots8_t far *var##_G = reinterpret_cast<dots8_t __seg *>(SEG_PLANE_G); \
	dots8_t far *var##_E = reinterpret_cast<dots8_t __seg *>(SEG_PLANE_E);

#define Planes_next_row(var) \
	var##_B += ROW_SIZE; \
	var##_R += ROW_SIZE; \
	var##_G += ROW_SIZE; \
	var##_E += ROW_SIZE;

#define Planes_offset(var, x, y) \
	var##_B += vram_offset_divmul(x, y); \
	var##_R += vram_offset_divmul(x, y); \
	var##_G += vram_offset_divmul(x, y); \
	var##_E += vram_offset_divmul(x, y);

#define PlanarRow_declare(var) \
	dots8_t var##_B[ROW_SIZE]; \
	dots8_t var##_R[ROW_SIZE]; \
	dots8_t var##_G[ROW_SIZE]; \
	dots8_t var##_E[ROW_SIZE]; \

#define PlanarRow_blit(dst, src, bytes) \
	memcpy(dst##_B, src##_B, bytes); \
	memcpy(dst##_R, src##_R, bytes); \
	memcpy(dst##_G, src##_G, bytes); \
	memcpy(dst##_E, src##_E, bytes);
/// -----------------------

/// Clipping
/// --------

#define fix_order(tmp, low, high) \
	if(low > high) { \
		tmp = low; \
		low = high; \
		high = tmp; \
	}

#define clip_min(low, high, minimum) \
	if(low < minimum) { \
		if(high < minimum) { \
			return; \
		} \
		low = minimum; \
	}

#define clip_max(low, high, maximum) \
	if(high > maximum) { \
		if(low > maximum) { \
			return; \
		} \
		high = maximum; \
	}

#define clip_x(left, right) \
	clip_min(left, right, 0); \
	clip_max(left, right, (RES_X - 1));

#define clip_y(top, bottom) \
	clip_min(top, bottom, 0); \
	clip_max(top, bottom, (RES_Y - 1));
/// --------

/// BIOS
/// ----

inline void graph_access_and_show_0() {
	page_access(0);
	page_show(0);
}

inline void z_graph_400line() {
	REGS regs;

	z_graph_hide();

	// 640x400
	regs.h.ah = 0x42;
	regs.h.ch = 0xC0;
	int86(0x18, &regs, &regs);

	// 16-color, analog mode
	outportb(0x6A, 1);
}

void z_graph_init()
{
	z_graph_400line();
	z_palette_set_all_show(z_Palettes);
	graph_access_and_show_0();
	z_graph_clear_0();
	grcg_off();
	z_graph_show();
}

void z_graph_exit()
{
	z_palette_black();
	z_graph_clear_0();
	graph_access_and_show_0();
	z_graph_show();
	grcg_off();
}

void z_graph_show()
{
	REGS regs;
	regs.h.ah = 0x40;
	int86(0x18, &regs, &regs);
}

void z_graph_hide()
{
	REGS regs;
	regs.h.ah = 0x41;
	int86(0x18, &regs, &regs);
}
/// ----

/// Hardware
/// --------

void z_palette_show_single(vc2 col, svc_comp2 r, svc_comp2 g, svc_comp2 b)
{
	outportb(0xA8, col);
	outportb(0xAA, g);
	outportb(0xAC, r);
	outportb(0xAE, b);
}

#define grcg_setcolor(mode, col) \
	outportb(0x7C, mode); \
	outportb(0x7E, (col & 1) ? 0xFF : 0x00); \
	outportb(0x7E, (col & 2) ? 0xFF : 0x00); \
	outportb(0x7E, (col & 4) ? 0xFF : 0x00); \
	outportb(0x7E, (col & 8) ? 0xFF : 0x00);

void grcg_setcolor_rmw(vc2 col)
{
	grcg_setcolor(0xC0, col);
}

void grcg_setcolor_tcr(vc2 col)
{
	grcg_setcolor(0x80, col);
}

void grcg_off_func(void)
{
	grcg_off();
}
/// --------

/// Palette
/// -------

Palette4 z_Palettes = {
	// These match the Z_ATRB_* bits, interestingly?
	0x0, 0x0, 0x0, //  0: Black
	0x0, 0x0, 0xF, //  1: Blue
	0x0, 0xF, 0x0, //  2: Green
	0x0, 0xF, 0xF, //  3: Cyan
	0xF, 0x0, 0x0, //  4: Red
	0xF, 0x0, 0xF, //  5: Magenta
	0xF, 0xF, 0x0, //  6: Yellow
	0xF, 0xF, 0xF, //  7: White

	0x8, 0x8, 0x8, //  8: Gray
	0x0, 0x0, 0xA, //  9: Dark blue
	0x0, 0xA, 0x0, // 10: Dark green
	0x0, 0xA, 0xA, // 11: Dark cyan
	0xA, 0x0, 0x0, // 12: Dark red
	0xA, 0x0, 0xA, // 13: Purple
	0xA, 0xA, 0x0, // 14: Gold
	0xC, 0xC, 0xC, // 15: Silver
};

void z_palette_set_all_show(const Palette4& pal)
{
	for(int i = 0; i < COLOR_COUNT; i++) {
		z_palette_set_show(i, pal[i].c.r, pal[i].c.g, pal[i].c.b);
	}
}

void z_palette_set_show(vc2 col, svc_comp2 r, svc_comp2 g, svc_comp2 b)
{
	r = clamp_min(clamp_max(r, RGB4::max()), RGB4::min());
	g = clamp_min(clamp_max(g, RGB4::max()), RGB4::min());
	b = clamp_min(clamp_max(b, RGB4::max()), RGB4::min());

	z_Palettes[col].c.r = r;
	z_Palettes[col].c.g = g;
	z_Palettes[col].c.b = b;
	z_palette_show_single(col, r, g, b);
}
/// -------

/// Whole-page functions
/// --------------------
void z_graph_clear()
{
	dots8_t far *plane = reinterpret_cast<dots8_t __seg *>(SEG_PLANE_B);

	grcg_setcolor_rmw(0);
	memset(plane, 0xFF, PLANE_SIZE);
	grcg_off_func();
}

void z_graph_clear_0(void)
{
	// Yes, page 2, twice. Which effectively is the same as page 0... at least
	// according to any real hardware and emulator tests I could come up with.
	page_access(2);	z_graph_clear();
	page_access(2);	z_graph_clear();
}

void graph_copy_page_to_other(page_t src)
{
	PlanarRow_declare(tmp);
	Planes_declare(p);
	page_t dst = (src ^ 1);

	for(screen_y_t y = 0; y < RES_Y; y++) {
		page_access(src);	PlanarRow_blit(tmp, p, ROW_SIZE);
		page_access(dst);	PlanarRow_blit(p, tmp, ROW_SIZE);
		Planes_next_row(p);
	}
}
/// --------------------

/// Palette fades
/// -------------

#define FADE_DELAY 10
#define fade_loop(pal, per_comp) \
	for(int i = 0; i < pal.range(); i++) { \
		z_vsync_wait(); \
		for(svc2 col = 0; col < COLOR_COUNT; col++) { \
			for(int comp = 0; comp < COMPONENT_COUNT; comp++) { \
				per_comp; \
			} \
			z_palette_show_single_col(col, pal[col]); \
		} \
		delay(FADE_DELAY); \
	}

void z_palette_black(void)
{
	for(svc2 col = 0; col < COLOR_COUNT; col++) {
		z_palette_show_single(col, 0, 0, 0);
	}
}

void z_palette_black_in(void)
{
	Palette4 fadepal;
	memset(&fadepal, 0, sizeof(Palette4));
	fade_loop(fadepal,
		if(fadepal[col].v[comp] < z_Palettes[col].v[comp]) {
			fadepal[col].v[comp]++;
		}
	);
}

void z_palette_black_out(void)
{
	Palette4 fadepal;
	memcpy(&fadepal, &z_Palettes, sizeof(Palette4));

	fade_loop(fadepal,
		if(fadepal[col].v[comp] > fadepal[0].min()) {
			fadepal[col].v[comp]--;
		}
	);
}

void z_palette_white_in(void)
{
	Palette4 fadepal;
	memset(&fadepal, fadepal[0].max(), sizeof(Palette4));

	fade_loop(fadepal,
		if(fadepal[col].v[comp] > z_Palettes[col].v[comp]) {
			fadepal[col].v[comp]--;
		}
	);
}
/// -------------

/// Restorable line drawing
/// -----------------------

// `true` copies the pixels to be drawn from the same position on page 1, thus
// restoring them with the background image. `false` (the default) draws them
// regularly in the given [col].
static bool graph_r_unput = false;

// Not used for purely horizontal lines.
static dots16_t graph_r_pattern = 0x80; // 1 pixel (*       )

void graph_r_hline(screen_x_t left, screen_x_t right, vram_y_t y, vc2 col)
{
	vram_byte_amount_t x;
	vram_byte_amount_t full_bytes_to_put;
	int order_tmp;
	dots8_t left_pixels;
	dots8_t right_pixels;
	dots8_t *vram_row;

	fix_order(order_tmp, left, right);
	clip_x(left, right);

	graph_r_last_line_end.x = right;
	graph_r_last_line_end.y = y;

	vram_row = (dots8_t *)(MK_FP(SEG_PLANE_B, vram_offset_muldiv(left, y)));
	full_bytes_to_put = (right / BYTE_DOTS) - (left / BYTE_DOTS);
	left_pixels = 0xFF >> (left & BYTE_MASK);
	right_pixels = 0xFF << (BYTE_MASK - (right & BYTE_MASK));

	if(!graph_r_unput) {
		grcg_setcolor_rmw(col);
	}
	if(graph_r_unput) {
		egc_copy_rect_1_to_0_16(left, y, RES_X - left, 1);
	} else {
		if(full_bytes_to_put == 0) {
			vram_row[0] = (left_pixels & right_pixels);
		} else {
			vram_row[0] = left_pixels;
			for(x = 1; x < full_bytes_to_put; x++) {
				vram_row[x] = 0xFF;
			}
			vram_row[full_bytes_to_put] = right_pixels;
		}
	}
	if(!graph_r_unput) {
		grcg_off_func();
	}
}

void graph_r_vline(screen_x_t x, vram_y_t top, vram_y_t bottom, vc2 col)
{
	vram_y_t y;
	int order_tmp;
	dots16_t pattern;
	vram_offset_t vram_row_offset;

	fix_order(order_tmp, top, bottom);
	clip_y(top, bottom);

	graph_r_last_line_end.x = x;
	graph_r_last_line_end.y = bottom;

	if(graph_r_unput) {
		egc_copy_rect_1_to_0_16(
			x, top, (sizeof(pattern) * BYTE_DOTS), (bottom - top)
		);
		return;
	}
	vram_row_offset = vram_offset_shift(x, top);
	pattern = graph_r_pattern >> (x & BYTE_MASK);
	pattern |= graph_r_pattern << (16 - (x & BYTE_MASK));

	grcg_setcolor_rmw(col);
	for(y = top; y <= bottom; y++) {
		grcg_put(vram_row_offset, pattern, 16);
		vram_row_offset += ROW_SIZE;
	}
	grcg_off_func();
}

void graph_r_line_unput(
	screen_x_t left, vram_y_t top, screen_x_t right, vram_y_t bottom
)
{
	graph_r_unput = true;
	graph_r_line(left, top, right, bottom, V_WHITE);
	graph_r_unput = false;
}

void graph_r_line_patterned(
	screen_x_t left,
	vram_y_t top,
	screen_x_t right,
	vram_y_t bottom,
	vc2 col,
	dots16_t pattern
)
{
	graph_r_pattern = pattern;
	graph_r_line(left, top, right, bottom, col);
	graph_r_pattern = 0x80;
}

#include "th01/hardware/graph_r_line.inc"

void graph_r_lineloop_put(const screen_point_t point[], int count, vc_t col)
{
	int i = 0;
	while((count - 1) > i) {
		graph_r_line(
			point[i].x, point[i].y, point[i + 1].x, point[i + 1].y, col
		);
		i++;
	}
	graph_r_line(point[i].x, point[i].y, point[0].x, point[0].y, col);
}

void graph_r_lineloop_unput(const screen_point_t point[], int count)
{
	int i = 0;
	while((count - 1) > i) {
		graph_r_line_unput(
			point[i].x, point[i].y, point[i + 1].x, point[i + 1].y
		);
		i++;
	}
	graph_r_line_unput(point[i].x, point[i].y, point[0].x, point[0].y);
}

/// -----------------------

void graph_move_byterect_interpage(
	screen_x_t src_left,
	vram_y_t src_top,
	screen_x_t src_right,
	vram_y_t src_bottom,
	screen_x_t dst_left,
	vram_y_t dst_top,
	page_t src
)
{
	pixel_t w = (src_right - src_left) / BYTE_DOTS;
	pixel_t h = (src_bottom - src_top);
	Planes_declare(src);
	Planes_declare(dst);
	pixel_t row;
	PlanarRow_declare(tmp);

	Planes_offset(src, src_left, src_top);
	Planes_offset(dst, dst_left, dst_top);

	page_t dst = (src ^ 1);
	for(row = 0; row < h; row++) {
		page_access(src);	PlanarRow_blit(tmp, src, w);
		page_access(dst);	PlanarRow_blit(dst, tmp, w);
		Planes_next_row(src);
		Planes_next_row(dst);
	}
}

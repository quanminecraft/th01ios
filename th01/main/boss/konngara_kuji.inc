enum kuji_flash_color_t {
	BLACK,
	WHITE
};

inline void kuji_flash(kuji_flash_color_t color) {
	if(color == BLACK) {
		z_palette_set_show(0, 0x0, 0x0, 0x0);
	} else {
		z_palette_set_show(0, 0xF, 0xF, 0xF);
	}
}

inline void kuji_put(kuji_flash_color_t flash_color, int image) {
	if(image == 7) {
		kuji_flash(flash_color);
		grz_load_single(0, GRZ_FN, image);
	} else {
		z_graph_clear();
		grz_load_single(0, GRZ_FN, image);
		kuji_flash(flash_color);
	}
	grx_put(0);
	if(image == 15) {
		grx_free(0);
	}
	frame_delay(10);
}


void item_unput_update_render(
	int slot,
	char &flag,
	hittest_func_t &hittest,
	screen_x_t &left,
	screen_y_t &top,
	pixel_t &velocity_y,
	unsigned char &splash_radius,
	main_ptn_id_t ptn_id,
	collect_update_and_render_func_t &collect_update_and_render,
	drop_func_t &on_drop
)
{
	hittest(slot);

	if(flag == IF_SPLASH) {
		#define circle_x (left + (ITEM_W / 2))
		#define circle_y (top + (ITEM_H / 2))

		shape_circle_sloppy_unput(circle_x, circle_y, splash_radius, 0x08);
		splash_radius += 4;
		if(splash_radius >= 48) {
			flag = IF_FALL;
			return;
		}
		shape_circle_put(circle_x, circle_y, splash_radius, V_WHITE, 0x08);

		#undef circle_y
		#undef circle_x
	} else if(flag == IF_FALL) {
		ptn_unput_8(left, top, ptn_id);
		top += velocity_y;
		if(top >= (PLAYFIELD_BOTTOM - ITEM_H)) {
			velocity_y = -8;
			flag = IF_BOUNCE;
			goto bounce_update;
		}
		ptn_put_8(left, top, ptn_id);
	} else if(flag == IF_BOUNCE) {
		ptn_unput_8(left, top, ptn_id);

bounce_update:
		top += velocity_y;
		velocity_y++;
		if(top >= PLAYFIELD_BOTTOM) {
			flag = IF_FREE;
			on_drop();
			return;
		}
		ptn_put_8(left, top, ptn_id);
	} else if(flag >= IF_COLLECTED) {
		collect_update_and_render(slot);
	}
}


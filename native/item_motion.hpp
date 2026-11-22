#pragma once
#include "item_backend.hpp"
typedef void hittest_func_t(int slot);
typedef void collect_update_and_render_func_t(int slot);
typedef void drop_func_t(void);
void item_unput_update_render(int slot,char& flag,hittest_func_t& hittest,
	screen_x_t& left,screen_y_t& top,pixel_t& velocity_y,unsigned char& splash_radius,
	main_ptn_id_t ptn_id,collect_update_and_render_func_t& collect,drop_func_t& drop);
namespace th01_native {
void item_splash_put(screen_x_t center_x,screen_y_t center_y,pixel_t radius,unsigned color,unsigned char angle_step);
void item_splash_unput(screen_x_t center_x,screen_y_t center_y,pixel_t radius,unsigned char angle_step);
}

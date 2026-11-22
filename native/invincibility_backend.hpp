#pragma once
#include "player_backend.hpp"
#include "th01/main/player/player.hpp"
#include "th01/sprites/shape8x8.hpp"
void shape8x8_invincibility_put(screen_x_t left,vram_y_t top,int cel);
#define shape8x8_sloppy_unput(left,top) egc_copy_rect_1_to_0_16_word_w(left,top,8,8)

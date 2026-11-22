#pragma once
#include "player_backend.hpp"
#include "graphics_defs.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/player/orb.hpp"
#include "th01/main/player/shot.hpp"
#include "th01/main/bullet/pellet.hpp"
#include "th01/sprites/pellet.h"
#include "th01/math/polar.hpp"
#include "th01/math/vector.hpp"
#include "th01/math/overlap.hpp"
#include "th01/rank.h"
#include "th01/main/bullet/pellet_s.hpp"
extern std::int16_t native_pellet_speed;
void native_pellet_put(screen_x_t left,screen_y_t top,int cel);
void pellet_cloud_put_8(screen_x_t left,screen_y_t top,unsigned color,int cel);
void pellet_cloud_unput_8(screen_x_t left,screen_y_t top,int cel);

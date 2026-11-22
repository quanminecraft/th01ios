#pragma once
#include "player_backend.hpp"
#include "stage.hpp"
#include "font_rom.hpp"
#include "graphics_defs.hpp"
#include "th01/sprites/pellet.h"
#include "th01/math/digit.hpp"
#include "th01/math/subpixel.hpp"
#include "th01/main/bullet/pellet_group.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/player/orb.hpp"
#include "th01/main/stage/card.hpp"
#include "th01/rank.h"

constexpr int STAGEOBJ_ORB_DISTANCE_X=(32*3/8)+(25/2);
constexpr int STAGEOBJ_ORB_DISTANCE_Y=(32*3/8)+(25/2);
constexpr std::uint32_t CARD_SCORE_CAP=25600;
enum native_card_flag_t {CARD_ALIVE=0,CARD_FLIPPING=1,CARD_REMOVED=2};
enum native_card_cel_t {CARD_CEL_NOT_FLIPPING=0,CARD_CEL_FRONT_HALF,CARD_CEL_EDGE,CARD_CEL_BACK_HALF,CARD_CEL_FLIPPED,CARD_CELS};
constexpr int CARD_FRAMES_PER_CEL=6,CARD_HP_MAX=5,STAGEOBJ_W=32,STAGEOBJ_H=32;
constexpr int GLYPH_HALF_W=8,GLYPH_H=16,FX_WEIGHT_BOLD=0x20;
inline int card_first_frame_of(int cel) {return cel*CARD_FRAMES_PER_CEL;}
namespace th01_native {
struct CardState {
	std::int16_t count=0;
	std::vector<std::int16_t> left,top;
	std::vector<std::int8_t> flag;
	std::vector<std::int8_t> hp;
	std::vector<std::int16_t> flip_frame;
};
struct CardRenderer {
	Framebuffer& framebuffer;
	const SpriteSet& sprites;
	const FontRom& font;
	std::vector<Sprite> backgrounds;
};
// Requests preserve gameplay order. Item movement and HUD painting are not
// silently simulated by this collision module; their consumers remain next.
struct CardEvents {
	struct Hud {std::int32_t score;std::int16_t combo,maximum;};
	struct Item {unsigned card;bool bomb;};
	struct Pellet {std::int16_t left,top;pellet_group_t group;subpixel_t speed;};
	std::vector<Hud> hud;
	std::vector<Item> items;
	std::vector<Pellet> pellets;
	std::function<void(unsigned,bool)> item_dispatch;
	std::function<void()> hud_dispatch;
	std::function<void(screen_x_t,screen_y_t,pellet_group_t,subpixel_t)> pellet_dispatch;
	CardRenderer* renderer=nullptr;
};
class CardBackendBinding {
	CardEvents* previous;
public:
	explicit CardBackendBinding(CardEvents& events);
	~CardBackendBinding();
	CardBackendBinding(const CardBackendBinding&)=delete;
	CardBackendBinding& operator=(const CardBackendBinding&)=delete;
};
void load_cards(const StageLayout& layout);
void capture_card_backgrounds(CardRenderer& renderer);
}
extern th01_native::CardState cards;
extern std::vector<std::uint32_t> cards_score;
extern std::int32_t score;
extern bool16 test_damage;
extern bool16 stage_cleared;
extern std::int8_t stage_num;
extern uint8_t CARD_ANIM[CARD_HP_MAX][CARD_CELS];
void native_emit_pellet_group(screen_x_t left,screen_y_t top,pellet_group_t group,subpixel_t speed);
void hud_score_and_cardcombo_render();
void items_point_add(unsigned card);
void items_bomb_add(unsigned card);
void page_access(unsigned page);
void stageobj_put_bg_and_obj_8(screen_x_t left,screen_y_t top,int id,int slot);
void egc_copy_rect_1_to_0_16_word_w(screen_x_t left,screen_y_t top,pixel_t width,pixel_t height);
void graph_putsa_fx(screen_x_t left,screen_y_t top,int16_t effect,const char* text);

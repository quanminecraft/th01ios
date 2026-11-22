#pragma once
#include "miss_backend.hpp"
#include "item_runtime.hpp"
#include "th01/main/player/gameover.hpp"
namespace th01_native {
struct GameOverBackend {std::int8_t& saved_bombs;};
class GameOverBinding {
	GameOverBackend* previous;
public:
	explicit GameOverBinding(GameOverBackend& backend);
	~GameOverBinding();
	GameOverBinding(const GameOverBinding&)=delete;
	GameOverBinding& operator=(const GameOverBinding&)=delete;
};
}
void native_gameover_save_bombs(int bombs);
inline void native_gameover_orb_put() {ptn_put_8(orb_cur_left,orb_cur_top,PTN_ORB);}
inline unsigned native_gameover_isqrt(unsigned value) {unsigned root=0;while(std::uint64_t(root+1)*(root+1)<=value) ++root;return root;}

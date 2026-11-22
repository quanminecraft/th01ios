#pragma once
#include "assets.hpp"
#include "th01/main/boss/entity_a.hpp"
namespace th01_native {
struct BossEntityBackend {
	Framebuffer& framebuffer;
	const std::vector<Entry>& archive;
	std::array<SpriteSet,4> slots;
    std::array<SpriteSet,2> animation_slots;
};
class BossEntityBinding {
	BossEntityBackend* previous;
public:
	explicit BossEntityBinding(BossEntityBackend& backend);
	~BossEntityBinding();
	BossEntityBinding(const BossEntityBinding&)=delete;
	BossEntityBinding& operator=(const BossEntityBinding&)=delete;
};
}

#pragma once
#include "assets.hpp"
#include "sprites.hpp"
#include <functional>
#include <memory>
namespace th01_native {
struct SarielBackgroundHooks {
 std::function<void()> vsync,hud;
 std::function<void(int)> hp;
 std::function<void(const Palette&)> stage_palette;
};
class SarielBackground {
 struct Impl;std::unique_ptr<Impl> impl;
public:
 SarielBackground(const std::filesystem::path& assets,unsigned target,int hp,SarielBackgroundHooks hooks);
 ~SarielBackground();
 SarielBackground(const SarielBackground&)=delete;SarielBackground& operator=(const SarielBackground&)=delete;
 bool advance();int frame()const;
};
}

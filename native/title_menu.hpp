#pragma once
#include "input_backend.hpp"
#include "font_rom.hpp"
#include "game_options.hpp"
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
namespace th01_native {
enum class TitleAction {menu,new_game,continue_game,quit,boss_practice};
struct TitleHost {
 std::function<SpriteSet(const char*)> load;
 std::function<void(const Framebuffer&,unsigned)> present_wait;
 std::function<void(unsigned)> delay_ms;
 std::function<void(const char*,const char*)> audio;
};
class TitleMenu {
 struct Impl;
 std::unique_ptr<Impl> impl;
public:
 TitleMenu(const FontRom& font,std::filesystem::path profile,TitleHost host,std::optional<GameOptions> remembered={});
 ~TitleMenu();
 TitleAction step(const InputKeys& keys);
 void resume_menu();
 const Framebuffer& framebuffer() const;
 GameOptions options() const;
 unsigned selection() const;
 unsigned menu_id() const;
};
}

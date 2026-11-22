#pragma once
#include "font_rom.hpp"
#include "input_backend.hpp"
#include "game_options.hpp"
namespace th01_native {
class ModeMenu {
 Framebuffer background,fb;
 const FontRom& font;
 GameMode selected=GameMode::standard;
 bool released=false,previous_direction=false,previous_confirm=false;
 bool cancelled=false;
 void draw_choice(const std::string& label,int top,bool active) {
  std::string wide;
  for(char c:label){wide.push_back(char(0x82));wide.push_back(char(0x60+c-'A'));}
  font.shift_jis(fb,0,316-int(label.size())*8,top,48|(active?15:5),wide);
 }
 void draw() {
  fb=background;
  draw_choice("STANDARD",296,selected==GameMode::standard);
  draw_choice("CHALLENGE",316,selected==GameMode::challenge);
 }

public:
 ModeMenu(const Framebuffer& back,const FontRom& bitmap):background(back),fb(back),font(bitmap) {fb.text_overlay.clear();background.text_overlay.clear();draw();}
 bool step(const InputKeys& keys) {
  if(!released) {if(!any_menu_input(keys))released=true;return false;}
  if(keys.escape||keys.strike){cancelled=true;return true;}
  bool direction=keys.up||keys.down||keys.left||keys.right;
  bool confirm=keys.shot||keys.enter;
  if(direction&&!previous_direction){selected=selected==GameMode::standard?GameMode::challenge:GameMode::standard;draw();}
  bool accept=confirm&&!previous_confirm;previous_direction=direction;previous_confirm=confirm;
  return accept;
 }
 GameMode mode() const {return selected;}
 bool was_cancelled() const {return cancelled;}
 const Framebuffer& framebuffer() const {return fb;}
};
}

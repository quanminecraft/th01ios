#pragma once
#include "input_backend.hpp"
namespace th01_native {
inline bool start_button_down(bool mapped_key,bool controller,bool mapped_only) {
 return mapped_key||(!mapped_only&&controller);
}
inline void apply_start_button(InputKeys& keys,bool pressed,bool in_menu) {
 if(!pressed)return;
 if(in_menu){keys.escape=false;keys.enter=true;}
 else keys.escape=true;
}
}

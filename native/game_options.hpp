#pragma once
#include <cstdint>
namespace th01_native {
enum class GameMode {standard,challenge};
struct GameOptions {
 std::int8_t rank=1,bgm_mode=1,credit_bombs=1,credit_lives_extra=2;
 GameMode mode=GameMode::standard;
};
struct ChallengeRules {
 GameMode mode=GameMode::standard;
 std::int32_t misses=0,extends=0;
 std::int32_t displayed_misses() const {return misses-extends;}
 bool good_ending() const {return displayed_misses()<=10;}
 void miss() {if(mode==GameMode::challenge)++misses;}
 void extend() {if(mode==GameMode::challenge)++extends;}
};
}

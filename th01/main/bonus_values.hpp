#ifndef TH01_BONUS_VALUES_HPP
#define TH01_BONUS_VALUES_HPP
// Shared with the original bonus animation. The final stage parameter is
// one-based for the stage just cleared, as passed after main_01 increments id.
inline int stagebonus_time_value(int timer) {return timer * 3;}
inline int stagebonus_combo_value(int combo) {return combo * 100;}
inline int stagebonus_resources_value(int lives,int bombs) {return lives * 200 + bombs * 100;}
inline int stagebonus_stage_value(int stage) {return (stage - 1) * 200;}
#endif

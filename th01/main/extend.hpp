#include "th01/score.h"

static const score_t SCORE_PER_EXTEND = 400000;

#ifdef TH01_NATIVE_ORB_PHYSICS
extern std::int16_t extend_next;
#else
extern int extend_next;
#endif

// Checks if [score] has exceeded a certain threshold, and grants an extra life
// in that case, redrawing the life section of the UI if that happened.
void score_extend_update_and_render(void);

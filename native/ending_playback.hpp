#pragma once
#include "ending_verdict.hpp"
namespace th01_native {
// Runs the original four-way ending, slideshow, verdict and registration
// handoff with one shared GRP palette/tone state throughout.
void play_ending(Framebuffer& fb,const EndingResult& result,VerdictHost host,
 std::function<void(const char*,const char*)> audio);
}

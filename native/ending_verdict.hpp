#pragma once
#include "ending_palette.hpp"
#include <memory>
#include <optional>

namespace th01_native {
struct EndingResult {
 std::uint32_t score=0,highest=0,continues=0;
 std::array<std::uint32_t,4> scene_continues{};
 unsigned rank=1,extra_lives=2;
 bool makai=true;
 bool challenge=false;
 std::optional<bool> good_ending_override;
};
struct VerdictHost {
 std::function<SpriteSet(const char*)> load;
 std::function<void(unsigned)> wait;
 std::function<void(int,int,int,const char*)> text;
 // Returns the original OK and shot flags. Reset consumes held input.
 std::function<std::array<bool,2>(bool reset)> input;
 std::function<unsigned()> random;
 std::function<void()> registration_colors;
 std::function<void(std::uint32_t,int,std::uint16_t)> registration;
 std::function<void()> resident_clear;
};
class EndingVerdict {
 struct Impl;
 std::unique_ptr<Impl> impl;
public:
 EndingVerdict(Framebuffer& fb,EndingPalette& palette,const EndingResult& result,VerdictHost host);
 ~EndingVerdict();
 void run();
};
}

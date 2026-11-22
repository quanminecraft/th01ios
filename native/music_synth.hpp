#pragma once
#include "archive.hpp"
#include <filesystem>
#include <memory>
namespace th01_native {
// 48 kHz stereo native music renderer. Call commands/render on one thread,
// or serialize them at the audio-device boundary.
class MusicSynth {
 struct Impl;
 std::unique_ptr<Impl> impl;
public:
 // Diagnostic output mask: 1 FM, 2 SSG, 4 rhythm. Sequencing is unchanged.
 explicit MusicSynth(const std::filesystem::path& assets,unsigned output_mask=7);
 ~MusicSynth();
 void command(const std::string& operation,const std::string& asset={});
 void effect(unsigned id);
 void render(std::int16_t* stereo,unsigned frames);
 bool playing() const;
 unsigned notes() const;
 void set_effect_timer_period(unsigned period);
};
}

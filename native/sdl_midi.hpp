#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
namespace th01_native {
// FluidSynth is loaded at runtime; it shares the existing SDL audio device.
class SdlMidi {
 struct Impl;
 std::unique_ptr<Impl> impl;
public:
 SdlMidi(const std::filesystem::path& assets,const std::filesystem::path& soundfont);
 ~SdlMidi();
 void command(const std::string& operation,const std::string& name);
 void render(std::int16_t* stereo,unsigned frames);
 unsigned timer_period() const;
};
}

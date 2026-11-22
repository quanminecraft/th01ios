#pragma once
#include "music_synth.hpp"
#include "sdl_midi.hpp"
#include "recorded_music.hpp"
#include <SDL.h>
#include <atomic>
namespace th01_native {
class SdlMusic {
 MusicSynth synth;
 std::unique_ptr<SdlMidi> midi;
 std::unique_ptr<RecordedMusic> recorded;
 SDL_AudioDeviceID device=0;
 std::string error;
 std::atomic<std::uint64_t> frames{0};
 bool mono=false;
 static void callback(void* userdata,Uint8* output,int length);
public:
 explicit SdlMusic(const std::filesystem::path& assets);
 ~SdlMusic();
 void command(const std::string& operation,const std::string& asset={});
 void effect(unsigned id);
 void check();
 std::uint64_t rendered_frames() const {return frames.load();}
};
}

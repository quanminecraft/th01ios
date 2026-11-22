#include "ending_playback.hpp"
#include "ending_sequence.hpp"
namespace th01_native {
void play_ending(Framebuffer& fb,const EndingResult& result,VerdictHost host,
 std::function<void(const char*,const char*)> audio) {
 EndingSequence* active=nullptr;
 EndingHost callbacks{host.load,host.wait,host.text,std::move(audio),[&]() {
  EndingVerdict verdict(fb,active->colors(),result,host);
  verdict.run();
 }};
 auto ending_continues=result.good_ending_override?(*result.good_ending_override?0u:1u):result.continues;
 EndingSequence sequence(fb,std::move(callbacks),result.makai,ending_continues,result.challenge);
 active=&sequence;
 sequence.run();
}
}

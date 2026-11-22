#pragma once
#include "archive.hpp"
#include <functional>
#include <memory>

namespace th01_native {
// Native MDRV2 music-bytecode sequencer. The callback receives OPNA register
// writes; no original executable or CPU interpreter is part of this class.
class MdtPlayer {
 struct Impl;
 std::unique_ptr<Impl> impl;
public:
 using Write=std::function<void(unsigned,unsigned)>;
 // Optional MIDI observation of the same sequencer (status, data1, data2).
 // It does not replace or change the original OPNA register stream.
 using Midi=std::function<void(unsigned,unsigned,unsigned)>;
 MdtPlayer(Bytes data,Write write,std::function<void(unsigned)> rhythm);
 // MDE effect IDs are one-based; its streams share the MDT event language.
 MdtPlayer(Bytes effects,unsigned id,Write write);
 ~MdtPlayer();
 void tick();
 unsigned timer_period() const;
 unsigned loops() const;
 unsigned notes() const;
 bool finished() const;
 void set_midi(Midi callback);
};
}

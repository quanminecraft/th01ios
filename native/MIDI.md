# MDT to MIDI (2026-10-30)

MyMini PortMaster launcher now defaults to MIDI BGM. Original MDE sound effects
remain synthesized through the existing native OPNA path and are mixed into
the same SDL audio device. No DOS/x86 emulator or external MIDI player process
is involved.

The same MdtPlayer interprets the original MDT note streams, gates, ties, loops,
transpose, detune, tempo, software pitch LFO and portamento. An optional MIDI
observer emits note, expression, pan and pitch-bend messages. A source note 0
is C1 (MIDI 24). Ties retain the active MIDI voice and change its pitch bend;
RPN sets bend range to 12 semitones. MIDI channels 0..5 carry FM parts, 6..8 SSG,
and channel 9 the six rhythm instruments. Channel pan 0 mutes an FM part.

GM instrument assignment is an approximation, not recovery of original FM
patch timbres: programs 80,81,4,33,48,80 for FM and 80 for SSG (zero-based).
This conversion also inherits any remaining MDT interpreter inaccuracies.
It is not a proof of original-driver or original-music accuracy. Channel
instrument changes currently keep the assigned GM program; source FM patch
IDs are not equivalent to GM program IDs. SSG envelope timbres and percussion
velocity/pan are not fully translated. These are remaining musical limitations.

Runtime timing uses the original Timer-B interval: period * 48 / 166400 seconds.
At 48000 Hz this is period * 180 / 13 frames, with the fractional remainder
carried across ticks. The MIDI backend renders in blocks ending at sequencer
ticks, so it does not depend on graphics frame rate. Stop/play resets MIDI
voices; fade changes synth gain. SE uses the current MIDI sequencer tempo.

FluidSynth is loaded dynamically, with one synth and no FluidSynth audio driver.
Reference API: https://www.fluidsynth.org/api-1.x/ and
https://www.fluidsynth.org/api-1.x/synth_8h.html . SDL owns the only audio device.
Reverb/chorus are disabled, polyphony is 64, sample rate is 48000 Hz, and gain
is 0.35. The tested MyMini already supplies libfluidsynth.so.3 and the
TimGM6mb.sf2 soundfont; neither is copied into the port package.

Environment:

* `TH01_MIDI_SOUNDFONT=/usr/share/sounds/sf2/TimGM6mb.sf2` enables MIDI in SDL.
* Device launcher sets that value by default. `TH01_AUDIO_BACKEND=opna` when
  invoking the launcher selects the previous OPNA BGM path instead.
* Windows preview still uses OPNA unless a compatible FluidSynth DLL and a
  soundfont are supplied and TH01_MIDI_SOUNDFONT is set.

Tools (paths relative to checkout):

```
build/linux/midi_export INPUT.MDT OUTPUT.mid [SECONDS=120]
build/arm64/midi_render ASSETS MDT SOUNDFONT OUTPUT.wav [SECONDS=6]
```

The SMF exporter produces format 0, 480 PPQ, 500000 us/quarter with rational
source timing rounded to MIDI ticks, and all-notes-off at the end. The files in
build/midi contain all 19 original inputs, capped at 120 seconds for audition;
INIT is intentionally silent. Runtime playback reads MDT directly and retains
its unbounded song loops, rather than looping the bounded audition MIDI file.

Verification:

* All 19 inputs produce in-range MIDI messages on Linux and host-run ARM64.
  OPNA register hashes/counts, note counts, loop counts and tempo remain
  identical with/without the MIDI observer at every tested tick (15000 ticks
  per file). A separate C1->D1 tied phrase checks one voice, octave, bend,
  start tick 0 and release at the rest on tick 8.
* Full Linux suite 141/141 passed before the final pan/SE-tempo refinements;
  focused MIDI tests pass after those refinements on Linux and ARM64.
* MyMini rendered six seconds of REIMU with TimGM6mb: peak 11267, no clipping,
  energy 838147588; render took 0.824 seconds at low process priority while
  another game process was active. This is not a standalone game FPS test.
* Installed binary SHA256:
  ba348da6f3ebf76e123ba2e8cce1cf3c4aee641a5405bf7699d004980fc138d0.
  Normal launch through Ports loads FluidSynth, owns ALSA and was confirmed
  OK by the user. Starting through SSH while suspending EmulationStation can
  leave its ALSA device open; use the normal Ports launcher for that case.

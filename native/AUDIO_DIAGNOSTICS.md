# Native MDT audio diagnosis (2026-10-30)

Scope: host PC diagnosis of the original MDT/MDE synthesis path. No device game,
launcher, profile or PortMaster package was changed during this investigation.

## Findings established by the original driver

MDRV98.COM SHA256:
`e14f26007f1a71d747f50deeb0f338065466e87ad142268416a1bcbb14b8ae7d`.

`mdrv_trace_reference.S` is a host-only DOS COM harness for a copied MDRV98 driver
under the user's DOSBox-X PC-98 reference environment. It hooks resident output
helpers with trampolines, preserves registers/flags and displaced instructions,
and stores register/data records in a separate allocated 64KiB DOS buffer. It
skips helpers that the driver has disabled with RET. Original files on disk are
not patched. Nothing from this CPU-emulated reference is shipped to the device.

Tick labels come from the Timer B processing branch at resident 0x18e3, not
from every interrupt/reset at 0x18d2: the latter includes Timer A interrupts and
would give misleading timing. Reference tick 1 aligns with native tick 0. This is
logical tick timing, not a per-OUT wall-clock timestamp. Latest metadata:
62676 bytes, 477 Timer B ticks, overflow=0. Compare only the first 476 complete
ticks to avoid stop/reset commands at the end of capture.

1. MDRV98 0x99e..0x9e2 selects streams by type mask and assigns them sequentially
   to physical channels. Native previously used low ID bits as hardware slots.
   REIMU has SSG IDs 0x41/0x42, so original maps A/B while native mapped B/C.
   This changes which music voice shares a channel with effects. Native now packs
   streams separately within the FM and SSG groups. MDE still owns C then B.
2. Original default FM master attenuation at resident 0x12f is 8. Native omitted
   it. Carrier TL values now match, including the observed +8 TL difference.
3. Rhythm master attenuation at 0x130 is also 8, with the driver's subsequent +1
   level calculation. REIMU level is 54; native previously sent 61. Now matches.

## What was compared

`fixtures/mdrv-reimu-fm.csv`: 130 captured original note-on states, including
logical tick, physical FM channel, FNUM/block, feedback/algorithm, four total
levels and 20 operator parameters. B0 bits 6/7 are ignored chip bits and are
masked in the oracle. All 130 states match after the fixes.

`fixtures/mdrv-reimu-ssg.csv`: 476 original tick-end states for SSG tone periods,
noise period, all three amplitudes and rhythm total level. These match after
fixes. Unused mixer channel bits and SSG IO-port direction bits are not treated
as audible differences. `mdt_driver_oracle_test` validates both independent
fixtures against the current parser and real user assets.

Stereo/mono: the REIMU reference and native samples have identical L/R samples.
Thus (L+R)/2 is unchanged and mono energy ratio is 1.0 for this excerpt. This
rules out phase cancellation for this title excerpt, not for every song.

Artifacts in `build/audio-diagnostic/`: `native-before.wav`, `native-after.wav`,
`native-before.csv`, `native.csv`, driver-trace/TRACE.BIN and captures/, plus
note-state and level reports. No WAV recording backend is enabled by these fixes.

## Reproduce the captured reference

Run from the repository root, with extracted assets in build/assets/th01-ja:

1. Copy mdrv_trace_reference.S to build/audio-diagnostic/driver-trace/logger.S.
2. GNU binutils: `as --32 logger.S -o logger.o`, then
   `ld -m elf_i386 -Ttext 0x100 --oformat binary -o PLAYLOG.COM logger.o`.
   The assembly's .incbin path is relative to the repository root.
3. Use prepare_mdrv_reference.py to prepare the copied driver, fonts, rhythm data
   and reference.conf. Change PLAY.COM to PLAYLOG.COM in that config.
4. Run the user's DOSBox-X with that config. Inspect META.BIN (<3H position,
   ticks, overflow); require overflow=0. TRACE.BIN records are <HHBB tick, register,
   value, reserved. Disabled helpers are untouched and restored live hooks are
   removed before the harness exits.
5. `build/linux/mdt_trace build/assets/th01-ja/REIMU.MDT native.csv 8`.

## Limits

This is a short REIMU excerpt. FM5/6 have no note-ons in this interval; absence
is a rest, not evidence of a dead channel. Full-song command/LFO/loop coverage,
other tracks, and calibrated FM/SSG/rhythm relative PCM gains still need work.
Register agreement does not establish bit-exact PCM agreement across FM cores.
Do not describe these fixes as proof that every missing-layer report is solved.

## 2026-10-30 second investigation: FM articulation and mixer balance

After the user reported the first device update still sounded wrong, added
output-only diagnostic group masks to MusicSynth/music_render (1 FM, 2 SSG,
4 PCM rhythm, 7 all). These preserve every MDT stream and tempo command; deleting
streams to make solos would change the clock if the tempo-setting stream vanished.
The original-driver solo harness similarly preserves all sequencing, suppressing
only key-on/SSG amplitude/rhythm trigger writes after saving caller registers.
Host reference probes live in build/audio-balance/{1,2,4}/, with zero log overflow.

Found two additional parser errors that note-on-only snapshots could not detect:

- MDRV98 enters its gate-decrement path (0x1bdc) on the note-on tick itself.
  Native had started one tick later. For six-tick notes, original key-off is at
  tick 5 and next on at 6; native had off and on together at 6. ymfm observes
  key state when clocked, so that immediate off/on pair could collapse and fail
  to restart the envelope. This made FM tones decay and differ irregularly.
  Native now decrements the initial FM gate on dispatch, omits redundant FM
  key-off before key-on, and preserves q=0 no-gate behavior. A MIDI gate test
  now expects release at tick 7, not rest tick 8, following original timing.
- MDRV98 0xcac..0xcc4 coalesces same-pitch tied FM fragments before computing
  quantized duration. Native computed only the final fragment's release. In
  LEGEND, tied 12+6 with q6 released at native tick57 versus original tick54.
  Coalescing before quantization fixes it.

Added original gate fixtures for REIMU (476 ticks), ST0 (338), LEGEND (376).
The former note-state oracle still matches all 130 note-ons. Independent manual
comparisons of active FM pitch/TL, SSG/rhythm state and gates in ST0/LEGEND now
have zero differences in the captured complete-tick windows.

After gate correction the FM reference/native RMS ratios became stable near
17.3, versus SSG near4.31. Matching the OPNA FM output scale to the SSG/PCM paths
with a 4x FM gain makes their common reference attenuation agree: FM ratio~4.32,
SSG~4.31 for the sampled windows. This is empirical calibration to the user's
DOSBox-X reference core/default mix, not a claim of hardware-perfect PCM.
New music_balance_test uses the independently recorded first 0.5..2.5s FM/SSG
ratio (~1.146) with a core/resampling tolerance; native ratio is1.18336. The
previous missing gain and collapsed-envelope behavior fail that tolerance.

Also moved AC coupling to the unipolar SSG component only. Signed FM and rhythm
PCM no longer pass through that unnecessary DC-removal filter. Existing MDE-only
SE energies remain unchanged. Rhythm has residual reference/core waveform and
level differences; it has not been declared bit-exact or perfectly calibrated.

Final 30-second renders of16 distinct non-silent tracks: peak<=14108/32768,
zero clipped samples. ARM64 host-run oracle/balance/all17SE/MIDI checks pass.
All outputs remain synthesized from original MDT/MDE, not recorded BGM. No
full-song proof is claimed by these short reference windows.

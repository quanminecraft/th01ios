# TH01 native port

Upstream: https://github.com/nmlgc/ReC98, `debloated` commit
`28d59d31331e27fcbe61d1b65bd8c0cd618757d8`.
Port branch: `codex/th01-native-arkos`.

Target: Xifan MyMini / ArkOS / Linux AArch64. The native game contains no DOS/x86
CPU emulator. Host-side QEMU is used only to validate ARM64 builds on the PC.

## Current playable checkpoint (2026-10-30)

Current device update: Standard/Challenge selection is available after Start;
Challenge uses unlimited survival and signed MISS = deaths - score extends,
with good ending at MISS <= 10. OPNA BGM is restored, mixed to mono for MyMini.
Stage preparation, boss music ordering, bomb sound onset, pre-launch/Continue
input and BGM Off on return were updated. Latest tests: 142/142 Linux plus
ARM64 integration regressions. Missing music-layer fidelity remains open.
See the last STATUS.md checkpoint for current evidence.



The unified SDL preview now includes the original title, Start/Continue,
Options and Music Test, both routes/all seven bosses, ending/slideshow,
verdict and original high-score name entry. It returns to title after a run.
Native MDT BGM playback is connected throughout; music is generated through
FM/SSG synthesis and the user's OPNA rhythm samples, without a DOS/x86 CPU
emulator. Original MDE sound effects are connected. Full original-driver audio parity and MyMini
hardware/performance validation remain unfinished.

Windows: run `build/windows/TH01 Native.cmd` for the unified preview.
Stage/boss launchers still bypass title for testing. Normal launches have no
frame limit. Z shoots, X strikes, Esc pauses and Q closes the preview.

Linux full suite: 141/141 passes (see MIDI.md for final focused audio checks). Title SDL images match Windows/Linux/ARM64.
A 12-second native BGM render is byte-identical on Linux and host-run ARM64;
Windows SDL also opens the normal audio device and renders while entering
stage 1. No TH01 deployment to MyMini has been made.
Detailed implementation history and evidence follow below.

MyMini now defaults to MIDI BGM converted from the original MDT event streams
through FluidSynth/TimGM6mb; original MDE SE remains enabled. Normal Ports
launch was verified on-device and confirmed by the user. GM timbres are an
approximation; Windows still defaults to OPNA. See [MIDI.md](MIDI.md).

## Foundation components and implementation history

- Read-only Anex86 HDI extraction with PC-98 partition and FAT12/16 support.
- C++17 TH01 archive reader, explicit little-endian wire fields, XOR and RLE.
- Bounds checks, truncated input rejection, compact directory handling.
- Synthetic raw/RLE/malformed input tests and a real-data validation CLI.
- PTN/GRC/BOS sprite decoding, including PTN color-key and inverted BOS mask.
- GRP/Pi predictor and move-to-front image decoding translated from the
  upstream `libs/piloadc/piloadm.asm` algorithm into C++.
- HUD MASK.GRF first-plane decoding using upstream run semantics.
- Two framebuffer pages with four bitplanes each, clipping, palette conversion,
  byte-aligned sprite blitting, and background restoration.
- STAGE data parsing: four-bit card order, multi-flip HP and obstacle layout.
- SDL2 sprite/background gallery and static stage layout preview.
- Original orb physics source compiled directly for native targets behind
  `TH01_NATIVE_ORB_PHYSICS`; original DOS rendering path stays separate.
- Fixed 16-bit coordinate/frame storage and master.lib's original RNG recurrence.
- Original CShots implementation compiled natively, with quarter-sprite draw,
  masked restoration and EGC-style word-aligned background restoration.
- Original Reimu movement clamp and orb-player hit/repel routines compiled
  natively, together with the original standing/moving shot, swing, slide,
  flipkick, slidekick and shot-combo state machine.
- Native PTN/player-animation rendering adapters, retaining the original
  one-byte overlap masks and invisible transition frames. Bomb effects,
  bomb HUD and pellet-speed changes are explicit external interfaces that
  reject calls until the corresponding systems are connected.
- Linux x86-64, Windows x86-64 and ArkOS-compatible AArch64 builds.
- Original input_sense/input_reset_sense compiled directly with a physical-key
  snapshot adapter. This retains edge-triggered/consumed attacks, arrow/numpad
  merging, pause quirks and the original bomb double-tap window.
- Original cards_hittest compiled directly with native card storage, scoring
  and ordered HUD/item/SE requests.
- Original cards_score_render/cards_update_and_render now compile directly:
  popup timing, both-page sprite updates, HP decrement, removal, stage-complete
  flags and Lunatic revenge-pellet requests. Item and pellet requests still
  need actor-system consumers; HUD score requests still need HUD integration.
- Native bitmap reader for the local V98-layout FONT.ROM, using the same JIS
  row 0x29 ASCII glyphs and bold-weight operation as ReC98's graphics text path.
  It currently renders ASCII text effects used by card score popups. Other
  text encodings/effects fail explicitly until implemented.
- Original obstacles_update_and_render, turret firing and portal routines
  compile directly with native fixed-size obstacle state. The native turret
  enum increment avoids Borland's reference-cast extension while retaining
  its original values and timing.
- A shared active framebuffer page now backs page_access and PTN operations.
  Explicit front-page restore/player-animation routines return to page 0;
  turret warmup/cooldown writes both pages as the original code requires.

The user's original Japanese `th01.hdi` was extracted into
`build/assets/th01-ja`; neither the English nor modified image was selected.
The original disk image is unchanged. Extracted copyrighted data stays in
ignored `build/` and must not be committed or bundled in source releases.

The existing bundle's `pc98lib/emulator/FONT.ROM` was copied as bitmap data
to `build/assets/th01-ja/FONT.ROM`, separately from HDI extraction. Its SHA256
is `c261bba53f2d7dab36c7eeb4be21c8c2b56c71d8d9c3c4609b19b7d230905153`.
The original font file is unchanged. Format layout was checked against the
primary [font loader source](https://github.com/AZO234/NP2kai/blob/wx_alpha/font/fontv98.c);
no code from that emulator is linked or executed by the port.

Real-data validation: 41 archive assets (418,524 bytes), 227 sprites,
47 GRP images and eight scene layout files. The stage preview renders a
static layout with background, cards, obstacles, HUD mask, Reimu and orb.
It has no active gameplay, score display, player controls, audio or boss AI yet.

Fourteen component test suites pass on x86-64 and AArch64. The isolated 10,000-frame orb
trace matches across Windows/Linux x86-64 and AArch64:
`c1b2b0da96abb803`. This is not original-PC98 replay equivalence.
QEMU is used only as a development runner for native ARM64 binaries on the PC;
it is not part of the port or preview.

Real-data integration also exercises the original shot update against STG.PTN
and ST0.GRP: shot/orb collision occurs at frame 12, preserving the extra orb
movement on impact. Its final framebuffer digest is `0d65c5fdd2da6afa`.
The sound adapter currently emits the original SE commands into an explicit
queue for verification; it does not play MDRV2 sounds. These modules are not
yet connected to the SDL asset viewer or a live game loop.

The action tests verify standing/moving shot timing, normal-rank swing
deflection, the original 13-step slide, moving/stationary flipkicks, slidekick,
the seven-shot position pattern and the three-special combo limit. Framebuffer
traces across the sequence match Windows/Linux x86-64 and AArch64: synthetic
`e5fe831898794205`, real sprites/background `874dd2c4e9cd1c6e`.

Input/action integration drives those original modules through sampled key
states: held Z produces one consumed shot rather than synthetic autofire,
release/re-press re-arms attacks and physical release/re-press during the
slide window reaches the seven-shot combo. Card tests cover strict geometry,
same-frame score/combo ordering, Lunatic multiplier, 25,600-point cap, the
original 16-bit square wrap (combo 256 scores the base value), drop-cycle
wrap and bomb-selected cards. Original keyboard/debug bit constants are data
only; no BIOS or CPU is emulated. DOS heap debug screen requests fail explicitly.

Card-animation tests cover removal at frame 30, multi-flip HP, score-popup
cleanup, stage-clear/player-hit signals, and Lunatic revenge requests at frame
25 (the request queue is not yet an active bullet simulation). Both-page
framebuffer traces match Windows/Linux x86-64 and ARM64: synthetic
`faa870a69cb00795`, real assets/font `364319637b537f55`. A controlled mass-hit
test clears all 26 cards in the real first-stage layout with score 113,100;
this is not a player-driven playthrough. A popup snapshot is available at
`build/card-popup.png`; its Windows/ARM64 BMP counterparts have identical SHA256
`5d8545b0242853cc592ce33aad91c3ad582c72bb676b4e770e6fa7d8314af6fd`.

Obstacle tests cover bumper displacement/fast-X quirks, strict bar boundaries,
shared vertical-bar collision blocking, portal destination RNG order, keyframes
20/40/60, the single-portal 17-roll fallback, all 12 slow/quick turret group
variants and Easy-rank suppression. Turret requests occur at frames 210/110
and their seven-frame cooldown restores both pages. These are spawn requests,
not yet active pellet actors. The real STAGE0/STG.PTN portal trace matches
Windows/Linux x86-64 and ARM64: `bbf1ac499dc3129e`. Frame-20 BMP files match with
SHA256 `049b4e674a63a664b5c9236ff576556fefa120bad590b436acdbd29bcc18c473`.

## Build

    cmake -S native -B build/native
    cmake --build build/native
    ctest --test-dir build/native --output-on-failure

Run `asset_check <extracted-game-directory>` to validate the real archive.
The CLI detects its header so Japanese filenames do not need conversion.
`extract_hdi.py IMAGE --output build/assets/NEW-DIRECTORY` requires a fresh
output directory and never executes the original DOS files.

### Windows

    powershell -ExecutionPolicy Bypass -File native/build-windows.ps1

The self-contained output directory is `build/windows/`; launch
`TH01 Native Asset Preview.cmd`. It has no automatic timeout. Arrow keys
browse assets and Esc closes the viewer. A gamepad D-pad left/right also
browses; Back closes. Optional CLI flags:

    sprite_preview ASSET_DIR --asset STAGE0.DAT --layout 0
    sprite_preview ASSET_DIR --asset MIKO.PTN

`--layout` selects one of the four card stages in a scene, 0..3. Unused
scene 7 is parsed by validation but omitted from the gallery because ST7.GRP
is absent. `--dump FILE.bmp` writes a screenshot; `--frames N` is an explicit
test-only frame limit (normal launches keep running).

### ArkOS cross-build (existing WSL toolchain)

    cmake -S native -B build/arm64 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/absolute/repo/native/arkos-aarch64-toolchain.cmake
    cmake --build build/arm64

Uses GCC 9.3 and the existing Debian 11 target sysroot, with a static C++
runtime. The current viewer ELF directly requires GLIBC_2.17 and system SDL2;
the SDL2 runtime is provided by the target OS. Hardware output/input has not
yet been verified on MyMini.

## Next

Integrate the native CPellets actor/motion/hit/render code and polar/aim math with the existing obstacles, card animation, input, player actions,
shots and orb routines in a deterministic game loop. Actor/item/HUD calls must
dispatch synchronously at original call sites to preserve RNG order; the
current request capture is a component-test boundary, not a finished runtime.
Then extend to items, stage progression, boss AI, menus, text and MDRV2 audio replacement. Finish
the MyMini display/controller backend and PortMaster packaging, and verify on
hardware. GRZ/GRX effects and PC-98 text rendering are still missing; the
original DOS/PC-98 hardware paths are not compiled into the native milestone.

The stage_tick library now compiles the shared original orb/pellet/stage
frame routine. A StageTimingBinding supplies its mid-frame wait, and a
StagePelletBinding routes card/turret spawns to CPellets synchronously.
The SDL viewer does not yet call this routine; full main-loop integration
and the remaining gameplay systems are still required.

Item core now includes original item.cpp lifecycle and plain Shift-JIS
collection popups. The resident point value is native uint16 storage.
The host must bind the card renderer and bomb HUD callback when running
item updates; complete interactive main-loop integration remains unfinished.

HarryUpAnimationBinding now implements the enlarged timer-expiry text using
ANK font data and a separate text overlay. Its host wait callback presents
the composite for 27 frames per phrase. The graphics pages are preserved.

## Interactive stage 1 prototype

Build with `cmake --build build/linux --target stage_preview` or, on Windows,
`./native/build-stage-windows.ps1`. Run `stage_preview ASSET_DIR` (Windows:
`build/windows/TH01 Native Stage 1.cmd`). Arrows move, Z shoots, X strikes,
Escape pauses, Enter confirms, Q closes. SDL controllers use D-pad/left stick,
A for shot/confirm, B for strike, Start for pause; hotplug is supported.
No default time limit or automatic exit is enabled. Original stage-1 gameplay,
miss/bomb/pause blocking animations use the same SDL presentation callback.
The host currently uses a provisional 60 Hz cadence pending original timing
validation. Audio commands are drained but playback is not implemented.
Stage clear/Game Over stop at an explicit boundary pending progression/UI.
`--frames N --autoplay --unpaced --dump FILE.bmp` is a deterministic smoke
mode for development, not a playthrough or input-accuracy test.
Linux and ARM64 SDL smoke runs of 600 ticks produce identical framebuffer
SHA256 9d2a57419a67a53c7d88e0f838e702bfea7ea176836f482b9150229d4b673e8b.
ARM64 runner needs the sysroot SDL libraries explicitly via qemu `-E
LD_LIBRARY_PATH=/opt/sysroot-debian11/usr/lib/aarch64-linux-gnu:/opt/sysroot-debian11/lib/aarch64-linux-gnu`.
No MyMini hardware run or device installation has been performed.
## Scene 1 regular-stage progression

Bonus acknowledgement now uses original input_reset_sense/input_shot/input_ok
latches, including press/release behavior. Four scene bonus values persist in
the session. Stages 2-4 load their actual STAGE0 layouts, preserve stock/score,
reset orb/combo/timer/invincibility/shot/noncloud-pellet state and begin playing
without another startup-shot gate. Native original-size ANK title overlay
shows STAGE and the two-digit number for 35 frames each. Stage 5 enters an
explicit boss_pending boundary; Singyoku is not implemented yet.
Controlled integration clears each stage, acknowledges and runs the next one;
this does not prove a natural full playthrough. Pixel checks preserve the
stock HUD between stages, and snapshots for stages 2/3/4 match Linux/ARM64.
Background rebuild currently uses the settled original scene image with HUD
preserved; original per-object restoration and first-scene title entrance
still need fidelity work. Items persist across regular stages as in source.
Interactive host supports the progression; no device deployment or audio yet.
## Singyoku connected to interactive stage 5

StageSession now enters the actual boss after stage-4 bonus acknowledgement.
A direct preview start is available with `stage_preview ASSET_DIR --stage 5`;
Windows launcher: build/windows/TH01 Native Singyoku.cmd. Default starts at 1.
Boss startup loads BOSS1.GRP and original three sprite banks, resets transient
bullet/cloud/laser/item storage (original MAIN process reload boundary), keeps
resident score/stock, initializes full HUD/timer/orb/player and runs the 70-frame
stage number plus 200-frame boss entrance. First-scene title effect is pending.
The shared gameplay frame invokes boss AI after invincibility and before lasers
and orb/pellet update; an order test asserts this sequence. Pause, actual miss
animation/respawn and remaining boss HP are exercised by the session test.
Controlled damage reaches phase 2 and defeat; defeat currently freezes at an
explicit boss_defeated boundary. The original defeat animation/route-selection,
TOTLE scene bonus, routes beyond stage 5 and Game Over/continue remain unported.
Music fade commands are counted but audio playback remains absent.
Linux full suite: 49/49 pass after rebuild. ARM64 whole-session test passes
through PC runner. SDL direct-boss smoke runs 600 ticks/1167 presentations on
Windows, Linux and ARM64 with identical inspected framebuffer SHA256:
e936c059bc03dfb4c8d675485d90d58b43c02f36cbc4d88d29567d13f8219991.
No default auto-exit/time limit. Windows preview rebuilt and smoke-tested.
No MyMini hardware verification or deployment has been performed.
## Original boss defeat/route UI and Game Over animation

Original defeat.cpp now compiles through DefeatBinding. White scanlines,
white-out/white-in palette loops, original Shift-JIS glyph rows, 2x scaling,
Makai/Jigoku selection, held-direction suppression, 50-frame input gap and
confirm SE are shared with the source. StageSession invokes this after boss
HP zero, stores selected_route(), then stops at the still-unported TOTLE/
next-scene boundary. A callback bridge keeps standalone AI tests independent.
Real tests cover both routes, held down and shot/Enter confirmation; session
integration confirms Makai through the host poll. Snapshots inspected.

Original gameover.cpp now drives final-life loss in StageSession. It resets
items, draws the randomized miss sprite and five expanding effect trails,
shakes for 16 frames plus a final reset and resets live/saved bombs to 1.
Tests verify 16 delay calls/14 delayed frames, 17 shake VSync callbacks,
scroll reset, one RNG roll and stock; session test exhausts all five lives.
The original clipped[frame] out-of-bounds read is explicitly bounded: first
five frames retain its wrong index, later frames skip unputs as documented
by upstream for adjacent nonzero coordinate words. This is a defined native
model of that source bug, not arbitrary DOS stack-memory emulation; unusual
zero-coordinate edge equivalence is not proven. Continue UI remains pending.

Linux full rebuild/suite: 51/51 pass. ARM64 defeat, Game Over and whole-session
checks pass through PC runner. Windows preview rebuilt and 600-tick boss SDL
smoke passes. Inspected Linux/ARM64 screenshots match SHA256:
Makai 51f9eae4e2d55c9e33b0a003ab544d70dfcc7d0afb5be06ad8f49228f3383938
Jigoku ad840260197e40da93c425c38816473c4fbfd4b0015e41949bdac78720680a80
Game Over 2c235edfca2b33591694fc2e87ddfe32b834585984984b90a34c70b30173d601
No MyMini deployment/hardware validation. Audio and later scenes remain.
## Game Over -> registration -> Continue -> current-stage reload

StageSession now loads GAME_O.GRP after the final-life animation, presents
original sixteen black-in palette steps with VSync and 10 ms waits, invokes
original score registration, then original Continue menu. Native profile data
lives beside extracted assets in native-profile by default (an explicit profile
path can be supplied to StageSession). Original asset/game directories are not
modified. Session startup and Continue reload read saved difficulty high score.
Yes resets live score/point value/extend state, restores credit lives and the
Game Over bomb stock of one, reseeds from frame_rand, initializes randomized
card-flip cycle and reloads the SAME stage. Regular stages wait for shot again;
Singyoku reloads HP/entrance and starts its fight. Transient actors, invincibility,
HUD backgrounds, timer, player/orb/combo state are reconstructed. Resident route,
scene bonus history and continue count persist. First-scene title effect and
native title/menu after declining remain unfinished.
No/Esc/timeout reach explicit title_pending; SDL currently closes the prototype
there because original title is not ported. Score registration polling now
renders even without a timed delay; SDL zero-count presentation is immediate
and idle rendering is throttled without advancing original gameplay counters.

Session test exhausts all lives, registers/acknowledges, uses Yes on stages 1
and 2, checks same-stage/score/stock/player resets, declines on a later Game Over,
and separately continues Singyoku with fresh eight HP. Test profiles are scoped
under each binary directory to avoid touching playing profiles or cross-build
fixture races. Linux full rebuild/suite: 54/54 pass, then targeted session test
passes after fixture isolation. ARM64 integrated session test passes with its
own profile. Windows preview rebuilt; 600-tick SDL boss smoke passes. Existing
launchers now include this playable Continue flow; no MyMini deployment yet.
Audio, TOTLE/post-boss progression, later scenes and complete title remain.
## Original TOTLE digits, arithmetic and interactive post-boss flow

TOTLE coordinates, original clamp/accumulation macro, rolling digit routine
and full totle_animate body are shared in includes with bonus.cpp. Native
long-sized metric storage is explicit int32; unsigned stage_timer*5 wraps to
uint16 before assignment, and the signed combo product narrows to DOS int.
Original NUMB.PTN big numerals and quarters retain the scrambled intermediate
10..19 rolling frames, sloppy background restore and final correction.
TotleBinding supplies real assets, input, frame presentation and millisecond
waits. Assets are host-cached; loaded/sloppy flags guard slot lifetime.

For the tested stage-5 sample (timer 700, combo 5, lives 4, bombs 3), original
bonus is 121000. Tests assert 245 frame waits, 296 four-ms callbacks/SE-2 events,
press/release acknowledgement, cleanup, unchanged RNG, uint16 timer wrap
(14000*5 -> 4464) and capped/final-stage arithmetic. Inspected NUMB screenshot
matches Linux/ARM64. Full Linux suite: 57/57 pass; ARM64 TOTLE and integrated
session tests pass. Windows preview rebuilt and SDL boss smoke passes.

StageSession now automatically runs TOTLE after boss route selection, resets
life-loop completion metadata, frees boss banks, commits bonus once and waits
for acknowledgement. It then reaches explicit next_scene_pending. The session
test checks the actual stock/timer-derived award and cannot grant it again.
Next: selected-route scene initialization and stage 6 onward. No MyMini device
changes; audio, later bosses and complete title/ending remain unfinished.
## Windows route preview verification

Windows stage_preview.exe rebuilt for stage 6-9 integration. Added direct
stage-6 Makai/Jigoku launchers to the reproducible Windows build script and
build/windows output. Both Windows SDL dummy-driver runs completed 600 ticks,
868 presentations and state=playing. Added two CTest SDL route smoke cases;
both pass on Linux (prior 58-test suite remains recorded above). These are
headless host checks, not visual inspection or MyMini hardware validation.
Stage-session API comments now describe current stage-10/title boundaries.
Next boss work requires original line sprite blitting, missile behavior and
route-specific effects; neither Mima nor YuugenMagan is playable yet.
## Boss byte-shifted row rendering prerequisite

Implemented CBossEntity::put_1line from original entity_a.cpp semantics:
source-row selection, all eight pixel shifts, per-word intended-row clipping,
raw foreground plane OR and alpha. Preserves the source right-edge spill into
the next row and negative-left whole-word clipping rather than replacing it
with rectangle clipping. Out-of-framebuffer writes are safely discarded.
Real boss_entity_test passes on Linux for all shifts, unchanged neighboring
rows, invalid cel, left clipping and right spill. ARM64 passed the all-shift
version; the two subsequently added edge assertions are Linux-verified only.
No gameplay caller uses this new method yet. unput_and_put_1line and Mima's
entrance remain unfinished, as do missile behavior and full stage-10 bosses.
## Boss row background restore prerequisite

Implemented CBossEntity::unput_and_put_1line with original little-endian
16-bit words, separate unsigned alpha-mask rotation and signed foreground
arithmetic shift/add. Negative foreground left shifts are expressed as safe
32-bit multiplication. This preserves the original sign-extension artifact
rather than substituting an ordinary pixel translation. Source per-word
intended-row clipping remains, with out-of-framebuffer accesses discarded.
Renderer returns to page 0 after valid calls. Invalid cels leave pages alone.
Linux and ARM64 real boss_entity_test pass: all put-row shifts and edge cases,
aligned background restore, page selection and a pinned synthetic 0x8000
foreground/0x7fff alpha case (shift one yields foreground c000, mask bfff).
These are renderer prerequisites only; Mima entry and full boss integration
remain unfinished. Preview binaries have not been rebuilt for this change.
## Original missile component port

Native missile_core compiles original missile.cpp with narrowly scoped native
branches for backend inclusion, safe enum increment and safe PTN-ID assignment.
Original Q12.4 motion, 50-slot pool, direction selection, interlaced draw/unput,
boundary explosion sequence, SE-7 command and player collision remain shared.
MissileBinding loads BOSS3_M.PTN and supplies frame parity and pixel-precise
quarter rendering. PTN color-15 transparency is masked before writing planes.
The renderer follows original three-byte shifts; its third byte correctly
carries the tail. Earlier commentary claiming tail loss was a misreading,
exposed and corrected by expanding tests to all 16 cels x 8 alignments.

Full Linux rebuild and suite: 61/61 pass. ARM64 missile_test passes with original
assets, including 128 rendering cases, subpixel motion, alternating field,
sloppy unput, capacity rejection, player hit/invincibility and explosion/free.
This component is not yet linked into StageSession or a live stage-10 fight.
Windows/device previews were not rebuilt/deployed. Mima still needs remaining
particle/shape/background operations, boss integration and entrance validation.
## Boss shape drawing prerequisite

Added native diamond/star/flake rendering from the original generated shape
masks and pixel-offset/page semantics. Original shape_ellipse_arc_put body is
shared verbatim in shape_arc.inc by DOS and native builds. Native adapter maps
its cached byte writes onto framebuffer planes, using original table polar
math. Preserves no drawing for equal/reversed bounds and no final cache flush.
Native color state is local to this renderer; persistent hardware GRCG state
outside these drawing operations is not modeled.
Linux shape test and ARM64 shape test pass: three masks x eight pixel offsets,
selected page, origin rejection, cardinal arc points and unflushed endpoint.
shape_core is not yet linked into StageSession; no preview/device deployment.
Particle source audit found its documented index-40 SoA corruption; that needs
an explicit DOS-layout model before sharing the routine safely. Particle
system, arc sloppy-unput and live stage-10 integration remain unfinished.
## Original particle component and explicit DOS overflow aliases

particle_core now compiles original particle.cpp with native rendering and
one explicit index-40 branch. Normal spawning/update/render remains shared.
Native overflow handler models original adjacent fields without C++ OOB or
invalid bool storage: x[40]->y[0], y[40]->velocity_x[0], vx[40]->vy[0],
vy[40]->alive[0..1], alive[40]->velocity_base[0], speed[40]->spawn_cycle.
The two overwritten bool bytes are represented by their nonzero truth values.
Preserves the two RNG calls, skipped initial slot zero, strict cycle > limit,
stale speed/cycle after init and spawn-cycle corruption on initial overflow.

Linux particle test passes for spawn/velocity/RNG, actual rendered pixel,
overflow alias values, subsequent suppression, wrap and init preservation.
Eight origins x 400 frames produce trace 17ac28a9671d638b on Linux and ARM64.
Final added pixel/init assertions are Linux-verified; prior trace version was
ARM64-verified. Trace equivalence is cross-host evidence, not a DOS capture;
original DOS runtime behavior of noncanonical bool bytes remains unmeasured.
System is not yet bound into StageSession or Mima, and no device deployment
occurred. Remaining Mima work includes sprite background operations, shape
unput, entrance/AI integration and runtime validation.
## Original Mima spread-in animation component

Moved original spreadin_bottom_cur/phase_spreadin verbatim into shared
mima_spreadin.inc. Native MimaEntrance supplies real CBossEntity row drawing,
page-one background snapshot and final both-page sprite put. Initial interval
1/speed 2 completes on iteration 51; iteration ten sets position/snapshots and
iterations 1-10 do not draw. Caller still owns the original two-frame waits.
This component does not run full mima_main, particles, HP or entrance palette.
Final puts retain existing transparent-pixel trails, as source put_8 does.

Real BOSS3.GRP/BOSS3_1.BOS test passes on Linux and ARM64: metadata, first ten
iterations, single snapshot, completion timing, pristine page-one background
composition and final opaque foreground. Full two-page trace matches:
89e3a4ff093c1bd7. Inspected iteration-30/51 images; Linux/ARM64 BMP SHA256s match:
30 516ddb75823c59ee09ab270304aab7c6d26dd51a1d4bc86c4c2a032a11d36ff0
51 bba283983806851887eeb38b4c210a9bf03af63b3dcb63da04fbe1b8c797ad36
No original DOS frame capture comparison or hardware timing validation yet.
Not linked into playable StageSession, and no Windows/device deployment.
Mima AI, background restore integration, remaining drawing operations and live
stage-10 fight remain unfinished.
## Stage-10 Jigoku diagnostic intro integration

StageSession now runs shared Mima spread-in after stage 9 Jigoku, then remains
boss_pending. Direct --stage 10 --route jigoku starts the same diagnostic
boundary. Loads settled BOSS3 background, clears stage actors, restores player/
orb/HUD and initializes original Jigoku timer 2000; waits two frames on each
non-completing spread iteration. AI, initial palette flash/entrance, HP setup
and particles/missiles in the actual fight remain absent. This is explicitly
an intro preview boundary, not a playable stage-10 boss. Makai stage 10 remains
unported. Windows script includes new source and Mima Intro launcher.

Full Linux rebuild/suite: 64/64 pass. Route scene test passes on ARM64 through
both paths, including clearing cards/obstacles and Jigoku timer at stage 10.
Windows preview rebuilt; direct stage-10 SDL smoke completed 2 host ticks,
172 presentations and boss_pending. Linux integrated screenshot inspected.
No MyMini deployment or hardware performance validation occurred.
## First two original Mima square attack routines

Moved CSquares and the first two attack bodies verbatim into shared
mima_square_patterns.inc. Native MimaSquarePattern runs the original schedules
and actually spawns original Pellets/Missiles. Square draw/unput geometry is
emitted to a required caller callback; original line rasterization is not yet
implemented here. The adapter enforces one active pattern instance, preserves
DOS-sized speed/frame state and the missile pattern's static captured target.
Added read-only native missile snapshots/count for behavioral verification.

Linux and ARM64 tests pass with matching line trace f090d6760cf06d3b. Pellet
pattern fires first on frame 184 (radius reached 112 at frame 176), produces
92 pellets and completes at frame 361; 33 puts/34 unputs and 93 SE commands.
Missile pattern captures player X at frame 224, ignores later target movement,
fires 40 missiles starting frame 248 and completes at frame 321; 28 puts/29
unputs and 11 SE commands. Tests verify previous-corner spawn positions,
original integer-velocity truncation and no RNG consumption in both patterns.
Counts are tested without projectile updates/removal to isolate spawning.

These attacks are not yet called by StageSession/Mima AI. Line rasterization,
remaining Mima patterns, HP/phase orchestration and full fight are unfinished.
No preview or MyMini deployment in this step; stage-10 intro boundary remains.
## Third Mima square attack routine

Appended original pattern_static_pellets_from_corners_of_two_squares verbatim
to the shared square include. Native wrapper adds two_squares selection and a
required casting/meteor render callback for this pattern. Original cast frame
50, opposite rotations, cloud-spawn switch and completion/meteor restoration
remain in shared source. Draw operations still emit geometry to the supplied
renderer; full graph_r line rasterization remains a prerequisite for live AI.

Linux/ARM64 three-pattern tests pass; combined line trace c9ff70a0ba6cdae3.
Third pattern: first eight cloud pellets at frame 184, interleaved previous
regular/flipped corner positions; 18 volleys attempted through frame 320,
completion frame 321, 56 square puts/58 unputs, 73 SE commands. Without actor
updates the original 100-pellet pool fills and rejects later additions; sound
commands still occur. Cloud-spawn flag resets each frame, cast/meteor events
are exactly true/false, and RNG is untouched. Finished advance is idempotent.
Not yet connected to stage-10 gameplay; no new preview/device deployment.
Remaining phase-one hop/chase pattern, phase-two/three AI, sprite/palette/line
rendering integration and boss completion are still unfinished.
## Original line raster loop and rendered Mima squares

Shared graph_r_line body in graph_r_line.inc; native build replaces only its
four-byte unput operation with bounded framebuffer byte copies. Native wrappers
provide color-mask writes and horizontal/vertical special cases. Preserves
sloped-line final cached-byte omission, original clipping arithmetic and the
advanced-coordinate final unput band. Real-mode faults/out-of-page memory at
edge offsets are not emulated; only in-page bytes are copied on native builds.

Added render_mima_square and made it the default renderer of MimaSquarePattern.
Tests now exercise actual square rasterization as well as geometry callbacks;
casting/meteor renderer is still required for the third pattern. Linux and
ARM64 line tests match eight-octant draw/restore trace a71124fe33e1f133 and pass
inclusive horizontal/vertical, page selection, clipping, omitted last byte and
sloppy restore-band checks. Mima tests pass on both with actual renderer active;
geometry trace stays c9ff70a0ba6cdae3, and no-callback default visibly draws.

Live StageSession still stops after Mima's intro; square attacks are not yet
part of a running boss fight. Remaining hop/chase, later patterns and full
phase/hit/palette/animation integration are unfinished. No device deployment.
## Original Mima hop-and-chase pattern

Shared original hop/chase body and broken vertical sprite transition in
mima_hop.inc/mima_vertical_transition.inc. Expanded b10j.cpp plus all four
shared Mima includes was checked against HEAD source and matches after
whitespace normalization. Native MimaHopPattern supplies real boss sprites,
page-one background snapshots, both-page raw restores, shared spread-in and
existing RGB4 palette operations. Original one-time direction RNG, hitbox
windows, off-center hop coordinates and alternating repeat direction remain.

Real-asset tests pass on Linux and ARM64 for all four ranks and two successive
runs. Each run completes after 176 calls and four bursts, with 20/32/40/48
PM_CHASE pellets at fixed speeds 30/35/40/45 (Q12.4), no projectile updates in
this scheduling test. Initial RNG becomes 015a4e36 and remains unchanged while
running/restarting. Checks include inactive initial hitbox/meteor, final active
hitbox/meteor, palette restoration and nine snapshots then eight on repeat.
Framebuffer trace matches 25ea7df247bd89c3. Inspected final image; both hosts
produce SHA256 4571ee38b24162532175afd69d9b151257fbf023c427a3b0e132386f570f7975.
Full Linux rebuild and suite: 67/67 pass.

This is a standalone attack adapter. Full mima_main hit/phase processing,
including periodic palette resets mentioned by original source, is not running
here. StageSession still stops after the intro. Later attacks, full boss fight,
Windows/device deployment and original-runtime frame comparisons remain.
## Original pillar-and-aimed-spread attack

Shared pillar row selection/drawing and full original pillar pattern in
mima_pillars.inc. Native MimaPillarPattern uses actual line/arc rendering,
generated pillar.bmp row masks and original pellet spawning/collision rules.
Required callback supplies casting/meteor sprite events. Added reproducible
pillar_sprites.inc generation and check, preserving original bitmap bytes.
Expanded Mima shared includes still match HEAD b10j.cpp after whitespace
normalization.

Added shared arc sloppy-unput body. Native explicitly seeds the first previous
coordinate sentinel rather than reading uninitialized host memory. The DOS
routine's initial stack-dependent skip behavior is not proven equivalent;
remaining arc loop/16-pixel restoration uses original logic. Out-of-page pillar
writes are safely discarded rather than emulating hardware memory.

Linux and ARM64 pillar tests pass across all four ranks: eight coordinate RNG
calls at frame 100, cast frame 50/meteor frame 240, first pillar collision at
192, 24 pellets across eight three-way aimed groups, 17 SE commands, finish
frame 351. Invincibility suppresses collisions; unprotected player is hit at
the first seeded column. Framebuffer trace matches 17e767c62c8ace13. Inspected
frame-200 column/warning-circle image; both hosts produce matching BMP hashes.
Shape tests cover single-angle aligned arc restore and reversed-bound no-op;
static-data regeneration check passes.

This pattern is standalone, not a running Mima fight. Casting sprites, actual
phase/hit orchestration, remaining attacks and StageSession integration remain
unfinished; no Windows or MyMini deployment in this step.
## Remaining three Mima square attack routines

Shared original halfcircle missile, reversing slow spray and aimed-laser bodies
in mima_square_late.inc. Native square runtime now retains original union views
for Q12.4/decimal/speed-times-eight values and rank selection. Original source
expansion still matches HEAD b10j.cpp after whitespace normalization.

Linux/ARM64 all-rank tests pass with trace 806d584c579fc753; prior three-square
trace remains c9ff70a0ba6cdae3. Halfcircle: first volley 184, forty missiles,
finish 341; decimal speeds deliberately truncate to 4/4/5/5 pixels. Slow spray:
first volley 184, sweep reversal after frame 270, ninety-six pellets, finish
371. Tests verify old-corner positions and rank-adjusted pellet velocities.
Laser: first spawn 181, rotation-frame spawn uses new/current corners at 184,
width 4, moveout age 20, speed truncation 6/6/7/7, finish 301. With no actor
updates, repeated spawn calls correctly leave only four already-live slots;
casting pose remains active at completion as in the original routine. None
of these attack routines consumes RNG. Rendering uses native original line
rasterizer; tests isolate scheduling and do not advance projectiles.

All eight Mima attack routine bodies now have native component adapters/tests.
This is not a full Mima boss port: original phase/hit/HP handling, shared sprite
animation/background/palette lifecycle and StageSession combat remain to be
integrated. Current preview still stops after the intro, and no new Windows or
MyMini deployment occurred in this step.
## Complete original Mima control routine compiled and exercised

mima_core now compiles original b10j.cpp, including all shared attack bodies,
HP/hit processing and phase orchestration. Native backend implements real
background PTN allocation/snapshot/raw restoration (including partial quarters),
monochrome pillar writes, line loops, sixteen-step white-in and explicit host
music/defeat callbacks. Native uses a real sized Mima entity rather than casting
a base object into a derived type, DOS-sized HP/frame/invincibility/union words,
fresh particle storage on MAIN reload and reset static hit/phase state at entry.
Original DOS branch expanded with shared includes matches HEAD after stripping
native branches and normalizing the int type alias/whitespace.

Full real-asset lifecycle test passes on Linux and ARM64: 12 HP load, 16 VSync
white-in/160 ms delay callbacks, 100-frame spread-in, real orb hit awarding 5000,
all four phase-one patterns, HP-six threshold deferred until pattern cleanup,
recenter phase and all four late patterns with actual actor updates, HP-zero
music/cleanup/defeat handoff exactly once, sprite/PTN/missile release, and fresh
reload with static hit state reset. Test player is invincible and orb position
is controlled; this is not interactive play validation. Original actor order
boss->lasers->pellets is retained by the test, with orb motion held fixed.
Linux full rebuild/suite: 70/70 pass. Inspected full-fight image; Linux/ARM64 BMP
SHA256 matches 058db7a73fee633e0f29d0f0caf846a7904e4cfb99319c782ed7713c78b42ad4.

StageSession/SDL still runs only the earlier diagnostic intro at stage 10.
Next work is binding full mima_core into live frame/Continue/defeat progression.
The existing particle/arc/line hardware-UB safety boundaries remain documented;
original DOS frame comparison, Windows update and MyMini deployment are pending.
## Playable Mima integration, Continue and post-boss scene

StageSession now binds full mima_core through MimaGameplay instead of the
spread-in-only diagnostic. Boss hooks dispatch by actual stage/route. Mima
load runs its white-in on a cleared black palette before BOSS3 background/HUD,
then real entrance and live attacks. RAII controller is destroyed before parent
bindings, including exception unwind. Normal miss retains boss state; Game
Over/Continue reloads fresh Mima and preserves stage/route. Defeat uses original
boss animation, retains Jigoku without another route menu, frees banks, runs
TOTLE once and loads selected-route scene index 4 for stages 11-14. Scene history
and resident score/lives/bombs survive handoff. Stage 15 Kikuri remains explicit
boss_pending; Makai stage 10 remains unported. Direct starts support 11-14 for
scene diagnostics. Mima status is available for integrated verification.

Linux full rebuild/suite: 72/72 pass. Integrated test covers Mima Game Over/
Continue, phase threshold, real defeat animation, exact TOTLE award, resources
at stage 11, correct STAGE4 layouts/timer 1400 through 14, and Kikuri boundary.
The same integrated test passes on ARM64. Windows rebuilt with full Mima and
TH01 Native Mima.cmd launcher. SDL Linux/ARM64/Windows each completes 600 host
ticks, 984 presentations, state=playing with matching gameplay BMP SHA256:
b5d06797fc062c2088a2071393a692be7c9ab62d934d7218249b74a3ee4344cf.
Inspected gameplay image. Integrated stage-11 BMP matches Linux/ARM64:
3ba977721961d2daa48058c5fe1019721dc09fdf75ef9f3330ece6feddab6852.
No interactive human play or MyMini installation/hardware validation yet.
Audio playback, complete title/settings/ending and remaining bosses are still
unfinished; existing native repairs for original undefined hardware behavior
remain documented in earlier sections.
## Kikuri ripple rendering/hitbox prerequisite

Shared original tear_ripple_hittest and ripple animation bodies in
kikuri_ripple_hit.inc/kikuri_ripple.inc; expanded source matches HEAD b15j.cpp
after whitespace normalization. Native ripple binding resolves TAMAYEN.PTN
separately from STG, reuses existing quarter alpha renderer and original
16-pixel-aligned sloppy unput for each ripple column. Nested shot binding is
scoped to the draw call, preserving outer SE commands and graphics page state.
Exports the same ripple put/flag backend for future full Kikuri integration.

Real-asset test passes on Linux/ARM64 for internal X 304, 312 and 307. Starting
at original tear frame 1, ripple finishes after 74 updates with three SE-7
commands, flag reset/page-zero invariants and full final background restore.
Tests preserve inclusive internal-coordinate hitbox, its disparity from
byte-aligned sprite X, and invincibility. Trace matches 1cbe441e28192877.
Inspected aligned/striped frame-24 images; cross-host BMP hashes match:
304 5980481e83390f34bc36744e487272fe69f825a3159329b3ba899c291683b495
312 c52f2c2e6b8f1544a733374c62718c80f018c99043f24ce28a53097a117f4a3a.

Kikuri load/entrance, soul/tear movement, attacks, HP phases and stage-15 live
integration are not implemented yet. Preview remains at Kikuri boundary after
stage 14; Mima playable build is unchanged. No device deployment in this step.
## Playable Kikuri and final Jigoku regular scene (2026-10-29)

Full original b15j core is now bound through KikuriGameplay into StageSession.
Real BOSS4/TAMASII/TAMASII2/TAMAYEN assets drive the original 752-wait entrance,
soul/tear/ripple attacks, cross lasers, HP thresholds and four final patterns.
Boss-hit palette restoration follows Kikuri's changing palette; defeat uses
its original black post-boss palette. Continue reloads stage 15 with fresh boss
state. Defeat frees boss resources, runs TOTLE once, then loads STAGE6 for
Jigoku stages 16-19, preserving resident resources and resetting interlace.
Stage 20 remains explicit boss_pending for unported Konngara. Makai bosses
remain unfinished. Direct preview starts now support stages 6-19.

Linux regression suite: 76/76 passed. Integrated Kikuri Continue/phase/defeat/
TOTLE/stages-16-19 test also passed on ARM64 under PC-side qemu. Windows rebuilt
with TH01 Native Kikuri.cmd. Linux, ARM64 and Windows SDL dummy-driver runs
complete 600 host ticks / 1719 presentations in playing state; gameplay BMP
SHA256 matches fcb70653094caece1b89d706db53278fc9ba5080b1f1ed8f3e4fe6aa8b4ac1c5.
Inspected gameplay and stage-16 images. Stage-16 Linux/ARM64 BMP SHA256 matches
8a95da5483c207874b22488e446f4eec988d1d2dc349606078d86f200f3cad4a.

These are controlled automated checks, not original-DOS frame-parity proof or
interactive MyMini validation. Audio playback, complete title/settings/ending,
remaining four bosses and device deployment remain outstanding.
## Konngara prerequisite: GRZ/GRX decoding (2026-10-29)

Added native/grz.hpp, a data decoder following th01/formats/grz.cpp. Validates
HGRZ/HGRX headers, image/stream counts, offsets, RGB4 palette and stream bounds.
Shared RLE expands to opaque eight-pixel units with interleaved B,R,G,E bytes;
zero color bits replace the destination while skipped units preserve it.
Preserves the original extra unit after nonzero runs, including skip runs,
and the single skipped unit for a zero-length run. Decodes multiple planar
streams without repeating command interpretation. No PC-98 instructions run.

Synthetic tests cover these run rules, plane/stream ordering, opaque zero vs
transparent units and rejected bad indices/truncation/planar underrun. Both
new Linux CTest cases pass. Real BOSS8.GRZ contains 16 images, all decoded on
Linux and ARM64; all 16 output BMP hashes match across hosts. Images 7 and 15
were inspected. This is source-based decoding with cross-host checks, not a
comparison against original DOS captures. Palette presentation/context for
these effects will be supplied by the live boss backend.

Konngara gameplay, entrance integration and final ending remain unfinished.
Current Windows playable preview is unchanged at Kikuri/stages 16-19.
## Konngara original entrance adapter (2026-10-29)

Extracted konngara_load_and_entrance into shared konngara_entrance.inc. Expanded
b20j.cpp matches HEAD byte tokens (verified with git bytes, not CP932-to-UTF8
terminal decoding). Native KonngaraEntrance runs that body against framebuffer,
GRP/GRZ/BOS readers and required host callbacks for boss-stage object reset,
waits and music requests. Implements decelerating scroll, palette ramp, shake,
page-one-to-zero pan and Siddham color-key overlays/fade. Loads three entity
banks and seven slash effect slots. Original stage-palette snapshot remains
BOSS7_D1 rather than BOSS8_A1, preserving the documented pause/bomb color quirk.
SE bank reload is visually inert; four SE-9 commands are recorded, not played.
The original pan rectangle starting at y=400 is discarded outside displayed
VRAM; this is an explicit native safety boundary, not a bus-level simulation.

New real_konngara_entrance test passes on Linux and ARM64. Verifies 498 wait
calls / 710 total frames, initial quarter-threshold scroll trace, music/reset
requests, slots, interlace, final scroll zero, synchronized pages, palette #9
ending at #0099AA, stage palette and repeat-load state. Inspected final image.
Linux/ARM64 BMP SHA256 matches:
42a150b561d9c19d2cf5c9f882f6c370859f398538e4df13da9d1840b1d079b8.
No original-DOS capture or MyMini hardware comparison yet. Konngara attacks,
HP/defeat, live session integration and ending are still outstanding. Existing
Windows preview remains the verified Kikuri/stages-16-19 build.
## Konngara first two original attacks (2026-10-29)

Shared konngara_early.inc now contains original diamond-cross/edge-rain and
symmetrical cup spread functions; expanded b20j.cpp still matches HEAD tokens
including prior entrance extraction. KonngaraEarlyPattern runs the bodies
with DOS-sized frame/pattern union storage, real CPellets and native diamond
render/unput. Expression changes are required renderer callbacks, ready to
bind to the live head/face controller. Only one pattern instance is allowed,
matching original static attack state. Completed advance calls are idempotent.

Linux/ARM64 tests pass across all four ranks: frame-ten expression changes,
no early projectiles, original muzzle positions, rank-adjusted speed, cup
completion at 300 with 90 pellets/no RNG, diamond completion at 435 for the
fixed test player, aimed opening group sizes, rank-dependent rain intervals,
nine coordinate RNG calls per five-projectile burst and one SE-12 command.
Combined velocity/framebuffer trace matches bcae5cbfa3a18813. Targeted Linux
Konngara entrance and early-attack CTests both pass after final assertions.
Tests hold projectiles at spawn to inspect complete emissions without actor
movement/collision or capacity losses; live moving-actor validation is pending.

Remaining Konngara attacks, face-controller binding, HP/defeat and live session
integration are unfinished. No Windows preview update or MyMini deployment.
## Konngara two/four homing snakes (2026-10-29)

Shared original Snakes template/macros and two/four-snake attack bodies now
compile into konngara_early_core. Expanded original b20j.cpp still matches
HEAD tokens after all five include extractions. Native adapter retains source
trail render-before-update order, wobble angles, target tracking/locking,
sliding-dependent collision, invincibility, spawn schedule and rank-dependent
semicircle volleys. DOS far qualifiers are omitted only in native compilation.

Linux/ARM64 tests pass for both attacks on all four ranks. Paired attack ends
at frame 500, four-snake attack at 400; paired volleys first occur at frame
280/240/200/160, with 6/7/8/9 twenty-pellet bursts. Verifies 750 plus volley RNG
calls versus 900 for four snakes, original SE-12/SE-7 schedule, no pre-frame100
activity, invincibility and unprotected hits in normal/sliding states. Emitted
pellets are inspected then removed each tick to avoid the 100-slot pool hiding
volley counts; this test does not advance those pellets. Snake movement/render
and direct snake collision run the original logic. Combined trace matches
6673645ef6a94544; inspected four-snake frame-240 image. Linux/ARM64 BMP hashes:
two 548324a8d277f598651d1a956a0ab2b530f0b4143a70ba3f99f45ce45717751f
four a7f31ee3f6e966c13567f3cd9d85a36628f331eff2e980110a02f9060ded2db9.

Remaining row/spray/ray/slash/laser attacks, head/face live rendering, boss
HP/defeat/session and ending remain unfinished. No Windows/MyMini update.
Full Linux CTest regression after snake additions: 81/81 passed.

## Konngara aimed rows and cup spray (2026-10-29)

Shared konngara_rows_spray.inc adds the original aimed-row diamond and aimed
cup-spray functions to the native pattern adapter. Native enum increment uses
an explicit enum cast rather than aliasing an enum as host int; original DOS
branch remains unchanged. Expanded six-include DOS b20j matches HEAD tokens.
Original aimed-row speed grows by 0.125 pixel per shot. Cup spray preserves
the offset aim origin, byte-wrapped offsets and reversal after nine shots.

Tests pass on Linux/ARM64 across four ranks. Rows follow y=64/128/192, fire
only on horizontal movement, and finish at frame 335 with 18/20/24/33 pellets;
checks each rank interval, muzzle coordinate, aimed velocity and cumulative
speed increment. Spray finishes at 700 with 121/151/200/301 pellets; checks
cadence, first ten offset/reversal velocities, no RNG and no extra SE. Both
attacks retain frame-ten expression changes. Emissions are inspected and
removed per tick, without projectile movement; combined geometry/velocity
trace matches 16b8d405ba4a2f69. All four targeted Konngara Linux tests pass.

Konngara now has six isolated original attacks available. Sword rays, slash
patterns, lasers, complete head/face rendering, HP/defeat and live session
integration remain unfinished. Windows playable preview and device unchanged.
## Konngara sword-ray edge rain (2026-10-29)

Shared konngara_swordray.inc adds original sword-ray endpoint movement and
edge-rain attack to the native adapter. Uses original spawnray macro plus
existing original line draw/unput backend. The macro header skips DOS hardware
includes on native; a null statement after the return label makes it valid
C++17 on the older ARM64 compiler without changing behavior. Native previous
endpoint belongs to the pattern instance, avoiding stale pointers on reload.
Expanded seven-include DOS b20j still matches HEAD tokens.

Linux/ARM64 four-rank tests pass: aiming expression frame ten, inactive warning
phase, endpoint path and post-movement projectile origin, rank cadence and
speed, random half-circle angles, gravity motion with one-subpixel acceleration,
360-frame completion, exact 106/174/262/262 emissions and 27 SE-6 commands.
Preserves original extra volley/SE on the last tick after frame reset to zero.
Full final framebuffer equals its background page. Inspected frame-180 ray.
Combined velocity/framebuffer trace matches 97851c4e0de7dc3e. All five targeted
Konngara Linux tests pass. Projectiles are inspected/removed after spawning;
moving-pellet integration remains pending with the live boss.

Konngara has seven isolated original attacks. Slash animation/patterns, lasers,
head/face controller, HP/defeat/session and ending still require work. No new
Windows preview build or MyMini deployment in this step.
## Konngara three slash attacks and original GRZ keyframes (2026-10-29)

Shared konngara_slash.inc / konngara_slash_aimed.inc provide original slash
animation plus rain, triangular and aimed attacks. Native KonngaraSlashPattern
uses DOS-sized frame/union storage and required callbacks for face centering,
expression and both-page slash rendering. Exposes direction-change lock state.
Keeps signed integer division along the triangular emitter path and original
union aliasing. Expanded nine-include DOS b20j still matches HEAD tokens.

Real BOSS8.GRZ/BOSS8_A1 assets tested on Linux/ARM64 across all four ranks and
three attacks. Checks seven image events at 50/60/100/120/140/160/170, one SE-8,
face lock at 10 through 200 and unlock/completion at 201, emitter origin and
corner snaps, rank-adjusted velocities, aimed directions and random rain
angles. Rain totals 50/66/100/140; triangular 66; aimed 33. Both pages match
and compose the expected seven GRZ overlays; emitted pellets are inspected
then removed, so moving-projectile integration remains pending. Combined
velocity/framebuffer trace matches a88ceb37beff2e86. All six targeted Konngara
Linux tests pass; inspected frame-140 composition. No original DOS capture or
MyMini hardware validation, no Windows preview update.

Konngara now has ten isolated attacks. Laser/three-spread and sleeve rain,
complete head/face rendering, full phase/HP/defeat/session and ending remain.
## Konngara laser sweep and sleeve semicircle (2026-10-29)

Shared konngara_lasers.inc and konngara_sleeve.inc add the last two original
attacks to the isolated adapter. Expanded eleven-include DOS b20j still matches
HEAD tokens. Laser sweep keeps rank-specific integer delta (128/96/80/64),
random direction, modulo-ten laser slots, final boundary volley and simultaneous
three-way aimed pellets. Sleeve rain alternates gravity and fall-from-top
motion using the original byte-wrapped angle decrement.

Linux/ARM64 tests pass across four ranks and both laser sweep directions.
Lasers are actually updated/rendered per tick: original origin/target, eight-
pixel extension/moveout, 30-frame extension period, width-five mask and white
color; 6/8/9/11 volleys, one direction RNG and one SE-6 per volley. Safe cleanup
fully restores the background. Pellet volleys are inspected then removed.
Sleeve emits 110 gravity and 100 fall-from-top pellets through frame 300 with
one-subpixel acceleration / three-pixel fall speed, no RNG and 21 SE-7 commands.
Combined trace matches 2e50df90b0b9f64d. Inspected live laser frame-150 image;
Linux/ARM64 BMP SHA256 matches
c3c2f74dcf5406a982dca4a4acc69e771b439117aa56199882e75bc535e8e074.

All twelve original Konngara attacks now have isolated native adapters/tests.
This does not complete the boss: full phase selection/HP transitions, live
head/face rendering, defeat cutscene and StageSession integration are pending.
Windows playable preview and MyMini device remain unchanged.
Full Linux rebuild and regression after all attack adapters: 85/85 passed.

## Konngara shared head/face controller (2026-10-29)

Extracted original face_direction_set_and_put / face_expression_set_and_put
into konngara_face.inc. Expanded twelve-include DOS b20j matches HEAD tokens.
Native KonngaraFace uses original CBossEntity rendering with BOSS8_1/E1/E2
banks in slots 0/1/2. Supports all expressions/directions, strict player-left
tracking thresholds (<198 / >396), direction lock independent of expression
changes, redraw of eyes on direction change, and RAII bank release.

Real-asset Linux/ARM64 tests pass: 12 direction/expression combinations versus
independent pixel-level BOS mask/color composition on both pages; repeated
states, locked tracking, neutral head restoration while locked, exact boundary
values 197/198/396/397, glare/aim overlays after changing direction, and bank
cleanup/reload. Combined framebuffer trace matches 800eff8f08a5ace3. Inspected
final head/aim image. Linux/ARM64 BMP SHA256 matches
823c72c4946ae05dc75793866c3a284a47eafd415a968e01b44576cdcc14bb6b.
All eight targeted Konngara Linux tests pass. No new full-suite run is claimed.

Complete Konngara phase/HP controller, defeat cutscene and StageSession binding
remain unfinished. No updated Windows playable preview or MyMini deployment.
## Konngara original defeat sequence (2026-10-29)

Shared konngara_kuji.inc / konngara_defeat.inc contain the original nine-image
flashing helper and blocking defeat body. Extended entrance graphics adapter
runs them with original GRZ decoding, scroll and palette operations, required
release/stop/fade callbacks, resident-score output and game-clear flag. Release
is called twice as original; native owned banks safely clear twice. Completed
run_defeat is idempotent. Expanded fourteen-include DOS b20j matches HEAD tokens.

Linux/ARM64 real-asset tests pass: nine alternating palette-zero black/white
images each held ten frames, 337 explicit wait calls totaling 418 frames,
44 SE-7 commands, music stop/fade, zero final scroll, all-black final palette,
freed entity/effect banks, 50000 score bonus exactly once and game-cleared flag.
These model explicit waits only: DOS printf/text-clear incidental execution
time is not reproduced. Text control commands clear native overlay; the
synchronous background load has no intermediate host presentation. Inspected
first Kuji image. No original-DOS capture or MyMini validation. All eight
Konngara Linux CTests pass; ARM64 entrance/defeat test passes independently.

Remaining work is compiling/binding full original phase/HP controller and
StageSession lifecycle; game ending beyond this boss-clear flag is unfinished.
No Windows playable preview change or device deployment.
## Full original Konngara core and phase lifecycle (2026-10-29)

b20j.cpp now compiles directly in konngara_core under its native namespace.
Original attack functions and full phase/main hit logic run together, with
DOS-sized frame/HP/invincibility/pattern storage and typed rank helpers.
Backend binds shared entrance/defeat graphics, actual BOS entities, dynamic
boss palette, original stage-palette snapshot, HP hit integration and original
GRZ overlays. Required lifecycle callbacks/framebuffer identity validated.
Observations expose current pattern/count without changing selection. Native
update stops after game-clear handoff. Expanded DOS branch matches HEAD after
native-branch removal and DOS int-alias normalization (build checker).

Linux and ARM64 full real-asset lifecycle tests pass: original 710-wait-frame
entrance, HP18/phase0, real orb hit for 10000 and HP17, original phase selection
covering all first-phase patterns, four phase-three patterns, three phase-five
patterns and all twelve final-phase patterns. Native actors advance in original
boss->lasers->pellets order with invincible test player and controlled orb.
Threshold tests explicitly set HP15/12/9 to exercise transitions; HP-zero plays
418-frame defeat and adds 50000 once, releases resources and fresh load resets
static lifecycle state. Coverage ticks match Linux/ARM64: phase1 2175, phase3
4596, phase5 1664, phase7 7115. Inspected live battle image; BMP SHA256 matches
8c49cad67a355452f6bda214e58c54c9464812ba5057e25381a05d05c41607ed.
These are controlled automated/source checks, not DOS capture parity or human
play validation. Original hardware safety boundaries remain in shared backends.

StageSession still stops at stage20 boss_pending. Thin gameplay bridge,
Continue/final-game-clear state and Windows preview update are pending; no
MyMini installation. Complete title/audio/ending and Makai bosses remain.
Full Linux rebuild and regression with complete Konngara core: 87/87 passed.

## Playable Konngara stage20, Continue and ending handoff (2026-10-29)

StageSession now binds full Konngara core through a thin KonngaraGameplay
PImpl, destroyed before parent graphics/hit bindings. Stage19 advances into
Konngara; direct Jigoku stage20 starts are supported. Boss palette callbacks
follow Konngara core, original entrance loads graphics before HUD/player setup,
and existing empty boss-stage object initialization precedes the entrance.
Normal miss retains boss HP; GameOver/Continue frees/reloads original boss.
Final clear is prioritized over regular-frame stage bonuses/miss handling,
retains original 50000 award and stops at explicit ending_pending. Ending is
not implemented; preview labels the boundary and permits closing. No extra
TOTLE or stage-clear bonus is awarded. Entrance/defeat SE commands are drained
through the session observer; actual audio remains unfinished.

Linux full rebuild/CTest: 89/89 pass. Integrated test verifies original stage20
HP18/timer8000, sound drain, normal miss retaining HP17, exhausted lives, fresh
Continue with reset stock/score, live HP thresholds through phase7, real defeat
and idempotent ending handoff. Same test passes ARM64. Updated Kikuri scene
progression test reaches full Konngara after stage19 on Linux/ARM64.
Windows rebuilt with TH01 Native Konngara.cmd; SDL Linux/ARM64/Windows each
completes 600 host ticks / 1578 presentations in playing state. Inspected image;
all three gameplay BMP SHA256 values match
ad02f597a7a99bef03db4f90b2a3126fd29372f374b3f2b054933b4b1bdc6ff2.

All Jigoku bosses are available in the native preview. This is not complete
TH01: Makai bosses, full title/settings/audio/ending, original-DOS frame parity
and interactive MyMini hardware validation remain. No device deployment.
## YuugenMagan eye entity/setup prerequisite (2026-10-29)

Started Makai stage10. Shared yuugenmagan_eye.inc/eye_helpers.inc/setup.inc
contain original eye class, geometry/constants, tracking/render helpers, BOS
load and setup. Expanded b10m matches HEAD tokens. Native YuugenmaganEyes
allocates five real CEyeEntity objects rather than original base-object casts,
uses BOSS2.BOS slot0 shared metadata and original lock/render-before-track order.
Supports flagged cel updates and tracking; rendering still follows original
three-frame lock behavior. Constructor/destructor owns the shared bank only
while caller's BossEntityBinding/ShotBackendBinding remain alive.

Linux/ARM64 real-asset tests pass for all five original positions, hidden and
inactive-hitbox initial state, seven cels, exact +/-32 player-tracking thresholds,
left/right beyond threshold, independent eyes, zero/selected flags and cleanup.
Combined framebuffer trace matches 1364651d0e51add5. Inspected BOSS2.GRP eye image;
Linux/ARM64 BMP SHA256 matches
c2ca85aa23a3f1f21dd2ae890a0b38dee712f6e5a62c55aa3b9eec7450daab0d.
This does not implement YuugenMagan entrance, attacks, phases or gameplay yet.
Windows/MyMini remain at the prior playable Jigoku/Konngara build.
## YuugenMagan original phase-zero opening/lasers (2026-10-29)

Shared rank helper, phase-zero helper functions and complete phase-zero body
now run in native eye adapter with existing HP, palette and line backends.
Expanded six-include b10m matches HEAD tokens. Eye rendering remains before
cel changes, with original lock latency. Palette/stage recolor, opening/closing
hitbox flags, downward laser drawing/unput and original phase-one handoff run
from the source body. No missiles are spawned by this isolated opening.

Linux/ARM64 four-rank tests pass: 331-frame duration; lateral/south/north cels
and active-hitbox schedule; strict left-laser bounds, enlarged east-laser box,
invincibility; eight color15 increments ending #DD5; Hard-only 15 pellets at
240 and Lunatic-only 25 at 260, preserving unreachable nested rank condition;
no RNG, lateral-only active eye hitboxes and idempotent completion. Pellet
volleys are inspected then removed, without projectile movement. Combined
framebuffer trace matches e4d9f5a1948ef403. Inspected frame-220 laser image.
Both YuugenMagan Linux CTests pass. No updated Windows preview/MyMini deploy.

YuugenMagan attack phases after opening, HP/defeat and StageSession integration
remain unfinished; current playable preview still covers Jigoku bosses only.
## YuugenMagan lateral aimed pellet attack (2026-10-29)

Shared yuugenmagan_lateral.inc contains original fire_from_lateral and
phase_1_pellets_from_lateral. Native eye adapter can advance this isolated attack
after original opening, updating lateral-eye lock/render/tracking first.
Expanded seven-include b10m still matches HEAD tokens. Full HP/phase transition
logic is not part of this adapter yet.

Linux/ARM64 tests pass for two cycles across all four ranks: periods
350/300/200/130; six firing offsets 10/25/40/60/80/110; both pupil-bottom muzzle
coordinates (92,160)/(540,160); five two-way wide aimed volleys followed by one
single aimed volley per eye. Verifies six base speeds plus rank adjustment,
+/-12 angular spread, different original aim offsets for groups vs singles,
retargeting after changing player position, 44 total pellets in two cycles,
no RNG and no SE commands. Pellets are inspected at spawn then removed; live
projectile movement and full phase-one HP progression remain pending. Combined
opening/framebuffer/projectile-velocity trace matches 77afe1f06db3bdb9.
Both targeted YuugenMagan Linux CTests pass. No Windows/MyMini update.

Next source area is southern aimed missile pairs and following phase changes.
## YuugenMagan southern missile pair patterns (2026-10-29)

Shared yuugenmagan_missiles.inc contains original aimed-missile offsets,
angle helpers, both shift variants and complete missile-pair pattern macro.
Native eye adapter selects isolated away/towards (three variations) or clock
(two variations) attacks, with caller-bound original MissileBackend. Loads
BOSS3_M.PTN, resets missiles, tracks southern eyes, snapshots target at frame100
and preserves original macro subphase/frame reset. Expanded eight-include b10m
matches HEAD tokens. Full HP/phase switching is still pending.

Linux/ARM64 tests pass for both modes, all variations and four ranks: prepare
closing at 90, one RNG/target snapshot at 100, first volley104, eight-frame
cadence, eleven pairs per shift section, locked target after player moves,
byte-wrapped angles, original offset source (220,204)/(412,204), integer speed8,
35 pairs through tick369 and reset to preparation/frame0. Combined emission
velocity trace matches 2254495764de0a03. Separate sustained runs use original
missile-before-boss order, keep live missile population, observe explosions,
then drain all slots with no invincible-player hit. Emission-count runs inspect
and clear after one real motion/render update to avoid fixed pool capacity
hiding pairs. All three targeted YuugenMagan Linux CTests pass; ARM64 missile
test passes independently. No new Windows playable preview or MyMini deploy.

YuugenMagan later sprays, crossing lasers/pentagrams, HP/defeat and StageSession
remain unfinished. Current playable preview still has Jigoku bosses only.
## YuugenMagan patterned line bullets and three-laser sweep (2026-10-29)

Native linebullet_core compiles original th01/main/bullet/line.cpp, with hardware
include/unput adaptation only. Added patterned line entry point to existing
original line renderer, resetting pattern afterwards (including exceptions).
Tests preserve endpoint-only strict collision, not collision along the whole
visible ray; three-dot pattern, regular-line pattern reset and sloppy restore.
Linux/ARM64 line-bullet trace matches 9f723098f5d89f88; old line test also passes.

Shared yuugenmagan_lasers.inc / laser_dispatch.inc run original phase-nine
single, reverse and dual sweep sequence in eye adapter. Expanded b10m matches
HEAD tokens. Tests pass across four ranks on Linux/ARM64: 40-frame preparation,
first sweep ends tick189, reverse at378, dual at491, including post-reset
16-pixel offset increment; Hard 9/Lunatic11 completion pellets, no RNG,
invincibility and real endpoint hit on unprotected player. Original interleaved
draw/unput dual-ray visibility defect is retained. Completed advance is inert.
Combined framebuffer trace matches 9b2bebfe5192a763. Inspected dual-ray image;
Linux/ARM64 BMP SHA256 matches
a207f8347b149c6e15ed28dc8d21727a8755d520bf93439aeb73d70b14d4c7c4.
All four targeted YuugenMagan and both line Linux CTests pass. Emitted completion
pellets are inspected then removed; full HP/phase progression remains pending.

Remaining YuugenMagan work includes sprays, pentagrams, palette transitions,
full main/HP/defeat and StageSession. No Windows preview/MyMini update.
## YuugenMagan circular lateral spray (2026-10-29)

Shared yuugenmagan_spray.inc contains original phase-five attack body between
frame-common and HP-hit processing. Native isolated adapter sets original
12/8/4/2 rank interval, advances the frame and runs that body with DOS-sized
subphase counter and byte angle. Expanded b10m matches HEAD tokens. Preserves
mirrored eyes, per-frame decreasing angle, first regular spray, pause, -15
second-spray subframe and fall-from-top motion; HP transitions remain separate.

Linux/ARM64 tests pass for two complete cycles across all ranks: first spray
starts100 and ends280, second starts331 at subframe-15/angle128, full cycle446;
892 total ticks, 104/148/300/596 emitted pellets in two cycles, including
4/4/12/28 emissions at negative subframes. Checks both muzzle positions,
mirrored wrapped angles, four-pixel base speed plus rank, motion fields, frame/
subphase/angle reset, no RNG and no SE. Pellets inspected then removed; live
projectile integration remains pending in the full boss. Combined emission
trace matches a4e2bcff74753047. No Windows preview or MyMini change.

YuugenMagan pentagrams, palette/eye transitions, full HP/defeat/main session
integration remain unfinished. Existing playable preview covers Jigoku bosses.
Full Linux rebuild and regression after YuugenMagan spray/line backends: 95/95 passed.

## Unverified pentagram shrink checkpoint (2026-10-29)

Added shared yuugenmagan_pentagram.inc geometry and yuugenmagan_shrink.inc
phase-eleven shrink dispatch, plus native begin/advance/state API and
real-asset yuugenmagan_pentagram_test. Linux target compiled and linked; binary
exists. Runtime assertions, screenshot inspection, source-equivalence check,
ARM64 build/run and regression have NOT been completed for this change.

The first Linux test execution was not launched: automatic approval review
failed because account usage was exhausted (message offered retry at 8:30 PM,
without a verified timezone). This was a review infrastructure failure, not
an unsafe-action determination. No retry through another execution path.
ARM64 build in that sequential tool script was not reached. Last fully
verified full Linux suite remains 95/95 from before pentagram changes.

Next: run build/linux/yuugenmagan_pentagram_test with build/assets/th01-ja and
BMP prefix; fix actual failures before further features. Then compile/run ARM64,
compare/inspect images, expand YuugenMagan includes against HEAD, relink affected
tests and regress. Preserve original shrink overshoot instead of clamping it.
Goal remains unfinished; Windows playable preview/MyMini unchanged.
## Verified pentagram eye-connection/shrink checkpoint (2026-10-29)

Approval review resumed accepting commands after its reported retry time;
completed the previously deferred checks without changing execution path.
Linux and ARM64 real-asset pentagram tests pass, trace e54858c632e19b24.
Original shared phase-eleven dispatch connects off-center eye corners at100,
starts shrinking after140, advances eight pixels every fourth frame, remains
active at exact distance160/frame217 and finishes at distance168/frame221.
Tests check initial/final five corner coordinates, original integer factors,
no RNG/SE, endpoint positions above the player and idempotent completion.
Retains original eye-unput holes and off-center/overshoot behavior. No CPU
emulation in port logic. Expanded YuugenMagan b10m source still matches HEAD.

Inspected final frame221 image. All three BMPs (100/181/221) match Linux/ARM64.
This supersedes the unverified/approval-blocked checkpoint above. Regular
pentagram spin/slam phases, transitions and full boss HP/defeat/session remain
unfinished. No updated Windows playable preview or MyMini deployment.
Full Linux rebuild and regression after pentagram shrink verification: 96/96 passed.

## YuugenMagan regular pentagram spin/grow/slam (2026-10-29)

Shared yuugenmagan_regular.inc contains original phase13 radial firing and
pentagram attack dispatch. Native adapter initializes isolated post-relocation
center (316,189), radius64 and original corner pose, then runs both prepare/spin/
slam and prepare/grow/slam paths. Native enum increments use explicit casts;
expanded DOS b10m matches HEAD after filtering native-only casts. Standalone
respawn handoff sets phase10/new prepare state and unputs the shape; it does not
yet implement the palette/eye transition and re-entry sequence from full main.

Linux/ARM64 four-rank tests pass: first preparation updates spin/growth at
frame0 in same tick, all five alternating spin phases, radius115 after original
frame-zero growth, original slam movement/render order and phase10 handoff at
y>=420, idempotence, no RNG and invincible player. Fixed-player spin path takes
1332 ticks with 44/77/110/137 radial volleys; growth path532 with 12/21/30/37.
Each volley has five real pellets from pre-update corners with outward velocity
and rank speed; inspected then cleared, not advanced as moving actors. Combined
framebuffer/emission trace matches ea20878fd8169e91. Inspected growth/slam image.
Linux/ARM64 images match:
spin c58e8f6c20e3774fd355305a44e29c3dd8e4f2633644e14bca9e328629b92a04
grow 877a1612ecdce8c86ec58b0a8ee43279d3d33e7f47600c9a3cc76c2ab9c0443f.
All seven targeted YuugenMagan Linux CTests pass; no new full-suite claim.

Full YuugenMagan palette/eye transitions, phase/HP hit and defeat controller,
StageSession/Continue and Windows preview remain unfinished. MyMini unchanged.
## YuugenMagan eye/palette transition helper (2026-10-29)

Shared yuugenmagan_transition.inc contains original eye toggle/recolor function.
Native component proxy preserves signed8 wrap for both current/stored RGB4
updates; general palette backend still clamps displayed colors separately.
This avoids silently widening the boss's repeated color arithmetic to host
integers. Component accessor macro expands back to original DOS .v[index];
expanded b10m matches HEAD after native-branch filtering/accessor expansion.
Native API runs isolated close/open flag transitions for original70 frames.

Linux/ARM64 tests pass: exactly63 recolor steps (every tenth frame skipped),
stored increment beyond15 versus clamped display, eye/hitbox events20/40/60,
original successive palettes DD5->(0,13,68)->(63,0,68)->(0,0,68)->(0,63,5),
then repeated red increments63,126,-67,-4,59. Checks negative values clamp to
black, component-count sentinel toggles eyes without recoloring, no RNG/hit,
and idempotent completion. All eight targeted YuugenMagan Linux CTests pass.
No new full-suite claim. Existing Windows preview/MyMini unchanged.

All major isolated attack/transition helpers are now present. Full original
YuugenMagan main/HP/invincibility, defeat handoff and playable StageSession
integration remain to be compiled and verified together.
### Playable Makai stage ten / YuugenMagan (2026-10-29)

StageSession now starts YuugenMagan from Makai stage 9 progression and direct
`--stage 10 --route makai`. A thin PImpl gameplay owner keeps original missile
and boss bindings alive without exporting compatibility macros into the session.
Boss hit palette restoration routes to the active YuugenMagan palette; generic
defeat retains the original BOSS2 background palette. Continue reconstructs a
fresh boss. Defeat commits original stage-ten TOTLE once, frees boss resources,
and loads Makai STAGE3.DAT / ST3.GRP stages 11-14. Stage 15 remains explicit
boss_pending for unported Elis.

Validation: Linux full build, 101/101 CTest passed, including route progression,
YuugenMagan live Game Over/Continue, final phase, defeat/TOTLE, retained resident
resources, Makai layouts/timers through stage 14 and Elis boundary. ARM64 session
test passed under host-side QEMU. Windows preview rebuilt with the full core.
Windows/Linux/ARM64 SDL preview all ran 600 ticks / 868 presentations / playing;
BMP SHA256 `84fb90f4f0feb29271a74704c509d9c132637de851c3d723cb506af9b683680f`.
Linux/ARM64 integrated final-phase session BMP matched
`6d2307c45b2980c41999cebe70373dabd7b116464aa66739a6d0e8af9d7e65fd`.
Inspected Windows preview and Linux session snapshots. DOS source-path audit
still passes. No MyMini deployment or original-DOS frame parity claim.

Windows launcher: `build/windows/TH01 Native YuugenMagan.cmd`.
Remaining full-port work includes Elis/Sariel, real audio playback, title/settings,
ending, accuracy validation and MyMini hardware testing.
### Playable Makai stage fifteen Elis and next scene (2026-10-29)

StageSession now starts Elis through Makai stage 14 progression or direct
`--stage 15 --route makai`. PImpl gameplay owner retains missile/boss bindings,
restores Elis hit palette, preserves original black post-defeat palette, and
runs original entrance/main/defeat. Game Over/Continue rebuilds fresh Elis at
the same stage. TOTLE commits stage-fifteen bonus once and enters Makai stage 16,
using STAGE5.DAT / ST5.GRP through stage 19 (timers 1600/1600/1600/600). Sariel
stage 20 is still an explicit boss_pending boundary. YuugenMagan progression
test now verifies live Elis arrival instead of that former boundary.

Cross-platform SDL comparison exposed unspecified argument evaluation in original
`wave_teleport(random_left(), random_top())`: ARM consumed coordinate RNG in the
opposite order from Windows/Linux. Native main explicitly evaluates Y then X;
DOS branch remains byte-equivalent after shared-body/type normalization. Added
full-core regression checks the snapped background at the expected random target.
This establishes native platform consistency, not original-DOS frame parity.

Linux full build and 109/109 CTest pass after this fix. ARM64 core and live
session tests pass under host-side QEMU. Windows preview rebuilt. All three SDL
previews ran 600 ticks / 1766 presentations / playing and matched BMP SHA256
`512c0ce67a78b1bab6d9202387e383412318177fcebb66fff2e0fd454c6e2787`.
Inspected Windows preview and Linux live-session image. No MyMini deployment.
Windows launcher: `build/windows/TH01 Native Elis.cmd`.

Remaining boss is Sariel; native audio playback, title/settings, ending and
hardware/accuracy validation remain unfinished after boss coverage.
### Sariel original body setup and wand background (2026-10-30)

Added SarielBody using shared original setup, shield/dress scheduling, wand raise,
and wand lowered capture/restore functions. Original DOS source includes the same
bodies; check_sariel_source.py confirms expanded sources still match HEAD.
Setup retains 18 HP, HUD color thresholds 8/2, shield hitbox, original coordinates,
and distinct animation instance/bank indices. Shield cycles every 10 frames;
dress every 15. Wand raise draws at calls 2 and 16. Restart resets timing only:
it leaves the last cel visible until call 2 of the next raise.
Native background storage captures the original 128x96 area from page 1 as twelve
32x32 tiles, restoring it to both pages when lowering. It deliberately retains
original WAND_W=128 despite the wand BOS being 96 pixels wide. Capture is explicit,
so the eventual main controller can call it at the original scene transitions.
Tests cover cadence, repeat raises, restart, hitbox, both-page restoration,
adjacent pixels, release/reload and original asset bank mapping.
Linux Sariel tests 3/3 pass; native ARM64 under host QEMU passes body test with
identical trace f728ceae2c1cedbf and matching rendered BMP. This is architecture
validation, not MyMini hardware validation. Last full suite remains 110/110 before
the entrance/body additions; no new full-suite claim or preview deployment.
Sariel attack patterns, phase controller, damage/defeat and StageSession remain
unfinished. Real audio, title/settings, endings and device validation also remain.

### Sariel random purple laser pattern (2026-10-30)

Shared original pattern_random_purple_lasers now runs through SarielRandomLasers.
No attack geometry or schedule was replaced: frame 50 chooses ten origins, then
frames 59..149 launch one laser every ten ticks, with a random floor target.
Pattern completes at frame 201, leaving live lasers for the common updater.
Original circle summon/unput, SE6 commands, purple color 4, width 5 argument,
25-age moveout and rank speeds (8/9/9/10 after original integer truncation) remain.
The adapter rejects simultaneous instances because original pattern state is
shared; completed calls are inert. This is a standalone pattern adapter, not
Sariel main/StageSession integration.
Linux Sariel tests 4/4 pass. New test exercises all four ranks and three seeds,
origin/target RNG order, exact launch slots/ticks/directions/speeds, first-circle
background restoration, sound commands, completion and sustained laser drain.
Linux and native ARM64 under host QEMU produce identical trace
c63bd04f425f8143. Source expansion audit still matches original DOS HEAD.
No MyMini deployment or full boss/visual parity claim. Remaining Sariel work:
other patterns, phase controller, damage/defeat and live StageSession integration.

### Sariel orb rings, aimed sling clusters and semicircle volleys (2026-10-30)

Added three shared original attack bodies: pattern_2_rings_from_a2_orbs,
pattern_aimed_sling_clusters and pattern_four_semicircle_spreads. DOS b20m includes
the same files; expansion audit matches HEAD. Generalized standalone pattern
adapter to SarielPattern with an explicit kind; existing random laser API remains
an alias with its original default. Shared-state one-instance ownership retained.
Orb rings use original x=568/64,y=160, four mirrored cloud pellets per volley,
intervals 18/12/10/8, wrapping byte-angle, 3.5px base speed, and frame-480 end.
Sling clusters sample one random angle at each rank-dependent interval in frames
150..199, retain swapped polar X/Y calls, PM_SLING_AIMED fixed 4.5px motion speed,
and finish at frame 299. Semicircles fire 20/10/8/10 pellets at frames 50/60/70/80,
with original angle steps/base speeds, finishing at 151.
New per-frame tests cover four ranks and two seeds for each pattern, exact volley
counts/positions/velocities, cloud delay vs immediate motion, fixed sling motion,
RNG consumption, totals and completion. Linux Sariel tests 5/5 pass. Linux/native
ARM64 pellet traces match ffb3a06a700bf5c3; ARM64 random laser regression retains
c63bd04f425f8143. These tests isolate emission frames; they do not yet validate a
full Sariel encounter or combined phase ordering. No MyMini deployment.
Remaining: other patterns (vortices/birds/snowflake/later attacks), phase control,
damage/defeat, StageSession and remaining full-port audio/menu/ending/device work.

### Sariel original vortex attack (2026-10-30)

Added SarielVortices with shared original vortex_fire_3_spread/pattern_vortices,
real BOSS6GR1.GRC cels, existing pellet backend and SarielBody wand methods.
Preserves static/diagonal/horizontal movement intervals, rank split frames
140/145/150/155, direction RNG at 200/240, sequential per-entity spread geometry,
original 7px rank-tuned speed, 48x32 sloppy unput and frame-320 completion.
The right-edge 48px restore wraps into the next scanline and can erase part of
the left vortex; the raster test accounts for that existing original behavior.
Two native compatibility branches replace Borland cast-to-lvalue assignments
with direct int16 assignments and initialize the discarded input angle to avoid
reading an indeterminate value. DOS branches remain unchanged; source audit
now selects the DOS preprocessor branches before comparing expanded source HEAD.
Adapter receives an existing SarielBody and requires its original background
capture before attack execution. It does not substitute a complete boss controller.
Tests run four ranks and sixteen seeds, cover all four direction combinations,
exact per-tick positions/angles/velocities and group types, sprite transparency,
RNG order, completion and inert subsequent calls. Emission test clears pellets
between ticks; it does not claim full encounter collision or visual parity.
Linux Sariel group 6/6 passes; native ARM64 under host QEMU matches trace
12c6bf32796d6773 and inspected Linux snapshot. Linux/ARM64 BMP SHA256:
4d24b0d36076f3fb22788702db61192794bafab94df1435716a30024547778af.
No Windows preview rebuild or MyMini deployment. Birds, snowflake, later attacks,
phase control, damage/defeat and StageSession still remain for Sariel.

### Sariel original bird flock lifecycle (2026-10-30)

Shared original CBirds struct/reset/spawn/fire/unput_update_render now compiles
through SarielBirds, using real BOSS6GR2.GRC sprites and existing pellet/graphics
backends. DOS b20m includes the same source body; expansion matches HEAD.
Retains thirty slots, first-free allocation, 25-call hatch (movement on final
hatch tick), accumulated hatch cels without erasure, double-coordinate movement,
48x32 sloppy restore, inclusive top-left screen bounds and original contact box.
Contact retains original 100ms delay through a host callback (real sleep when no
callback is supplied). Aimed, six-ring and random gravity rain firing preserve
original muzzle offsets, rank tuning and RNG consumption. Native adapter owns
one flock; constructor clears full state for deterministic reload.
Tests compare full raster throughout hatch, check fractional movement, capacity,
reset/reload, all firing groups/four ranks, rain RNG and acceleration, clipping,
invincibility and contact delay. Linux Sariel tests 7/7 pass; native ARM64 under
host QEMU matches Linux trace 7b4d05bb344688b. This verifies the flock primitive,
not ellipse-arc or symmetric-bottom attack controllers, which still remain.
No Windows preview rebuild or MyMini deployment. Remaining Sariel: controllers
for both bird patterns, snowflake, later attacks, phase control, damage/defeat and
StageSession; other full-port audio/menu/ending/device work is still outstanding.

### Sariel ellipse-arc bird attack controller (2026-10-30)

Shared pattern_birds_on_ellipse_arc now runs through SarielBirds begin_arc /
advance_arc, with an existing SarielBody and real BOSS6GR3.GRC spawn crosses.
Each advance updates the flock before the attack, matching original phase 1.
Preserves spawner fixed-point acceleration, rank egg intervals 20/15/10/8,
reverse egg spawn order at frame 200, random integer flight vectors, minimum
spawner drift, wand scheduling, aimed/ring group choice, paired firing ticks
(mod16=0/2) and completion at 400. Original late-cross subpixel/pixel clipping
bug is retained. Body background capture remains an explicit caller prerequisite.
New test independently simulates trajectories and all bird slots every frame,
with four ranks/eight seeds. Checks RNG order, reverse eggs/hatch/movement,
mature firing and muzzle coordinates, aimed-group bounds vs unrestricted single
ring pellets, 100-slot pellet capacity, SE8 command and completion. Both firing
groups are covered. Emission test clears pellets per tick; not full encounter
projectile integration. Linux Sariel tests 8/8 pass. Linux/native ARM64 under host
QEMU match trace 2e8350e6b56687ae and snapshot (Linux image inspected).
Expanded DOS source audit passes. No Windows preview rebuild or MyMini deployment.
Symmetric-bottom bird attack still needs debris/spawnray controller; snowflake,
later attacks, full phase control, damage/defeat and StageSession also remain.

### Sariel symmetric bottom birds and shared spawnray/debris helpers (2026-10-30)

Shared original vortex/debris sprite declarations, Sariel spawnray wrapper and
SymmetricSpawnraysWithDebris template now compile natively with the original
pattern_symmetric_birds_from_bottom. SarielBirds begin_bottom/advance_bottom
updates the flock before the pattern. Real BOSS6GR1 debris and common line backend
render original alternating rays and sprites; original sloppy restore/wrap and
right-ray erase calls remain. Pattern spawns four mirrored pairs at 60/70/80/90,
with integer vector2_between flight and original 25-call hatch. Mature birds fire
gravity rain on mod6 ticks below 100, then mod10 after 100; frame 100 only erases
the ray. Frame 300 fires before completion. Rank-selected unknown value retained.
New test independently models eight bird states at every frame over four ranks
and three seeds, verifies mirrored spawns/velocities, hatch/update ordering, RNG
per firing bird, gravity acceleration, rank speed, SE6 schedule and completion.
Linux Sariel tests 9/9 pass. Linux/native ARM64 under host QEMU match trace
2c68601d51345a9e and screenshot (Linux snapshot visually inspected). Expanded DOS
source audit passes. Tests isolate emitted pellets per tick, so no full encounter
collision or original-DOS frame parity claim. Arc/flock Linux regression passes.
No Windows preview rebuild or MyMini deployment. Both bird attack controllers
now exist standalone. Snowflake, later attacks, complete phase transitions,
damage/defeat and StageSession integration remain for Sariel, in addition to
full-port audio/menu/ending/device validation work.

### Sariel detonating snowflake combined with orb rings (2026-10-30)

Shared original pattern_detonating_snowflake compiles through SarielPattern kind
SnowflakeWithRings with an existing SarielBody. Each tick invokes snowflake first,
then original orb rings, matching phase-3 ordering and frame-480 completion.
Retains the 160-frame trigger, 16-call wand raise, aimed fixed-point flake flight,
bottom trigger/one-call return, crossed ellipse/star explosion, original phase /
detonation-frame union behavior, seven shrinking updates, horizontal hitbox and
invincibility. Original star_left=subpixel assignment bug remains. Native body
background must be captured by caller before the wand lowering call.
Tests independently model every tick over four ranks, three aim positions and
both invincibility states. Verify two launches/two detonations, fourteen radius
updates total, exact RNG/sound timing, contact damage window, continuing cloud
rings/velocities and completion/inert calls. Linux Sariel tests 10/10 pass;
native ARM64 under host QEMU matches trace 4df102822b614f03 and snapshot (Linux
explosion image inspected). Expanded DOS source matches HEAD by audit.
Emission tests clear pellets each tick, not full encounter projectile simulation.
No Windows preview rebuild or MyMini deployment. Later attacks, full phase
transitions, damage/defeat and StageSession integration remain for Sariel; audio,
menus/endings and hardware validation remain for the full port.

### Sariel four aimed lasers and shared ray wrapper (2026-10-30)

Shared original pattern_four_aimed_lasers now runs as SarielPattern kind
FourAimedLasers. Factored the existing original spawnray wrapper/reset out of
sariel_spawnrays.inc into nested sariel_ray.inc, used by both bird and laser
adapters. Source audit expands nested includes to convergence and verifies the
expanded DOS source still matches HEAD. Common line backend handles ray drawing.
Preserves launches at 50/100/150/200, origins (460,120)/(180,120)/(608,164)/(24,164),
first target capture, second target mirrored from the captured X, later live
player aims, moving spawn rays, rank speed truncation 7/8/8/9, width 8, moveout
age 50, SE6 commands and frame-301 completion. Live lasers remain for updater.
New test covers four ranks/three initial positions while player changes position
between launches, validates slots/origins/aim/speed/width/lifetime, no RNG, sound
schedule, completion and subsequent laser drain. Linux Sariel tests 11/11 pass;
native ARM64 under host QEMU matches trace 3b3bb3251b3edf0b and rendered snapshot
(Linux inspected). No original-DOS frame parity or hardware validation claim.
No Windows preview rebuild or MyMini deployment. Remaining Sariel: rain overlay,
radial/vertical/final attacks, background transitions/particles, full phase
controller, damage/defeat and StageSession, plus full-port audio/menu/ending work.

### Sariel top-rain overlay and radial stacks/lasers (2026-10-30)

Shared original pattern_rain_from_top and pattern_radial_stacks_and_lasers run
as FourLasersWithRain / RadialStacksAndLasers. Rain native branch selects through
an int temporary then casts to pellet_group_t, avoiding DOS enum/int aliasing;
the original DOS branch is unchanged and source audit passes.
Combined laser/rain calls laser first then rain, retaining the extra rain volley
when laser completion resets frame to zero at 301 (38 volleys total). Lunatic
rain adds its original random wide angle; other ranks rain vertically.
Radial attack retains accelerating spawnray prelude, 40 rotating volleys at
205..400, rank-dependent stack speeds/counts, cyclic laser slots and 450 finish.
It requests 13 lasers; tick 225 reuses busy slot 0, so that request is ignored,
yielding 12 spawns with continuously updated lasers. Pellet totals are
107/107/187/267 by rank. Original SE6/SE7 schedule and no radial RNG retained.
New tests cover all ranks/two seeds, final zero-frame overlay call, exact RNG,
positions/angles/velocities/types, sound sequence, busy-slot rejection, width /
lifetime/speed, completion and laser drain. Linux Sariel tests 12/12 pass;
native ARM64 under host QEMU matches trace a61e4291fb318ab7 and rendered snapshot
(Linux inspected). Emission tests reset pellets each tick, not full collision
simulation. No original-DOS frame parity or MyMini hardware validation claim.
No Windows preview rebuild/deployment. Remaining Sariel includes vertical stacks,
final sprays/leaves, transitions/particles and full phase/damage/defeat/StageSession;
full-port audio, menus, endings and device validation also remain.

### Sariel vertical stacks followed by random rain (2026-10-30)

Shared original pattern_vertical_stacks_from_bottom_then_random_rain_from_top
runs through SarielColumns with real debris sprites and shared spawnray template.
Preserves alternating scan rays, original initially-zero debris cel, rank rain
intervals 16/13/10/8, nineteen bottom-stack opportunities at 160..340, four speeds
per side and strict player-center distance >32 safety gap. Finishes at frame 601.
Original branch fallthrough also fires random rain at eligible ticks 131..150,
from y=399, moving the bottom spawners before columns begin. At 350 spawners reset
to the top; late rain advances origins outside the viewport on Lunatic. Right
rain aims still subtract the left spawner coordinate, as in the original source.
No bounds sanitization or simplified phase schedule was substituted.
Tests independently model every emission frame over four ranks, three player
positions and two seeds, checking safety suppression, early/late rain, exact
origins/angles/velocities, rank speed, RNG order, sound sequence and completion.
Linux Sariel tests 13/13 pass. Native ARM64 under host QEMU matches Linux trace
5995221b12ff556a and screenshot (Linux inspected). DOS source expansion audit
passes. Emission tests clear pellets per tick, not full encounter collision
simulation. No Windows preview rebuild or MyMini deployment.
Remaining Sariel: final curved sprays/seal rain/leaves, transitions/particles,
full phase/damage/defeat and StageSession. Audio/menu/endings/hardware validation
still remain for the complete native port.

### Sariel final curved sprays and seal-center rain (2026-10-30)

Shared original dottedcircle helper, CurvedSpray class and once/twice sweep
controllers now run as SarielPattern CurvedSprayOnce / CurvedSprayTwice.
Preserves original integer target steps 32/24/18/16 by rank, one pellet per tick,
7px initial speed, 0.25px decrease and speed reset at each reversal. Existing
pellet minimum-speed clamp retained. Single sweeps emit 41/54/72/81 pellets;
double sweeps emit twice that, starting at 100/80 respectively. Original expanding
dotted-circle prelude and update/unput timing compile shared source.
Shared pattern_rain_from_seal_center runs as SarielColumns SealRain, retaining
alternating rising rays/debris, broken ray unput calls, 30/35/40/45 gravity pellets
with integer spacing, full random angle, SE6/SE3 sequence and frame-104 completion.
Tests cover all ranks/two seeds, exact per-tick vectors/counts/speed clamp and
completion for both sprays; seal burst positions/types/RNG/sounds are verified.
Linux Sariel tests 14/14 pass. Native ARM64 under host QEMU matches trace
21f5bee327640f05 and screenshot (Linux dotted-circle image inspected). Expanded
DOS source audit passes. Emission tests clear pellets per tick; full encounter
projectile integration and original-DOS parity remain unverified.
No Windows preview rebuild or MyMini deployment. Remaining Sariel includes
swaying leaves, transitions/particles, full phase/damage/defeat/StageSession.
Full-port audio, menus, endings and hardware validation remain outstanding.

### Sariel swaying leaves and shared 8x8 monochrome renderer (2026-10-30)

Shared original pattern_swaying_leaves now runs through SarielLeaves, preserving
30 slots, first-free spawning, decimal-subpixel coordinates (1/10 pixel), spark /
splash / leaf states, even-frame updates, random sway reversals, retirement and
strict player contact. Reset command 999 clears flags without rendering/RNG.
Original offscreen splash-render bug is retained. Native branches bind read-only
state inspection, include checked-in bitmap-derived leaf rows instead of DOS-
generated CSP files, and replace Borland enum cast-to-lvalue increment. DOS branch
still matches HEAD after expansion. Leaf_s/l/r rows are checked against BMP files.
Original grcg_put_8x8_mono body is shared with DOS via hardware include; native
GRCG/VRAM adapters render its unaligned 16-bit masks without CPU emulation.
Tests simulate all thirty slots over 1800 frames for two seeds, four intervals
(1/20/32/500), and both invincibility states. Verify exact flag/position/velocity,
ordered RNG, capacity saturation, sway reversal, retirement, collision, reset
side effects and first-spark raster. Linux Sariel tests 15/15 pass. Native ARM64
under host QEMU matches trace 1e23334775c23cb9 and screenshot (Linux inspected).
Source audit covers original Sariel, transition, mono renderer and leaf bitmaps.
This is a standalone leaf subsystem; final phase combinations are not integrated
or verified yet. No Windows preview rebuild or MyMini deployment. Sariel still
needs background transitions/particles, combined phase control, damage/defeat
and StageSession; full-port audio/menu/ending/device work also remains.

### Sariel three-gust background transitions (2026-10-30)

Shared original bg_transition runs through SarielBackground with explicit VSync,
stage-palette and HUD/HP host callbacks. Native GRCG compare reads set mask bits
for pixels equal to the selected tile color (as documented in master.lib
pc98_gfx.hpp GC_TCR); RMW writes apply stripe colors only through those masks.
Preserves three 20-step gusts, frame rewinds 69->49, 109 total ticks, 100 RNG row
offsets per active step (6000 total), same-row clipping and SE10 at gust starts.
Final operation resets scroll, loads/copies the selected real GRP to both pages,
saves stage palette, rerenders HUD/HP and completes on page zero. Targets 1..3
exercise original A2/A3/A4 backgrounds. Callbacks are required, not silent stubs.
Native-only cell-mask accessor maps indices 5..9 to the existing unput array,
avoiding original out-of-bounds indexing through put[5] while preserving intended
adjacent-array data. DOS branch remains byte-equivalent after source expansion.
Test independently models all stripe mask/read/recolor/erase operations over a
mixed synthetic background, comparing both pages every tick for three targets /
two seeds, then exact real GRP pixels/palette and callback order/HP/scroll/page.
Linux Sariel tests 16/16 pass; native ARM64 under host QEMU matches trace
dbacee9b34fedc20 and final screenshot (Linux inspected). Source audit passes.
No Windows preview rebuild or MyMini deployment. Particle effects, final combined
phases, full damage/defeat and StageSession integration remain for Sariel; audio,
menus/endings and hardware validation remain for the full native port.

### Sariel vertical, wavy and horizontal 2x2 particles (2026-10-30)

Shared original particle macros and all three unput/update/render functions now
run through SarielParticles. Native TCR reads compare page pixels against COL_AIR;
RMW masks restore air or draw particle color 4. Keeps vertical byte-boundary shift,
wavy truncation at byte edge, and horizontal two-byte split. Invalid native VRAM
addresses read zero / discard writes, consistent with other native backends.
Retains first-free 30-slot banks, frame%7 spawning, even-tick updates, both vertical
directions, 100-age wavy lifetime and horizontal double-coordinate drag by 0.1.
Native-only hooks expose state and reset colors on host reload without rendering
or RNG; original DOS expanded bodies remain unchanged by audit.
Tests independently simulate all slots over 700 frames for three modes, both
vertical-direction settings and two seeds. Compare state/RNG plus both complete
framebuffer pages every tick against a pixel model on mixed air/non-air patterns.
Covers retirement, byte-edge split and reset side effects. Linux Sariel tests
17/17 pass; native ARM64 under host QEMU matches trace 50b1e584a7896ae3 and BMP
SHA256 57ff86de427fd7fd1ab809ba79eba1bc82ad42a952a00887d130c3b59738ce90.
No Windows preview rebuild or MyMini deployment. Effects remain standalone:
combined phases/main controller, damage/defeat and StageSession integration are
still pending. Audio/menu/endings and hardware validation remain for full port.

### Complete original Sariel controller compiles as native object (2026-10-30)

Added native compile branch to original b20m.cpp, using sariel_backend.hpp and
sariel_original namespace. All original entrance, patterns, combined phase flow,
two forms, hit calls, transitions and defeat code now compile together on Linux
and ARM64 as sariel_controller_object. This is OBJECT compilation only: host API
implementations and linkage/runtime tests are not yet complete. Preview remains
on the existing playable bosses; StageSession still marks Sariel pending.
Introduced sariel_frame_t (DOS int / native int16) for boss frame, hit invincibility,
pattern-state union and by-reference pattern frames. Standalone isolated adapters
keep their existing host int frame through local alias; short patterns remain
within range. Expanded DOS source normalizes this alias and matches HEAD.
Native header declares explicit graphics/assets/timing/music/HUD/palette APIs,
plus read-only leaf/particle observation hooks. No fake host function bodies or
silently successful transition stubs were added. Existing entity/laser/shot/hit /
math APIs remain common. Object undefined-symbol inventory was inspected with nm:
Sariel-specific APIs still need definitions in the host backend before a linked
runtime test can run. Global existing library APIs are also naturally undefined
in the object until final linkage. ARM64 object build completes successfully.
Rebuilt affected standalone adapters after signature changes; Linux Sariel group
17/17 passes. This does NOT prove combined controller runtime, full encounter,
original-DOS parity or MyMini hardware operation. Next: implement backend from
verified helper adapters, link complete controller, exercise phase/damage/defeat
and integrate StageSession. No Windows preview rebuild or MyMini deployment.

### Linked full Sariel controller/backend runtime verified (2026-10-30)

sariel_core now links the complete original b20m controller with real native host
APIs for four GRC banks, wand background tiles, GRP/palette load, TCR/RMW masks,
8x8 mono rendering, scrolling/copy, shared diagonal transition, HUD HP, timing and
audio command callbacks. Required callbacks fail explicitly when absent. Native
stage palette uses existing bomb palette storage. Native black-out adapter retains
the original erroneous unconditional component decrement and 4-bit display wrap.
Missing BOSS6.GRP is handled exactly like original grp_put file-open failure:
page and loaded palette remain unchanged; missing name is recorded. No replacement
asset was invented. All present image filenames are normalized for Linux case.
Original source now observes its actual local phase state at function return and
clears native actor/particle/leaf banks on host reload, including interrupted wand
flags. A cleared guard prevents replaying defeat/free/bonus on subsequent calls.
DOS branches still expand to HEAD; native frame/hit/pattern union storage is 16bit.

New linked test runs full original entrance (801 waits / 5200ms diagonal callbacks),
18HP first form, real orb hit +10000 and invincibility, all first-form attack /
background phase states 1..8 and loop, first-form defeat and six-HP second form.
It runs final patterns 0..8 INCLUDING completion of leaf/double-spray combo 8 and
return to 4, then verifies real second-form hit/invincibility, original final
animation/audio commands, +50000 clear bonus, resource cleanup and inert clear.
A second full load/entrance/defeat run verifies reset/reload. Common live pellet
and laser updaters run throughout; debug damage is used to drive HP to defeat.
This is controller-level runtime validation, not manual gameplay or DOS parity.

Linux full build succeeds and entire suite 127/127 passes. Native ARM64 under host
QEMU runs same complete test with matching trace d75a078ee951e49b. Linux/ARM64
final-combo snapshot SHA256:
d04a212062aa24f1d809589cee499ae1f4d2c42e4c03f3fc22d3c1ebc4662c54.
Linux snapshot visually inspected before final coverage extension; emitted-image
hashes for extended test match across architectures. Source audit passes.
StageSession still marks Sariel pending: next step is SarielGameplay adapter,
Makai stage20/Continue/ending handoff and preview integration. No Windows preview
rebuild or MyMini deployment. Actual audio synthesis, title/options/endings and
hardware validation remain part of the full port objective.

### Live Makai stage20 Sariel, Continue and Windows preview (2026-10-30)

Added SarielGameplay owning the real full controller/backend lease and asset
lifecycle. StageSession now starts Sariel directly at Makai20, enters it from
stage19, preserves boss HP across normal misses, reloads it after Continue,
provides original HUD rerender/stage reset/audio-command callbacks, and hands
final clear to ending_pending without replaying +50000. Boss-hit palette dispatch
selects Sariel while active; destructor/GameOver cleanup release the actor lease.
Original Makai20 timer remains 6000. Ending presentation is still not ported.
New generic GameAudioCommand queue records music/SE-bank load/play/fade/stop
requests; preview drains it alongside SE commands, with playback still pending.
Updated Elis scene test to verify actual stage19->20 Sariel handoff rather than
expecting boss_pending. New live session test covers startup/normal miss, stock
exhaustion/GameOver, Continue reset (HP18/timer6000/resident), both forms and
victory bonus/ending boundary. Linux and native ARM64 host-QEMU session tests pass.
Linux full build and entire CTest suite 129/129 pass, including SDL Sariel smoke.
Windows preview rebuilt with original b20m and native backend/gameplay sources;
launcher build/windows/TH01 Native Sariel.cmd starts --stage 20 --route makai,
with no normal-launch timeout. Windows/Linux/ARM64 SDL tests each run 600 ticks /
1810 presentations / playing, matching BMP SHA256
923765b1867e0473ff0501f6b9511912c91d67e726e28803f44ae7b38dd89cfa.
Windows snapshot visually inspected (battle background, HUD, player/orb and boss).
DOS source audit passes. All seven bosses are now integrated in live StageSession.
This is still a prototype: real audio playback, title/options, endings and full
accuracy/performance/hardware validation remain. No MyMini deployment; TH08 and
original HDI remain untouched.


## PRESS KEY confirmation release (2026-10-30)

The title dismissal key previously carried into the original level-triggered
main menu and immediately selected Start. Native title now displays choices
immediately, consumes all held menu input until release, and accepts a fresh
press to navigate/confirm. Tests hold Enter, Z, controller A (Enter+Z), and
Escape for 30 frames, then select Continue with a fresh direction/confirmation.
Linux full suite 139/139 and ARM64 title regression pass. Autoplay explicitly
releases between title dismissal and Start; normal launch has no autoplay.

Recorded BGM (2026-10-30): the MyMini launcher now selects the 16-track PCM WAV
pack in `th01-native/music`, while SE still use ZIGOKU.MDE. Mono mixing remains
on for the device's single speaker. `TH01_WAV_DIR` enables this backend on other
builds. Preparation/provenance and loop limitations are documented in STATUS.md;
`native/prepare_recorded_bgm.py` prepares the host files. Recordings are local
assets, not source-code dependencies and are not committed to Git.

Current music default after user revert (2026-10-30): OPNA/MDT, not recorded WAV.
The launcher explicitly unsets TH01_WAV_DIR; original SE and mono remain enabled.
The historical recorded-BGM notes above describe the now-reverted trial.

2026-10-30: deployed audio correction packs MDT streams into physical channels
by type/order (SSG A/B for the title), and restores MDRV98's default FM/rhythm
master attenuation. See AUDIO_DIAGNOSTICS.md for reference evidence and limits.

Physical Start (2026-10-30): confirms title/options/mode menus and pauses during
gameplay. PortMaster maps it to a dedicated F1 input; SDL assigns its meaning by
context. Reopen through Ports after updating the mapping.

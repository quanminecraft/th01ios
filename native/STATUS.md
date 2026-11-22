# Checkpoint: 2026-10-30

Goal remains a complete native TH01 port for Xifan MyMini / ArkOS / ARM64.
Current unified preview supports original title/options/Music Test, both
routes/all seven bosses, ending/slideshow/verdict, real score registration
and return to title. Native MDT BGM and original MDE sound effects play through SDL;
full original-driver audio parity and on-device validation remain
unfinished. TH01 is installed on MyMini; latest gameplay patch awaits user feedback.

Latest verification: Linux full suite 142/142 passes; latest integrated regressions also pass on host-run ARM64. Original title SDL BMPs
match Windows/Linux/ARM64. Native 12-second REIMU PCM matches Linux/ARM64;
Windows SDL normal-device audio test reaches gameplay with 222208 rendered
audio frames. ARM64 uses host-side QEMU for validation only; no CPU emulator
is shipped with the port. Windows launcher: `build/windows/TH01 Native.cmd`.
See latest dated entries at the bottom for detailed evidence and remaining work.

## Historical foundation checkpoint (2026-10-28)


Goal remains a playable native TH01 port for MyMini/ArkOS, using fresh ReC98
source and the user's original assets, with no DOS/x86 emulator in the product.
This checkpoint is progress toward that goal, not completion.

## Foundation evidence (historical)

- Upstream base: debloated `28d59d31331e27fcbe61d1b65bd8c0cd618757d8`.
- Original th01.hdi SHA256 before extraction and after current work matches:
  `cd2f9cb42318d2b7a280071c92781a8412b2b2f15f8663ba2cbb2d9d50c9f2fb`.
- Windows native build: `build/windows`, including local runtime DLLs and
  `TH01 Native Asset Preview.cmd`. No automatic timeout in a normal launch.
- Linux native build: `build/linux`; CTest passes all nineteen tests, including
  validation against the real extracted data.
- ARM64 build: `build/arm64`; CTest passes all nineteen tests under the
  development ARM64 runner. Direct glibc symbol requirement of sprite_preview:
  GLIBC_2.17. CMAKE_CROSSCOMPILING_EMULATOR is configured for tests only.
- All 41 decoded archive payloads and 227 sprite/47 GRP raster checksums match
  between Linux x86-64 and ARM64. Reports: `build/validation-linux.txt` and
  `build/validation-arm64.txt` (identical).
- Stage 1 static rendering, including MASK.GRF HUD, is byte-identical as BMP
  between Linux x86-64 and ARM64. SHA256:
  `fe5dca6dc8d2d14fc03ecd612031625e34740a4ec8e6ebe116c50eeb8c503220`.
- Original orb source compiles on Windows/Linux x86-64 and ARM64. Isolated
  10,000-frame trajectory hash: `c1b2b0da96abb803` on all three builds.
- Visual inspection: Reimu sheet, Mima BOS sheet and stage 1 scene/HUD.
  BOS transparency uses zero bits for opaque pixels; PTN uses color 15.
- Original CShots now builds natively and calls an actual framebuffer backend:
  quarter-sprite draw, masked restoration and word-aligned EGC restoration,
  including horizontal wrapping within valid page memory.
- Shot tests cover eight-slot capacity, motion/removal, orb/pellet/boss edge
  rules, impact extra orb movement, decay and sound-command dispatch.
- Original player clamp, orb collision, invincibility and repel routines build
  natively; their boundary/quirk tests pass on Windows, Linux and ARM64.
- Real-data shot integration uses STG.PTN and ST0.GRP; collision at frame 12,
  final framebuffer digest `0d65c5fdd2da6afa`, matching across architectures.
- The original player action state machine is now compiled, using native
  PTN/BOS draw and restore adapters. Native temporary storage replaces two
  Borland-specific casts used as assignable lvalues; action logic is shared.
- Action tests cover standing/moving shot timing, swing deflection, 13-step
  slide, moving/stationary flipkick, slidekick, seven-shot combo origins and
  the three-special cap. Per-frame framebuffer traces match Windows/Linux
  x86-64 and ARM64: synthetic `e5fe831898794205`, real `874dd2c4e9cd1c6e`.
- Bomb effect, bomb HUD and pellet-speed callbacks fail explicitly until
  connected. The action test checks that unsupported bomb activation cannot
  silently succeed. Native audio playback remains unimplemented.
- Original input.cpp now owns shared input state and player_is_hit. A physical
  key snapshot replaces key_sense's hardware access; the original edge and
  consumption logic, direction merge, pause behavior and bomb combo detection
  are compiled unchanged. Debug heap screen callbacks explicitly reject use.
- New input tests cover the frame-20 combo-window expiry, consumed held input,
  release/re-press, opposing directions, cutscene numpad exclusion and the
  original paused-shot/Enter hit flag quirk. Input -> player integration checks
  actual motion, casts, single swings and the slide-to-seven-shot combo.
- Original cards_hittest now runs with native card storage and ordered score,
  combo, HUD/item/SE requests. Tests cover geometry, repeat-hit exclusion,
  Lunatic/scene scoring, cap, 16-bit combo-square wrap, item cycle and bomb
  selection. Requests have no item/HUD consumer yet.
- Original cards_score_render and cards_update_and_render now run directly
  with native both-page sprite/background and score-popup rendering. Tests
  verify frame-30 removal, HP decrement/re-hit, popup cleanup and the original
  stage_cleared/player_is_hit completion signal. Lunatic revenge requests occur
  at frame 25 with the original group/speed; bullet actors remain unported.
- Both-page animation traces match Windows/Linux x86-64 and ARM64:
  synthetic `faa870a69cb00795`, real `364319637b537f55`. A controlled mass-hit
  test clears the real first stage's 26 cards and reaches score 113100; this
  verifies the card lifecycle, not an actual player-driven clearance.
- FONT.ROM bitmap data was read from the user's existing bundle at
  `C:/Games/The Touhou98 Experience v3.0.0/pc98lib/emulator/FONT.ROM` and copied
  into ignored build/assets/th01-ja. SHA256:
  `c261bba53f2d7dab36c7eeb4be21c8c2b56c71d8d9c3c4609b19b7d230905153`.
  Native rendering uses the JIS row 0x29 halfwidth glyphs and original bold
  transform. General Shift-JIS and additional text effects are still missing.
- Popup BMP is byte-identical on Windows and ARM64; SHA256
  `5d8545b0242853cc592ce33aad91c3ad582c72bb676b4e770e6fa7d8314af6fd`.
  Visual inspection confirmed the real background and the native-rendered 100
  popup in build/card-popup.png. No BIOS, CPU or video device emulation is used.
- Card animation IDs and pellet group enums are shared with upstream source
  through card_anim.inc and pellet_group.hpp. Subpixel storage is explicitly
  16-bit; SPPoint's packed expression retains negative-X sign-extension behavior
  without host aliasing. Positive and negative packing/layout checks pass.
- Original obstacle collision, turret fire/reset and portal enter/reset
  routines now compile directly from stageobj.cpp. Native state preserves
  16-bit obstacle/frame storage; turret enum increment uses a standard cast
  instead of aliasing its storage through an int reference.
- Shared active render page now lives in ShotBackend and is used by PTN,
  card and text helpers. Explicit front-page restore/player-animation APIs
  return to page 0. Regression traces for previous components remain unchanged.
- Obstacle tests cover bumper response/cooldown/fast-X behavior, top/bottom
  boundaries and adjacent vertical bars without double reflection. Portal
  tests verify RNG call order, destination selection at frame 20, exit at 40,
  unlock/re-entry at 60 and 17 RNG rolls for a single-portal fallback.
- All twelve turret groups pass slow/quick spawn timing and seven-frame
  cooldown tests; both pages reflect warmup/reset. Easy rank leaves the fire
  timer at zero. Pellet requests still do not create active bullets.
- Real first-stage portal animation trace: `bbf1ac499dc3129e` on Windows,
  Linux x86-64 and ARM64. Frame-20 BMP SHA256 matches Windows/ARM64:
  `049b4e674a63a664b5c9236ff576556fefa120bad590b436acdbd29bcc18c473`.
  Visual inspection confirmed real stage/card/portal assets in
  build/portal-frame20.bmp. Source Unicode comments were preserved as UTF-8.

No on-device testing or installation has been done for TH01. TH08 is untouched.
QEMU runs native ARM64 tests on the PC only; it is not included in the port.

## Foundation implementation boundary (historical)

The SDL executable is an asset/layout viewer. It has no active game loop,
player action state machine, shots, collisions, damage, scoring, stage
progression, boss AI, menu or sound. Input, shot, player-action, card and obstacle
libraries are now compiled and tested separately, not connected to the SDL executable. The SE
adapter captures original sound commands in a queue, without playing audio.
The orb library is not yet linked into an interactive game. Its shared original source
retains the upstream velocity-clamp/return-value quirk and wall overshoot.

Next: connect the native CPellets implementation and polar/aim math with input, motion, actions, shots, orb and
stage collision through native rendering/audio/input interfaces, retaining
original update order and fixed-size arithmetic. Actor/item/HUD dispatch must
occur synchronously at the original calls, rather than draining requests later
in the frame, to preserve RNG order. Then implement the rest of gameplay,
text/audio and the MyMini/PortMaster integration. Continue from native/README.md
and the current source; do not mistake this milestone for a finished game.

## Pellet component checkpoint (2026-10-28)

Original CPellets now compiles as a native library with integer aim lookup,
seven motion modes, delay clouds, player/shot/orb collision and decay.
Original BMP masks replace DOS-generated CSP data at the rendering boundary.
The table/mask generator supports --check against upstream ASM/BMP inputs.
Native builds explicitly retain signed char and 16-bit game parameters.
Card/turret request capture now offers synchronous dispatch; full actor
integration and the interactive SDL game loop remain unfinished.
Linux x86-64: 22/22 CTests passed, including real-asset checks.
Windows pellet component test passed.
ARM64: 22/22 CTests passed under the PC-only test runner.
Windows: updated full build script and component/real-asset checks passed.

## Synchronous stage projectile integration

StagePelletBinding now connects original card/turret calls to live CPellets.
The integration test verifies two random turrets create bullets before return,
consume RNG in obstacle order, and preserve slot/origin order. Card revenge
creates a real random aimed bullet on frame 25 with its RNG consumed inline.
Linux: 23/23 CTests pass. The shared runtime binding integration also passes
on Windows and ARM64.
The SDL viewer still has no interactive game loop.

## Shared original stage tick

The original orb/pellet/stage update body is now shared verbatim in
th01/main/player/orb_stage.inc between DOS and native stage_tick.cpp.
StageTimingBinding supplies the original mid-frame wait boundary.
Integration checks cover orb-before-wait, wait-before-pellet-update,
turret spawn after pellet update, and first bullet motion on the next frame.
Linux and ARM64 full suites: 23/23 pass; Windows integration also passes.
This is the stage subroutine, not the complete main loop: input/player/item
updates, counters, miss/respawn, progression and SDL host integration remain.

## Original item spawn integration

Original item state and both spawn functions are shared with the DOS source
in item_state.hpp / item_spawn.inc. Native storage preserves the 10-byte
item structure, ten point slots, four bomb slots and first-free-slot reuse.
StageItemBinding connects card drops immediately to these actors.
Tests cover original initial splash state, capacity/reuse, real point drops
from collision and the bomb drop cycle. Linux/ARM64: 24/24 CTests pass;
Windows item integration passes. Original spawn bodies were checked verbatim
against upstream. Item motion, splash rendering, collection/popup and point
value progression are not yet ported; full gameplay remains incomplete.

## Shared original item movement

The verbatim item_unput_update_render body is shared via item_motion.inc.
Native rendering draws/restores splash circles using original polar tables,
including the original omitted final cache flush. The original uninitialized
previous coordinates in circle restore are replaced with an explicit first
point restore; this is a deliberate deterministic rendering repair.
Tests cover ten splash ticks, transition on tick 11, two-pixel fall, same-tick
floor bounce, eventual slot release, callback order and visible splash pixels
without modifying the background page. Linux full suite: 24/24 passed; updated
item integration passes Windows and ARM64.
The collection callback route is tested, but actual player/item hittest,
score/bomb changes, point value, collection popup and full main loop are next.

## Full original item lifecycle

The original item.cpp now compiles for native, including hittest, bomb/point
collection, progression, drop/reset, render loops and sixteen-tick popups.
The resident point field maps explicitly to native uint16 storage; the DOS
path retains its resident pointer. Original number formatting is reused.
FontRom now decodes standard two-byte Shift-JIS for plain wide text, while
ASCII retains existing effects. The original bomb-cap bytes 8250e4dd map to
JIS 2331/685f, independently checked with Python cp932/iso2022_jp conversion.
Wide text effects and halfwidth kana remain unsupported and fail explicitly.
Tests cover point increments through 65530, missed-item reset, strict hitbox
boundaries, splash immunity, no double collect, bomb cap conversion to 10000,
HUD/SE commands, popup lifetime and bottom clamp. Linux/ARM64 full suites:
24/24 pass. Windows item and font tests pass.
Remaining: integrate complete input/player/item/counter/stage main loop and
SDL host, then miss/respawn, progression, bosses, menus/audio and device port.
HUD and sound are still command/callback boundaries, not final presentation.

## Composed regular-stage frame

regular_frame now composes sampled input, original player/item updates,
frame counters, stage tick, pause/quit and pellet destroy point settlement.
The gameplay prefix is shared verbatim with main_01.cpp via gameplay_prefix.inc.
State preserves uint32 frame counters and int16 lives/cycle fields.
Tests verify player movement before pickup, same-frame popup, invincibility
expiry, timer cadence, hook order, score-extend before pellet point award,
pause quit skipping remaining settlement and life-loop exit on hit.
Integration passes Linux, Windows and ARM64 (additional end-frame branches
validated Linux/ARM64). No interactive SDL game loop yet.
Timer, invincibility visuals, shootout laser, score-extend and pause-menu
implementations are required host hooks; missing hooks throw before frame
updates. Boss stages are explicitly rejected by this regular-stage API.
Next: implement these remaining regular-stage systems and bind the SDL host,
then startup/miss/respawn/progression and the rest of the complete game.

## Original pellet speed and score extend

Original pellet_s.cpp now compiles with explicit native int16 speed storage.
PelletSpeedBinding routes player/main-loop float requests through the original
1/40-unit conversion to integer speed changes and original clamps.
Original extend.cpp shares live/resident life state through ExtendBackend,
updates HUD/SE, and raises pellet speed by one integer unit per awarded life.
Tests cover 400000 threshold, six-life cap, one threshold per frame, consumed
capped thresholds, speed clamp/lower quirks and actual spawned bullet speed.
The composed frame now tests original score extend and periodic speed raising.
Linux/ARM64 full suites: 25/25 pass. Windows full build script and all
component/real-asset checks pass, including composed frame and extend tests.
Timer, invincibility graphics, laser, pause and HUD presentation are still
unimplemented runtime integrations; full playable/native device port unfinished.

## Original stage timer and expiry pattern

Original timer.cpp compiles natively with uint16 counters and the shared
route enum. The native snapshot renderer restores timer backgrounds and
uses original number formatting/font effects on both pages, including the
original same-source snapshot for the two halves of the timer background.
Tests verify route-dependent values, two-unit decrement, red below 200,
expiry speed increase/SE, first hurry-up firing cadence, four actual Lunatic
bullets and eight interleaved RNG calls, and 1000/200 timer extensions.
The composed regular frame now invokes the real timer on its fourth frame.
Linux/ARM64: 26/26 CTests pass. Windows standalone timer integration passes.
The enlarged TRAM HARRY UP animation remains an explicit required callback;
it is not replaced with a silent no-op in the runtime. Timer logic/expiry
pattern are implemented, but that visual and remaining gameplay/SDL/device
integration are unfinished.

## Native HARRY UP text animation

FontRom now reads the V98 ANK 8x16 glyph block at offset 0x800. Native
HARRY/UP rendering follows tram_x16 geometry: five leading 16px text rows,
16x dot scaling, five letters across the first phrase, and 192px margins
around the second. Each phrase waits 27 frames through host timing.
HarryUpAnimationBinding supplies the timer's actual animation; the remaining
callback is timing/presentation, not an unimplemented visual effect.
A separate fixed-color text overlay composes over either graphics page; both
pages remain unchanged, and overlay cleanup occurs on success/interruption.
Synthetic and real FONT.ROM tests pass. Real-asset screenshots were visually
inspected. Linux/ARM64 full suites: 28/28 pass. Windows synthetic/real text
animation integration passes. Windows/ARM64 screenshots match exactly:
phase 1 SHA256 6b3bc82d9c720a0a7151d2a1487f385e7199cd566bda616f3586f4494867269a
phase 2 SHA256 76b1c31b11d37d3baeee1cb18a4d553ff042f4e1ac2872f818896680e0d458ba
Remaining: invincibility visuals, laser, pause/HUD/audio, interactive SDL
host and lifecycle/progression/bosses/device integration. No playable build
or TH01 installation on MyMini yet.

## Original invincibility sprites

Original inv_spr.cpp now compiles with int16 frame fields and generated masks
from shape8x8.bmp. The native renderer preserves unaligned placement, right
edge wrap, red color, no per-cel background restore and final page selection.
The original B-plane-masked pass is redundant before the full same-mask red
pass; native draws their equivalent final pixels without hardware emulation.
Tests cover initial cleanup/sentinel, 24 RNG rolls for eight sprites, known
first-slot position/cel, 64-frame trace, disabling/reactivation and edge wrap.
The composed regular frame invokes actual visuals and consumes expected RNG.
Linux/ARM64 full suites: 29/29 pass. Windows isolated invincibility integration
passes. Framebuffer trace 4c9c92021b8da4ff is checked across builds.
Remaining: laser, pause/HUD/audio, lifecycle/progression/bosses, interactive
SDL host and device packaging/verification. No playable deployment yet.

## Original shootout lasers (2026-10-29)

Original laser_s.cpp now compiles with fixed Q24.8 values and int16 age/state.
The original laser_s.bmp is converted to little-endian word masks, preserving
its unusual preshift rows. Native draws original cached 16-bit mask flushes
and restores moving segments through the shared background interface.
Tests cover alive-slot spawn guard, speed truncation, extension/moveout/exit,
omitted final flush and hit delay, hitbox boundary, invincible/non-damaging
flags, diagonal aim and a 70-frame framebuffer trace 3754135f4e60b242.
Native intentionally repairs unput_and_reset's broken Q24.8 endpoint casts:
it restores actual current ray coordinates rather than the crashing garbage
line. This reset behavior differs from DOS deliberately; regular movement
and cache/hit ordering retain shared original source.
The composed frame now updates all ten laser slots in original array order.
Linux/ARM64 full suites: 30/30 pass; Windows isolated laser test/trace passes.
Remaining: pause/HUD/audio, lifecycle/progression/bosses, interactive SDL host
and device packaging/verification. No complete playable deployment yet.

## Original life/bomb HUD icons

Life/bomb snapshot/change logic is shared verbatim with original hud.cpp in
hud_icons.inc. Native uses captured 16x16 color backgrounds, original PTN
quarter sprites and the original one-row page-0-to-page-1 copy behavior.
HudIconsBinding supplies live life state; native_hud_lives_render and
native_hud_bombs_render connect to ExtendBackend/PlayerBackend HUD callbacks.
Tests cover startup, decrement restoration, increase snapshots, copy width,
unchanged-count page selection, actual bomb pickup and actual life extend.
Linux/ARM64 full suites: 31/31 pass. Windows isolated HUD integration passes.
The original icon block was checked verbatim against upstream.
Score/hi-score/combo digits, HUD background/stage/rank and full initialization
remain next. Complete HUD, pause/audio, lifecycle/progression/bosses, SDL host
and device integration are not finished. No playable deployment yet.

## Original fullwidth HUD numeral renderer

Original graph_putfwnum_fx now compiles with explicit int32/uint32 arithmetic
for host-independent DOS long behavior. Shift-JIS numeral bytes are explicit
encoding-independent constants. FontRom wide glyphs now support original
heavy/bold/black row transformations, retaining 16-bit left-edge truncation.
Tests cover seven-digit positions, changed-digit updates, leading-zero
suppression, rightmost-digit selection, right-edge clipping and bold/black
wide glyph pixels. Linux/ARM64 suites: 32/32 pass; Windows standalone numeral
integration passes. Existing real-asset traces remain unchanged.
This implements numeral drawing, not the complete score HUD state machine.
Next: original score/high-score/combo backgrounds and update logic, then HUD
initialization, pause/audio, lifecycle/progression/bosses and interactive host.

## Original score/hi-score/combo HUD state

Score HUD logic is shared with original hud.cpp in hud_score.inc, with explicit
int16/int32/uint32 storage and an adapter for the resident hi-score field.
HudScoreBinding connects original scoring calls synchronously to actual digit
rendering after recording observer events. The common HUD background store
now supports score/combo digit quarters as well as life/bomb icons.
Tests cover 18 background quarters, both rows/pages, changed/decreasing digits,
new/preserved hi-score, current/max combo independence, forced repaint and
actual card collision driving score/hi-score presentation inline.
Linux/ARM64 full suites: 33/33 pass; Windows isolated score HUD test passes.
Remaining: full HUD background/stage/rank and initial setup, pause/audio,
lifecycle/progression/bosses, interactive SDL host and device verification.
No complete playable deployment yet.

## Complete regular HUD layout and MASK.GRF

The original stage/rank/full-HUD setup body is shared verbatim via
hud_layout.inc. HudLayoutBinding composes original life/bomb/score/combo
setup and timer rendering with stage-number background restore and centered
ASCII rank labels. The native MASK.GRF reader preserves the specialized
first-plane RLE repeat-plus-one rule, literal marker and final-run overshoot.
Real MASK.GRF produces 5376 bytes and consumes 2365 of its 2677 plane bytes,
matching the original 66-row loop behavior; no generic whole-image shortcut.
Tests cover initial/later-stage redraw, both pages, stage/rank positions,
forced redraw cleanup, malformed data and complete setup with real assets.
The real-asset HUD screenshot was visually inspected; Windows/ARM64 BMP SHA256
5e196e33b230e5a3f484a7863689979c7ab05780d5ae99fa251c6bc13e9dbd9f matches.
Linux/ARM64 full suites: 35/35 pass. Windows synthetic/real HUD layout tests
pass. The fixture draws a stage/HUD, not an interactive playthrough.
Remaining: pause/audio, bomb/miss/respawn, stage progression/bosses and the
interactive SDL host plus MyMini packaging/hardware validation.

## Original bomb animation and player integration

Original bomb.cpp now compiles natively with explicit palette-component
accessors and existing native bomb globals. BombBackendBinding connects the
actual player bomb action to this source. Native adapters render kuji GRC
glyphs with original byte clipping, clamp RGB4 palette changes, invert only
the B plane, copy pages and present graphics scrolling separately from text.
The host supplies VSync waits; no hardware/CPU emulator is linked.
Tests cover frame-0 pellet decay/invincibility, damage at 50, active palette
flash/scroll, 23 extra waits, original SE cadence and complete restoration
at frame 140 with 100 invincibility frames. Player action arming, one stock
consumption, pellet speed increase and completion are integrated and tested.
Synthetic trace fdff07922223e267; real KUZI1/KUZI2 trace 5aeb01cd158b1213.
Linux/ARM64 full suites: 37/37 pass. Windows synthetic/real bomb tests pass.
Real snapshots were visually inspected and Windows/ARM64 hashes match:
frame 24 d2978e57607094c5b65c5dc8f36c8a7346035506f070bece66aa789bad022432
frame 64 5b880c18997c86c7853757088d281ce0c4f571a81681841e4ba3b10b74648b34
Remaining: pause/audio, miss/respawn, complete stage progression/bosses and
interactive SDL host plus device packaging/hardware verification. Boss bomb
damage helper remains with the unported boss layer. No playable deployment.

## Original miss animation and life restart

Original player_miss_animate_and_update is shared verbatim in player_miss.inc.
MissBackend supplies blocking presentation delays; existing native bomb scroll,
HUD, shots, pellets, timer and speed adapters implement its original effects.
resume_after_miss mirrors life-loop termination: clears debug damage, sets
bomb_frame=200, skips respawn for stage clear/Game Over, decrements live/saved
life stock, runs original animation, clears hit, increments the original BGM
reload-suppression counter and sets 150 respawn invincibility frames.
It does not restart music; the original nonzero counter suppresses reload.
Tests cover 46 delay calls/82 frames plus 17 shake VSyncs, one miss RNG roll,
SE 5, both-page HUD updates, shots reset/pellet decay, capped bomb grant,
expired/running timer extensions, speed lowering and original X truncation
(e.g. starting at 100 returns to 300 rather than exactly 304).
Synthetic trace e255208d1fd34de5; real trace 838515a9ea101049.
Linux/ARM64 full suites: 39/39 pass. Windows synthetic/real miss tests pass.
Real snapshots were inspected; Windows/ARM64 BMP hashes match:
event 26 9284c4c90f3ca900dd8089e601def6932d107b209dc6180a0390cbe3adf3a4be
event 37 d68b4b1b28c269a4d3f7d6245ff0c2469d2c2574ed89351aacf4c2b6ffc95bc5
Remaining: pause/audio, full startup/stage progression/bosses and interactive
SDL session/host, then MyMini packaging and hardware verification.
No complete playable deployment yet.

## Original pause menu and blocking input integration

Pause code is shared in pause_menu.inc with only palette-component access
adapted. Original Shift-JIS bytes are generated as escaped literals from
hud.hpp and checked by the existing static-data generator. PauseBackend
polls host key snapshots into original input_sense and supplies menu delays;
it also references the current scene's uint16 continue counter.
Tests cover default resume, Escape release/repress, No-default confirmation,
confirmed quit, 40-percent tone excluding white, original 20-frame gap,
continue-counter side effect and simultaneous shot/Enter early-exit quirk.
The composed gameplay frame invokes actual pause and confirms no gameplay
frame/RNG advancement during its blocking wait.
Linux/ARM64 full suites: 41/41 pass. Windows synthetic/real pause tests pass.
Real menu snapshots were inspected and Windows/ARM64 hashes match:
pause 4e3a50ea6001188b282b060049ee5d66b3cad73c034693518173217674dba56f
confirm 94b12906ebc6f1328af9404831f5a869faa5f3e75d944ce9aac738b4f75e3cbb
Next priority: compose stage/session startup and the interactive SDL host
with these actual backends, rather than leave gameplay isolated in tests.
Full audio, stage progression/bonuses, continue/game-over/bosses and native
MyMini packaging/hardware verification remain. No complete playable deployment.

## Stage 1 session composition

StageSession now owns real stage-1 assets and all original gameplay backends,
with a single-session lifetime guard, startup shot gate, regular frame loop,
blocking pause/miss presentation, respawn and explicit terminal states.
The host supplies input and presentation waits; sound commands are drained
without replaying old events. Stage clear remains an explicit boundary;
progression, Game Over UI, bosses and audio playback are still unfinished.
Real-asset integration checks waiting movement without orb motion, shot launch,
orb advancement, actual miss/respawn and SE command, pause resume, terminal
clear and creation of a fresh session. Linux and ARM64 suites: 42/42 pass.
ARM64 tests execute through the PC-side runner, not on MyMini hardware.
Interactive SDL gameplay host remains the next task. No device deployment.
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
## Regular-stage bonus arithmetic

The four metric expressions are now shared with original bonus.cpp via
th01/main/bonus_values.hpp. Native stage_bonus_values preserves assignment
to signed DOS int before the 6553 upper clamp and unsigned x10 accumulation.
Tests cover stage 1, later-stage number, both upper caps, empty metrics,
overflow-before-clamp and all 65536 DOS narrowing bit patterns.
Linux/ARM64 bonus tests pass. This is preparation for the original bonus UI;
the interactive session still stops at stage clear and does not yet award
or render this bonus or proceed into stage 2. No hardware deployment.
## Regular-stage bonus box integrated

Stage clear now restores the stage palette, opens the original 8x4 stippled
box from bonusbox.bmp with original geometry/24 waits, renders original
escaped Shift-JIS labels/fullwidth digits, commits the bonus once and invokes
original HUD/score-extend handlers. The source bitmap and strings are generated
and checked with existing static data. Invalid negative numeral indices fail
explicitly instead of accessing memory outside the original numeral table.
The real session test forces its clear flag after exercising miss/pause; it
checks the bonus against the remaining lives and terminal ticks cannot award
it twice. This is controlled integration coverage, not a natural clear run.
Linux suite: 44/44 pass. ARM64 session test passes; Linux/ARM64 bonus screenshot
SHA256 d52ac743890c79445fb15f0834c19ca62b3a4483433f72c548c4e2ab99fe11ff.
Snapshot inspected visually. Next: acknowledgement, scene bonus history and
original between-stage cleanup/startup, then stages 2-4 and boss handoff.
Stage clear currently remains terminal; Hit Z prompt is original artwork but
acknowledgement/progression is not connected yet. Audio/hardware work remains.
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
## Original boss hit and bomb damage core

boss.cpp now compiles directly for native builds through BossHitBinding,
which requires live frame/life references and real HP/palette callbacks.
Native invincibility/HP references use int16 storage. The original
bomb_d_f.cpp is shared with only its frame parameter adapted to uint32.
Tests exercise orb HP/score/rebound, shot decay without HP damage, debug HP,
flash colors, frame-40/41 boundary and the periodic palette-reset source bug.
Original bomb damage requires zero reserve lives and a global frame multiple
of 50; it bypasses boss invincibility. This unusual rule is preserved/tested.
Linux suite: 45/45 pass. ARM64 boss-hit test passes through PC-side runner.
This is a boss subsystem, not a playable Singyoku fight: entity rendering,
HP HUD, phase AI, defeat/route selection and session integration remain.
Current SDL session still stops at stage 5 boss_pending. No device changes.
## Original boss HP HUD

hp.cpp now compiles natively: original increment/decrement/rerender routines
and doubled-point background addressing remain shared. Only GRCG foreground
writes use a framebuffer adapter. Seven columns by 15 rows preserve the gap;
red/alternating red-white/white thresholds follow original bitplane tiles.
Background capture/restore uses the existing native quarter-sprite HUD store.
BossHitBinding can call the actual hud_hp_decrement; its callback bridge was
renamed to avoid colliding with the real HUD function.
Tests cover exact increment completion, colors/gaps on both pages, odd/even
pair restoration, fresh background rerender, completed-animation no-op and
actual boss hit driving rendered HP changes. Linux suite: 46/46 pass.
ARM64 HP test passes; snapshot SHA256 matches Linux:
b9c9ea0a849f38dac8c7b5031ef212ba836f458ca4a80727034c0c9220c0d43c.
Synthetic snapshot inspected. Boss sprite/entity code and Singyoku phase AI
remain; this subsystem is not yet connected to the stage-5 session boundary.
No playable boss, audio or device deployment is claimed.
## Singyoku entity rendering and shared movement

Original CBossEntity inline movement/hitbox methods are reused with native
fixed-width BOS metadata. pos_set, both locked-move functions and hittest_orb
were extracted unchanged into entity_motion.inc shared by original/native.
BossEntityBinding loads actual BOSS1/BOSS1_2/BOSS1_3 sprite banks and implements
word-based put_8/unput_and_put_8 through page/alpha operations. Image/slot
metadata and the three original form banks are retained independently.
Tests cover actual sphere dimensions/eight cels, movement clamping/previous
position, lock timing, collision using previous orb coordinates, deactivation,
real alpha/background pixel results, invalid cel and independent bank release.
Linux suite: 47/47 pass. ARM64 real-entity test passes. Three-form snapshot
visually inspected and identical on Linux/ARM64. Line/wave/CBossAnim rendering
is not implemented by this adapter yet; original edge clipping requires more
boundary coverage. Singyoku phase AI/session integration still remains, so
stage 5 continues to stop at boss_pending. No device deployment or audio.
## Original Singyoku AI compiled and exercised

b05.cpp now compiles directly with native entity/pellet/HP/hit backends.
Original attack and phase bodies remain shared. Native changes preserve
16-bit counters/pattern union fields, rank selection and palette component
access. Entrance executes the original blocking 200-frame rotation/fade.
Host timing, music fade and defeat/route handoff are required callbacks;
defeat animation/route UI itself is not yet ported, not silently emulated.
The real-asset test executes normal-rank sphere firing, controlled HP threshold,
more than 3000 phase-2 ticks with actual pellet lifecycle updates, transformations,
slams and HP-zero defeat handoff. Player invincibility and debug damage are
used for deterministic coverage; this is not a natural playable fight or
proof that every random pattern/rank is covered. It frees all three banks.
Linux suite: 48/48 pass. ARM64 AI test passes through PC-side runner.
Inspected snapshot matches Linux/ARM64 SHA256:
bc1f5401f78256ffc36c602480fd711238bb7e14af84005d5d5eb09f3de70641.
The SDL StageSession still stops at stage 5 boss_pending. Next: connect the
original boss update at the correct frame-order point, boss stage startup,
miss/pause integration, then defeat animation/route/scene bonus/progression.
Audio and MyMini deployment remain unfinished. No new device changes.
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
## Original Continue menu core

Continue body extracted unchanged from menu.cpp into continue_menu.inc shared
with original/native; only native Yes returns a reload request instead of
invoking the DOS executable switch. ContinueBinding supplies input, frame
waits, font/framebuffer, saved resident fields, scene uint16 continue counter
and music-stop callback. Original doubled bold Shift-JIS text is generated
from hud.hpp. Score_highest update, saved score zeroing, credit lives +2,
point reset on Yes and the integer-unit pellet-speed reduction are retained.
Tests cover default Yes, No, Escape, strict >3000 timeout, confirm-before-Down
ordering, count consumption even on decline and the different music-stop paths.
Linux/ARM64 Continue test passes. Inspected Yes/No snapshots match SHA256:
Yes a7b7adce1a44c7c401976f4797c9123db4183280b212043f1a2d60eda78fd752
No 5d5d0108217e7ff11d0508ba4c7aab8f04706d3102f64572c1d1de9c51f76ef3
This core is NOT connected to StageSession yet. Original pre-Continue Game Over
background/fade and score registration, followed by actual MAIN-style restart
of the current stage, still need integration. Existing preview still stops
after Game Over; no new playable Continue or device deployment is claimed.
## Original score-file I/O before score registration

formats/scoredat.cpp now compiles directly with native binary-file adapters
and an explicitly bound score directory. It retains original 247-byte REYHI
layout, four rank filenames, 0x9c name cipher, defaults, signed score/stage,
route kanji codepoints, first-four-magic-byte validation and in-place name
encoding during save. CP932 literals are generated encoding-independently.
Native-only safety adaptations: mutable local default-name copy, little-endian
host gate, leaf-only filenames, short-file rejection before allocation and
null pointers after freeing. Fixtures stay under build directories; no source
game or extracted assets are written. This is not yet a registered score UI.
Tests cover every cipher byte, all four recreated files, default fields,
entered name/score/signed-stage/route roundtrip, weak original magic check,
invalid header and truncated input. Linux full suite: 53/53 pass. ARM64 file
I/O test passes; recreated score files match byte-for-byte across both builds.
Next: original registration table/name entry and its Game Over integration,
then actual Continue reload. Existing preview still stops after Game Over.
Generator now leaves unchanged generated files untouched to avoid needless
rebuilds; existing generated-data validation remains green.
## Original high-score table and name entry compiled natively

hiscore/regist.cpp now compiles directly against native score-file, graphics,
fullwidth-number and input adapters. Original ranking/insertion, alphabet
navigation, eight-kanji cursor/End logic and name save body are shared.
CP932 UI strings/codepoints are generated from original title/regist headers.
Native ShiftJISKanjiBuffer keeps the original byte/word views; one conditional
cast handles modern C++ pointer typing. RegistBinding explicitly presents while
polling because original idle loops draw directly to VRAM without delays.
FONT.ROM renderer now supports original FX_CLEAR_BG and FX_REVERSE after font
weight processing; narrow glyph inversion uses ff00, wide uses ffff. Synthetic
font tests verify background clearing, holes and glyph-width boundaries.
Real tests enter fullwidth a/b, insert/shift a score with stage/route, check ties
rank ahead, fill eight glyphs and confirm End, and ensure a nonqualifying score
waits for press/release without touching the file. Separate fixture directories
only; source game/extracted assets remain untouched.
Linux full rebuild: 54/54 tests pass. ARM64 registration/font tests pass.
Inspected UI screenshots and persisted rank file match Linux/ARM64 SHA256:
initial 35891b7449f34d7cc0c431b30d8b519ec3667130909d081e30c1f6291763cdef
entered c40a32e79667c43bdd14262a0c3a8d0108c8b1e324f89a716fed874b90f293d6
score dd5ad1930fb6badc48c720beaba9470b015344b27b0376c95baea1e7698c7d75
Not yet connected to StageSession. Next: original Game Over background fade,
registration then Continue, MAIN-style current-stage reload on Yes and explicit
return-to-title boundary on No. Preview still stops after Game Over. No new
Windows/device deployment; audio, further scenes and full title remain pending.
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
## Original TOTLE screen row transition

The original totle_pagetrans_animate scheduling loop is shared verbatim in
totle_transition.inc. Native builds replace its hardware EGC setup with direct
four-plane 32-dot row shifts, preserve the eight-row activation batches and
source palette-step cadence, and use stack arrays for exception-safe temporary
state. Even rows shift left and fill from successive page-1 chunks; odd rows
shift right and fill in reverse chunk order. CLEAR3.GRP provides the real
screen and palette. Every row finishes after 70 two-frame waits (140 frames).
Synthetic tests check initial inactivity, both shift directions and untouched
next batch; full-page/palette traces guard every callback. Real screenshots
at iterations 30/70 were inspected and match Linux/ARM64:
30 8d026af15f87c77fa81726d8527977b00ef7e77bf5d1e409809ba70114d031ec
70 c21b3bd2be11181adf3afeaf074924c90c448660d45c974670b5f44afc0f2d21
Synthetic trace 526e81c9d9f23b4d; real trace 4b6c02cf8ae757a6.
Linux full suite: 56/56 pass. ARM64 synthetic/real transition tests pass.
This is NOT yet linked into the playable post-boss flow: original digit rolling,
NUMB sprite rendering and TOTLE bonus arithmetic/commit/acknowledgement remain.
Their DOS-width expressions need care: stage_timer*5 wraps as unsigned 16-bit
before assignment to original long; original combo product is signed 16-bit.
Next: those routines, then route-specific next scene. Existing SDL post-route
boundary remains; no new Windows or device deployment in this step.
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
## Route-specific stages 6-9 integration

Selected Makai/Jigoku now loads STAGE1/ST1 or STAGE2/ST2 after the original
Singyoku route menu and TOTLE acknowledgement. Stage layouts use scene-local
indices; regular timers retain the selected route. Scene handoff preserves
score/lives/bombs, rebuilds actors and HUD, and waits for the original shot gate.
Continue reloads the same route/stage. Initial card-flip cycle uses original RNG.
Stage 10 stops at boss_pending: neither route boss is implemented yet.

Verification: Linux suite 58/58 passed, including real Game Over animation,
integrated session and new two-route progression test. ARM64 route_scene_test
passed under PC-side qemu with original assets, including stage 6 Continue and
all stage 6-9 layouts/timers. ARM64 stage_preview rebuilt. This is not a MyMini
hardware test or deployment. Windows preview still needs rebuilding for this
change. Audio playback remains absent; visual miss/Game Over effects are wired.
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
### YuugenMagan original integer widths (2026-10-29)

Added shared `yuugenmagan_types.hpp`: DOS retains `int`, while native uses
`std::int16_t`. The eye-transition frame reference and pentagram angle now use
this type, including the isolated adapter's transition state. This prepares the
transition function to accept the full boss's 16-bit phase clock without widening
that clock or adding a temporary copy.

Validation: full Linux build and 98/98 CTest cases passed. ARM64 transition and
regular-pentagram binaries also passed under host-side QEMU; regular pattern trace
remains `ea20878fd8169e91`. This is not a MyMini hardware test. Full YuugenMagan
main/HP/defeat integration and preview availability remain pending.
### YuugenMagan complete original boss core (2026-10-29)

`yuugenmagan_core` now compiles the original `b10m.cpp` inside a native namespace,
using the same extracted attack/transition bodies as the isolated tests. Five
real `CEyeEntity` instances replace DOS base-to-derived storage casts only in the
native branch. All main integer locals and union members retain DOS 16-bit width.
Load resets local phase/hit/pentagram state; defeat handoff is idempotent and
missile/BOS banks are released on free. Missiles update inside original main,
before the boss attack; hosts must not update them a second time.

`yuugenmagan_test` verifies the 331-tick intro, real orb damage/5000-point hits,
timeout transitions across phases 1-13 with live pellets/missiles (6982 ticks from
phase 3 to 13), pentagram return/re-entry, final vulnerable-eye cycling,
invincibility, defeat, bank cleanup, and reload. Additional HP-threshold routes
cover all four ranks and defeat during shrink phase 11. Linux and ARM64 binaries
pass. Full Linux CTest passed 99/99 before the final test-only HP-route extension;
the extended test passed separately on Linux and ARM64.

`native/check_yuugenmagan_source.py` expands shared bodies, removes native branches
and normalizes DOS type/component aliases; resulting DOS source matches HEAD.
Linux and ARM64 final-phase framebuffer BMPs matched SHA256
`0635b8eee4f39f534e8b4b78ddb1b5ddd5b5c1135d0c169c6983af8b3252993d`.
These are host-side checks, not hardware/DOS frame parity. StageSession, Continue,
TOTLE-to-Makai-stage-11 integration, Windows playable preview and MyMini testing
remain pending; the existing preview has not changed.
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
### Elis real entities and original wave teleport (2026-10-29)

Started Makai stage 15 with shared `elis_entities.inc`, `elis_surround.inc`,
`elis_background.inc`, `elis_setup.inc`, and `elis_teleport.inc` from b15m.cpp.
Native uses actual CGirlEntity/CBatEntity instances rather than DOS storage casts.
The isolated controller loads BOSS5/BOSS5_2/BOSS5_3 banks, retains original base
positions and hitboxes, caches twelve 32x32 background tiles, and executes the
original wave teleport and five-star animation. This controller accepts explicit
targets; full main's per-frame random target evaluations are not integrated yet.

Linux 102/102 CTest passed. ARM64 helper passed under host-side QEMU; both traces
`db174c36f9a563fd`. Test verifies the 81-tick cycle, position change at 52,
hitbox disabled 20-75 / re-enabled 76, original cel sequence, page-one background
restoration, repeated targets including byte alignment, 200 star RNG draws per
cycle (also after stars stop drawing), and bank cleanup. Inspected Linux BMP.
`native/check_elis_source.py` verifies expanded DOS branch still matches HEAD.
No preview update or MyMini installation: Elis entrance, transformation, attacks,
full HP/phase/defeat core and live StageSession remain unfinished.
### Elis original sphere/wave entrance (2026-10-29)

Added shared `elis_grc.inc` and `elis_entrance.inc`. The helper now executes the
original two-sphere/trail rotation, offscreen departure, wave appearance and
slight ripple, then hands off to teleport. Boss GRC sprites use a separate bank
from bomb kuji sprites. Shared `entity_wave.inc` now supplies original per-line
wave blit/unput to the native entity renderer; native math uses the same Sin8
lookup. Native-only initial trail positions are placed outside the framebuffer
to exclude the original uninitialized-stack reads; DOS code remains unchanged.

Tests verify 799 VSync waits / 259 callbacks, active final girl at (256,96),
phase-clock reset, unchanged RNG, correct page-one background and teleport after
entrance. Linux full build and 102/102 tests passed. ARM64 test passed under
host-side QEMU. Entrance BMP matches across Linux/ARM64 SHA256
`7bf99ee397d00759356a4360d97847d9ca7ac6dea233bfebb087b1322cd3924f`.
Expanded DOS Elis plus shared entity motion/wave source match HEAD by audit.
Elis attacks/transforms/full boss HP/defeat and StageSession remain unfinished;
Windows preview and MyMini installation have not changed.
### Elis girl/bat transforms and original bat flystep (2026-10-29)

Shared original rift rendering, both transformations, form coordinates, bat
movement/cel schedule and bat random-target flight in `elis_rifts.inc`,
`elis_transform.inc`, `elis_form_coords.inc`, `elis_phase5_enum.inc`,
`elis_bat_move.inc`, and `elis_bat_flight.inc`. The isolated controller can now
execute entrance -> teleport -> girl/bat transforms -> flight -> girl, with
original sprite/background operations, screen-scroll shake and captured SE.

Tests cover repeated 101-tick transformations, 42 scroll-wait callbacks each,
21 SE7 commands each, exact per-frame rift RNG draws, original missing first bat
sprite, restored background, and centered girl/attack pose on return. Real bat
flight retains original movement-before-target-update ordering, two-tick delay,
three-cel/three-frame animation, and post-flight center/clamps. The fixed fixture
flies 201 ticks to bat position (306,198), identically on Linux/ARM64.

Linux 102/102 CTest passed. ARM64 extended test passed under host-side QEMU.
Transformation BMP Linux/ARM64 SHA256
`6e631e489ca3af084120fee9533a5dbdee0709ad26c0af7f65058013a767336f`.
DOS Elis and shared entity source audits pass. This is helper coverage, not a
complete Elis fight: offensive patterns, full HP/phase/defeat and StageSession
integration remain pending. Audio commands are captured, not played. No Windows
preview update or MyMini deployment.
### Elis four original bat firing patterns (2026-10-29)

Shared `elis_bat_attacks.inc` compiles all four original bat patterns: rank-based
slow spreads, random narrow/wide aimed pairs, gravity rain, and alternating
2/3-way aimed bursts. Native rank selection uses an explicit enum conversion
instead of aliasing enum storage through an int reference; DOS branch preserved.
The isolated flight controller can execute any of these after original bat
movement and flystep update, including the final phase-clock-reset frame.

New `elis_bat_attacks_test` checks 400 ticks per pattern/rank, exact volley groups,
muzzle coordinates, aim/spread velocities, launch speed plus rank/minimum clamp,
rain gravity, and RNG consumption. Totals: slow spreads 75/125/125/125 by rank;
random pairs 200; gravity rain 100; alternating bursts 100. Also runs actual
pellet update/render during armed bat flight. Linux and ARM64 tests pass with
trace `c2b309f6632dc78e`; existing transform/flight test also passes on ARM64.
Linux full CTest passed 103/103; expanded DOS source audits pass.
Human-form attacks, full Elis HP/phase/defeat and StageSession integration remain
unfinished. No Windows preview change or MyMini deployment in this step.
### Elis symmetric stacks/arc and five curved rings (2026-10-29)

Shared `elis_symmetric.inc`, `elis_curved_rings.inc`, star return enum and pattern
state. The isolated controller runs both complete original human attacks,
including attack-cel rendering and completion semantics. Symmetric attack emits
8/8/8/20 pellets at ticks 50/60/70/80 and finishes at 120. Curved rings emit five
14/16/18/20-pellet rings (by rank) at 10/16/24/32/40 and finish at 61.

New human-attack test checks mirrored muzzle offsets, per-stack speeds, ring
angle truncation, rank speed adjustment, no RNG, three SE7 commands only for the
symmetric attack, and inert completion. Also preserves the original curved-ring
ending quirk: unblits the sprite from page 1 and leaves that page selected.
Linux full CTest passed 104/104. ARM64 test passed under host-side QEMU with the
same trace `fa0a22897931b614`. Expanded DOS/source audits pass.
Remaining human patterns include lasers, missile/rift attacks, circle/sphere
attacks and final safety/laser patterns; complete Elis phase/HP/defeat integration
and live preview are still pending. No Windows/MyMini deployment in this step.
### Elis downward rift missiles: first native check (2026-10-29)

Shared `elis_rift_missiles.inc` now executes original downward missile pattern
as human helper pattern 3. It requires entrance-loaded GRC effects and an active
MissileBackend binding; caller owns projectile update timing. Linux extended
human-pattern test passes across all four ranks: 37 launches every third tick
from 63 through 171, 26 rift sound commands, final launch before completion and
phase clock reset. Existing symmetric/ring tests still pass; DOS audit passes.
This step has not yet verified exact rift-coordinate/RNG trajectories, sustained
missile updates, ARM64, or a new full-suite run. Those checks and remaining human
attacks/full boss integration are still pending. No preview/device change.
### Elis rift-missile trajectory/RNG verification (2026-10-29)

Extended the human-pattern test with an independent seeded RNG schedule and all
five rift positions. Each launch is checked against pre-update rift coordinates,
rank angle range and exact integer-vector-to-subpixel velocity, including ticks
where launch and rift update coincide. Corrected test-only reference geometry to
TH01's full 640-pixel playfield (surround width 192, base X224); native source was
already correct. Also corrected a dormant bat-to-girl test clamp to X0..512.

Sustained test runs original missile update before the boss pattern, observes
multiple live missiles and floor/edge explosions, then drains all slots with an
invincible player. Linux and ARM64 extended human tests pass with matching trace
`9601321ddff47094`. Expanded DOS/source audit passes. Full Linux build succeeded.
This verifies the previously pending rift pattern checks; remaining Elis human
attacks and full fight/StageSession integration are still unfinished.
### Elis original eleven-laser sweep (2026-10-29)

Shared `elis_lasers.inc` runs human helper pattern 4: original circle growth,
hand/still poses, random sweep direction, eleven laser spawn requests at ticks
70..170 every ten ticks, then completion/reset. Native rank speed helper keeps
original multiply-by-eight representation and integer truncation.

Extended human-pattern test covers both sweep directions at all four ranks,
left-eye origin (316,124), 64-pixel target progression including both edges,
cyclic slots, exact Sin8/Cos8 direction, actual extend speeds 6/6/7/8, width four,
color seven, moveout age 25, eleven SE6 commands and one RNG draw. It updates
actual lasers between spawns and verifies all eventually leave the screen.
Linux and ARM64 tests pass with matching combined trace `a9a5d33560926694`.
DOS/source audits pass. Full Linux build succeeded. Preview/MyMini unchanged;
remaining Elis circle/sphere/final patterns and complete fight integration are
still pending.
### Elis summon/flash circle volleys (2026-10-29)

Shared original `elis_bigcircle.inc` and `elis_circle_pellets.inc` implement human
helper pattern 5: four-quadrant summon/flash, then 32 attempted circle-origin
volleys. Native enum rank selection uses explicit conversion; DOS code preserved.
Tests verify no early emission/RNG, completion at tick 162, left-eye X origin,
rank groups and exact aim/spread velocities, launch-speed minimum, Normal-only
random angles, and single SE8. Includes original single-shot Easy aim offset.
At original base height accepted counts are 23/23/69/100 due top clipping and
capacity; at lower test position they are 32/32/96/100, verifying the original
hundred-pellet limit. Native rendering retains original fractional quadrant arc
endpoint conversion through shared macros.

Linux and ARM64 tests pass with matching combined trace `481dcfaa75ea60bd`.
Full Linux build and DOS/source audits passed. Remaining Elis sphere clusters,
random rift pellets, final human patterns, full fight and StageSession integration
remain pending. No preview/MyMini change.
### Elis ten-sphere cluster attack (2026-10-29)

Shared `elis_sphere_clusters.inc` runs helper pattern 6, retaining original
sphere positions, GRC/rift overlay behavior, removal timing and downward random
clusters. Test covers both left/right branch positions and all ranks: spheres
appear at 10..100, fire at 110..200, removal through 230, completion at 251.
Exact seeded sphere coordinates and each pellet's angle/speed are checked;
rank totals are 50/70/90/130 emitted pellets when slots are cleared between
volleys. Ten SE7 commands match original. This is emission coverage; does not
claim 130 simultaneously live pellets (original pool remains 100).

Linux three related Elis tests pass. ARM64 extended human-pattern test passes
with matching combined trace `14b1d681d9f4e52d`. Expanded DOS/source audit passes.
No new full-suite run this step (last full suite: 104/104 before this addition).
Remaining random rift pellets/final human patterns and complete Elis fight /
StageSession integration are pending. Preview/MyMini unchanged.
### Elis two-stage random rift pellets (2026-10-29)

Shared `elis_rift_pellets.inc` implements human helper pattern 7. First stage
uses rank intervals 6/3/2/2 between ticks 60 and 120; second fires every frame
120..199. Preserves original unsigned-byte angle-offset division/multiplication,
pre-redraw rift positions, seeded position/cel/shot RNG and cleanup at 221.
Tests cover exact per-shot position/velocity and RNG on all ranks, emission totals
89/99/109/109, and 36 rift redraw sounds. Dispatcher now uses explicit switch.

Three Linux Elis tests pass; ARM64 extended human test matches combined trace
`4145b568737ddb8c`. Expanded DOS/source audits pass. Last full Linux suite remains
104/104 before the sphere/rift additions; related tests rerun for these changes.
Remaining safety-circle/final laser human patterns, Star-of-David/phase control,
complete Elis HP/defeat and StageSession integration are pending. No Windows
preview update or MyMini deployment.
### Elis safety-circle and final spread/laser/ring helpers (2026-10-29)

Shared `elis_safety_circle.inc` and `elis_final_lasers.inc` supply human patterns
8/9. Safety circle retains original asymmetric inclusive boundaries, invincibility
exception, real 100ms hit delay (injectable for headless tests), top rain, late
aimed shots and stale bat-relative tail muzzle. Tests cover complete 382-tick
safe route and both edge hits; totals 62 Easy / 111 other ranks with exact rain
RNG and tail origin after a real girl/bat/girl transform.

Final combined attack tests cover paired wide five-way wing volleys, fixed-speed
width-eight lasers and clipped final ring at tick 162. Emission totals by rank
51/61/81/91 and laser counts 2/2/3/4; no RNG. Linux and ARM64 extended human tests
pass. Trace `4145b568737ddb8c` covers earlier shot trajectories; these two final
helpers are checked by explicit assertions, not added into that hash. Full Linux
build and expanded DOS/source audits pass.

All nine offensive human helper entries are now implemented alongside four bat
patterns, entrance/teleport/transforms/flight. Star-of-David geometry, original
phase controllers, full HP/hit/defeat/reload and live StageSession remain pending.
No preview or MyMini deployment in this step.
### Elis six-point star and original phase-three attack dispatcher (2026-10-29)

Shared `elis_star_lines.inc`, `elis_star.inc`, and `elis_phase3.inc`. Original
circle/six-edge star summon, flashing and cleanup now run natively. Controller
supports original phase-three dispatcher with fixed selected attack, including
its reset path; random attack choice remains part of future full main integration.
Tests check 132-tick animation, white/FX/white chord pixels, no RNG, one SE8,
background restore and repeated runs. Original dispatcher handoffs are exercised
for ring (193 total ticks), sphere clusters (383), and rift pellets (353).

Linux full build and 105/105 CTest pass. ARM64 star/controller test passes under
host-side QEMU with matching star-frame trace `94ac2edcf63acde2`. Inspected Linux
frame 81 image; Linux/ARM64 BMP SHA256
`8030184117362744db1163180f4ed080e701fc7e9c9c246a20559d08646a83ff`.
Expanded DOS and shared entity source audits pass.
Remaining: phase-one/final controllers and complete original Elis main with HP,
invincibility, defeat, reload and live StageSession. No preview/MyMini update.
### Elis original continuous final phase controller (2026-10-29)

Shared `elis_phase5.inc` compiles original phase_5_girl/phase_5 with native-only
observer hooks. Isolated controller now runs bat motion before original final
phase update, random bat/girl attack selection, star prelude, both transforms
and subsequent flight cycles. Original reset path reconstructs controller state.

New final-controller test runs real pellets and shootout lasers across all ranks,
covering all four bat patterns and all three girl patterns, with 10/24/24/24 form
changes over 5433/10124/10124/10463 ticks. Linux and ARM64 match state trace
`7e1a86819f4c691b`; invincible-player actors and clock resets checked throughout.
Linux full build and 106/106 CTest pass; expanded DOS/source audits pass.

This is continuous final-phase coverage, not the full Elis fight. Full original
main, HP/invincibility, early phases and transitions, defeat/reload and live
StageSession remain pending. No Windows preview or MyMini deployment.
### Complete original Elis core compiled and exercised (2026-10-29)

`elis_core` now compiles original b15m.cpp in a native namespace, retaining shared
attack/entrance/transform bodies. Native backend supplies three BOS banks, GRC,
twelve background tiles, missiles, timing, palette restore and idempotent defeat
callbacks. Main int locals use explicit 16-bit type; DOS alias normalizes to int.
Native load resets actual derived entities, pattern state and function-local
main state on its first tick. Source audit still matches expanded DOS HEAD.

New full-core test loads fourteen HP, runs original 799-wait entrance, lets HP
background initialize, then verifies real human/bat orb damage, 7000-point hits,
invincibility, HP-ten/HP-six transitions, central teleport, final bat flight and
human return, defeat in both forms, inert defeated state, bank cleanup and reload.
Actual missiles/pellets/lasers update in original ordering; defeat callback is a
core boundary here (generic defeat animation remains host/session integration).
Known original early-hit-before-HP-bar-initialization behavior is not simulated
by this fixture; HP bar is allowed to initialize before injected orb collisions.

Linux full build and 107/107 CTest pass. ARM64 full-core test passes under
host-side QEMU. Inspected Linux final-form snapshot; ARM64/Linux BMP SHA256
`36ea82874f659664b68f85afd024de7289327b98118ddb91ff4d4570379b8b2f`.
StageSession/Continue/TOTLE-to-stage-16 and playable Windows preview remain
pending; no MyMini deployment. Sariel/audio/title/ending still remain afterward.
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
### Sariel original non-collidable BOS animation support (2026-10-29)

Started Sariel with native CBossAnim load/put/free, using animation banks separate
from collidable CBossEntity banks. Two animation banks retain original wand/dress
assets alongside the shield entity. Decoded opacity represents the original
load-time inverted alpha; renderer preserves raw foreground-plane OR, byte
alignment, selected graphics page, original word clipping and bottom bounds.
Sariel original call sites explicitly use both pages, so put_8 honors the selected
page (despite the header's broad page-zero wording).

New Sariel animation test loads BOSS6_1/BOSS6_2/BOSS6_3, verifies all cels on both
pages against decoded alpha/planes, unaffected opposite page, invalid cels,
bottom clipping invocation and independent bank cleanup. Inspected composed
BOSS6_A1 screenshot. Linux and ARM64 tests pass with matching trace
`e3df41e9c6bfdc71`; BMP SHA256
`7bbe06f7819ef586e0bb1bc1847db9ff946120bb8e1eab22ec09b84b25a9452e`.
Full Linux rebuild succeeded after shared backend layout change.
Sariel entrance, scheduling, patterns/HP/defeat and live StageSession are still
pending. No Windows preview rebuild or MyMini deployment in this step.
### Sariel original scrolling/diagonal entrance (2026-10-29)

Shared `sariel_entrance.inc` and `pgtrans_diagonal.inc` now run in a native
SarielEntrance adapter. Original L/H GRP pages scroll/copy 400 lines, then the
130-frame diagonal interleave transition reveals BOSS6_A1. Mask bytes match
source ileave_m.bmp; palette loop and diagonal scheduling compile shared source.
Music/SE loads/play are host commands only, not audio synthesis.

Test runs entrance twice, verifying 801 one-VSync waits, each copied scanline,
scroll sequence, 130 delays of 40ms (5200ms), first interleave tile, final identical
battle pages/palette, text/page/scroll reset, interlace and seeded RNG, plus
stage reset and TENSI.MDT/Miko.SE commands. Linux two Sariel tests pass; ARM64
entrance test passes under host-side QEMU. Inspected Linux final background;
Linux/ARM64 BMP SHA256
`a70563ac49eeeb8a6f91b37d6eedc6b5d469d0504ac71e3ddd9cd5130f0368e7`.
Expanded Sariel and pgtrans DOS bodies match HEAD by new source audit. Last full
Linux suite was 110/110 before entrance addition; related tests rerun here.
Sariel setup/animation scheduling/attacks/HP/defeat and StageSession remain
pending. No playable preview update or MyMini deployment.
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

## Original ending picture copy (2026-10-30)

Extracted the original `end_pic_show` unchanged into
`th01/end/pic_show.inc`, shared by the DOS ending and a native planar-storage
adapter (`ending_picture.cpp`). Expanding the include reproduces HEAD's
original ending source byte for byte. The native test checks every pixel of
all four source quarters, centered destination borders, unchanged source
page/palette and invalid-quarter rejection. Linux and ARM64 builds pass;
Linux and host-run ARM64 execution pass. This is ending infrastructure only: the original
ending sequences and StageSession handoff are not connected yet.

## Original ending palette fades (2026-10-30)

Moved the original GRP tone/fade functions unchanged into
`th01/formats/grp_palette.inc`; the DOS source and native `EndingPalette`
share that implementation. Include expansion reproduces the original
`grp.cpp` byte for byte. The adapter retains RGB4 integer arithmetic,
color-zero behavior, tone across GRP loads, and all four 20-step fade timings.
Focused Linux tests pass (ending_picture and ending_palette); the ARM64
palette executable also passes under the host runner. The test covers tone
requests -5 through 205 across every color component, loading a new palette
at half brightness and fade callback counts/frame totals. Ending sequence
playback, captions, verdict/registration and StageSession integration remain
unconnected; no new Windows preview or device install is claimed.

## All four original ending sequences (2026-10-30)

The original text typing and complete ending/slideshow sequence bodies now
live in shared `th01/end/text_type.inc` and `sequence.inc`. Expanding these
and pic_show.inc restores HEAD's original end.cpp byte for byte.
`EndingSequence` runs the actual original good/bad branches for both routes
with native picture/palette storage and explicit wait, font, asset, audio
command and verdict callbacks. Generated captions preserve original bytes,
including the extra NUL typed for the Stage 5 slideshow caption. Original
shake/boom timing (including immediate overwritten pictures) is retained.

Real-data test uses all original GRPs and FONT.ROM. Linux and host-run ARM64
pass all four routes: Jigoku good 5528 frames/176 typed glyphs/8 GRPs,
Jigoku bad 2910/44/6, Makai good 6174/178/10, Makai bad 3202/44/7. Asset
order, caption placement, good/bad selection, slideshow gating, music command
order and exactly one verdict handoff are checked. Final BMPs match between
architectures for all four cases. Makai good image visually inspected: Sariel
slideshow and original font captions at centered positions. Good-ending BMP
SHA256s: Jigoku bf4ecfbd7fa8e4c81c9f8be6ca7ed5dfc5cb1f1ba81c878644b2e3554510d539;
Makai d091cafcb38ba9862b73884e21838e0a9f200d85667e219e2e7541f9abf250db.

Still standalone: not yet connected to StageSession/SDL preview. Verdict UI,
registration handoff, title return and real audio playback remain unfinished.
No Windows preview rebuild or MyMini deployment in this checkpoint.

## Native original verdict screen (2026-10-30)

Original verdict rendering/title calculation extracted unchanged into
`th01/end/verdict.inc`. Expanding all ending includes reproduces HEAD's
end.cpp byte for byte. Native EndingVerdict owns the ending result snapshot
and adapts original page copies, GRP palette, typing, input, RNG and score
registration callbacks. Generated complete ending/title Shift-JIS data passes
the generator check. Native formatting passes actual unsigned long for DOS
32-bit `%lu` values, preserving the separately signed title comparisons.

Real ENDM_A.GRP/FONT.ROM tests cover both routes, confirmation and original
2000-poll timeout, exact registration parameters/order, text removal, 32-bit
score 4294967295 and all 18 titles across the six skill levels/three RNG
groups. Linux ending suite 4/4 passes. ARM64 verdict execution passes using
the host runner. Captured verdict images match byte for byte across builds;
Makai image visually inspected (Japanese font, original layout and title).
SHA256: Makai da511bdd340c99c28cf7e941d645c2a602b67614965a5c6db69c22f28b3d69a4;
Jigoku 893038c9d69f326c24a7bfb6f201cd54625877f850fa61bb5c4d52d496fe2324.

Registration currently uses an explicit callback in the standalone test;
the actual registration UI and StageSession/SDL ending connection are still
next. Real audio/title and hardware validation remain unfinished. No Windows
preview update or TH01 MyMini installation in this checkpoint.

## Live ending, verdict and original name registration (2026-10-30)

StageSession now advances from ending_pending through the original 120-frame
MAIN delay, native ending/slideshow and verdict, then calls original
regist_menu() with the cleared-route constants. Shared EndingPalette state
persists throughout via play_ending. The result snapshot includes current
score, credit high score, rank, extra lives, all four scene Continue counts
and selected route. Original input cutscene flag is set during ending; the
resident result is cleared after name registration. The visible graphics page
is cleared at the binary transition as original FUUIN graph_start does, so
no boss background remains outside centered ending pictures. Registration
receives the dimmed verdict palette with original text colors 3 and 7.

Live Sariel and Konngara tests now complete ending and real registration,
reload the score file to verify final score/cleared stage/route, check clean
picture borders and prove a further step cannot replay the ending/bonus.
Both ARM64 live tests pass with host QEMU. The full Linux suite passed
133/133 after integration; final transition adjustment is being rechecked.
Windows build script includes the new native ending modules.

Remaining: native title/options (ending currently returns to title_pending,
so SDL preview closes), actual music/SE playback and MyMini installation/
performance validation. No TH01 deployment to the device has been made.

## Unified native title/options and playable return path (2026-10-30)

Extracted original OP input, graphics, white-line title effect, menu rendering
and menu update bodies into shared title_input.inc, title_graphics.inc and
title_menu.inc. Expanding includes/static-storage adapter macros restores
HEAD's original op_01.cpp byte for byte. Instance-owned original local state
allows repeated title creation after gameplay. Packed REIIDEN.CFG storage,
rank, BGM mode, player stock and Music Test are connected. GameOptions reach
StageSession before original HUD/score initialization. Continue can start any
stage 1-20; SDL retains the last stage/route when returning to title.

Title/menu/config test passes on Linux and ARM64; original menu screenshot
SHA256 a39acc3981d3d083cc8a24eb594c2106d76fd0adb70c0395a65a8d5ee983dbf7.
Default SDL title snapshot matches Windows/Linux/ARM64 at 4 host ticks/220
presentations: d203c2d9ff99694bc8312b5a1d7abc7c2d70a41c684b807d3a418f7bd729115d.
Windows unified launcher is TH01 Native.cmd; no automatic frame limit.

## Native MDT BGM and SDL audio integration (2026-10-30)

New MdtPlayer interprets music data natively: channel/instrument/envelope
banks, notes/rests/ties, three loop forms and escapes, macros, tempo,
articulation, transpose, detune, portamento and software modulation. Format
work references the original user's MDRV98.COM (SHA256
 e14f26007f1a71d747f50deeb0f338065466e87ad142268416a1bcbb14b8ae7d),
static host disassembly and public MDT format/decompiler documentation.
No original executable is run or linked by the music renderer.

OPNA synthesis uses unmodified BSD-3-Clause ymfm files pinned at
81aec25ccbb98f4873a255f7551ac4dadac59b4a; license is copied beside the Windows
binary. FM/SSG render at the core's native rate and mix to 48kHz stereo.
Rhythm uses six 48kHz mono PCM samples from the user's existing Touhou98
Experience pc98lib/emulator directory. prepare_rhythm_assets.py validates
and copies these to the native asset directory; originals stay untouched.

SdlMusic owns the audio device/callback. Immediate SessionHost audio callbacks
keep original loads/plays/stops/fades audible during blocking boss entrances,
ending and verdict; old command queues remain inspectable. Stage/scene themes
and all Music Test selections are connected. Konngara entrance music callbacks
are now forwarded, as are Sariel's two-form music changes. Gameplay respects
BGM Off, while the original title/ending enable behavior is retained.

Validation: all 19 MDT files traverse 15000 native sequencer ticks or finish
(INIT is intentionally silent). The 18 music files render nonzero PCM without
clipping in a 6-second-per-file check. 12-second REIMU render peak 8501,
clipping 0, PCM hash 106cf62a3250eadd, WAV SHA256
6de9d74d83828ca77859d46081976bf153ea6ec9ada207b50c48833c349f9c30 is identical
on Linux/host-run ARM64. Linux paced SDL title-to-stage run rendered 289792
audio frames; Windows normal audio-device run rendered 222208 and entered
stage 1. Linux full regression suite passes 137/137 after integration.

Remaining fidelity work: no hardware/reference-driver audio comparison yet;
fade mix/rate and less-used modulation/portamento cases need longer checks.
Unused MDT fade/hardware-LFO/sync commands are not a complete general MDRV2
implementation. Native .MDE sound effects are still pending. Title Continue
currently carries stage/route, not every original resident-history field.
MyMini deployment, audio/performance checks and broader original-game parity
remain outstanding. The overall port goal is still active.

## Start entrance and native original sound effects (2026-10-30, validated on host)

The user's observed immediate Start-to-gameplay was a real missing boundary:
StageSession stage 1 did not call the original scene-title or STAGE-number
animations. Native scene_title_text_animate now reproduces the six original
JIS characters from th01/shiftjis/entrance.hpp (including the star), 8 frames
each, with 16x centered reverse-black TRAM masks revealing the background.
StageSession calls this on scene/boss reset stages (except the two final
bosses with their own entrances), followed by existing 35+35-frame STAGE/number
text. Initial stage 1 therefore presents 118 entrance frames before player
control/launch. No controls can bypass this constructor sequence.

INIT.MDT and ZIGOKU.MDE load before the opening, matching original MAIN setup.
Also corrected stage-15 BGM selection against main_01.cpp: Makai/Elis uses
KAMI2.MDT and Jigoku/Kikuri uses KAMI.MDT.

MdtPlayer now accepts original one-based MDE effects directly, parsing its
effect directory, per-effect channel pointer tables and shared SSG envelope
bank. MusicSynth plays effects on the same OPNA SSG, shadows background
registers while an effect owns channels, and restores them when it finishes.
ShotBackend optional immediate dispatch carries SE during blocking animations;
SessionHost forwards it to SDL, and Konngara's separate SE queue is flushed
without discarding inspectable command history. The BGM sequencer keeps running
while effects override its sound channels.

Focused Linux verification passes: all six mask phases/48-frame timing/SE14
requests/cleanup and real-font screenshot; all 17 original MDE effects terminate
in the sequencer, render nonzero PCM without clipping over a three-second
sample, and a two-channel effect does not reset the BGM note timeline.
Linux full suite passes 139/139; ARM64 effects and entrance tests pass with matching results. Windows preview includes both features.
Full hardware/driver audio parity, effect preemption details and MyMini checks
remain unproven.


## PRESS KEY confirmation release (2026-10-30)

The title dismissal key previously carried into the original level-triggered
main menu and immediately selected Start. Native title now displays choices
immediately, consumes all held menu input until release, and accepts a fresh
press to navigate/confirm. Tests hold Enter, Z, controller A (Enter+Z), and
Escape for 30 frames, then select Continue with a fresh direction/confirmation.
Linux full suite 139/139 and ARM64 title regression pass. Autoplay explicitly
releases between title dismissal and Start; normal launch has no autoplay.

## MyMini first installation (2026-10-30)

Connected to user-provided 192.168.1.5: ArkOS4Clone 06272026, aarch64,
Cortex-A35, 1 GB, 640x480. Installed separate /roms2/ports/th01-native and
Touhou 1 Native.sh with local assets and ymfm license. ldd resolves all libraries.
Device SDL has no dummy driver. Initial SSH launch flickered with ArkOS because
EmulationStation remained rendering; retry suspends actual frontend PID 719 with
SIGSTOP and wraps game in an EXIT trap to SIGCONT it. User display/control
confirmation is pending. No timeout/autoplay in normal device launcher.

SDL --stage now actually selects direct-stage mode (previous parsing only set
starting_stage while still constructing TitleMenu). Rerunning genuine boss SDL
tests found Sariel loading a stale native placeholder Miko.SE. Corrected native
Sariel/Konngara entrance constants to zigoku.mde per th01/shiftjis/fns.hpp.
Regression rebuild ongoing; previous all-green SDL tests did not exercise those
direct-stage paths. MIDI music requested by user as next audio change; device
has libfluidsynth and GM soundfonts, but MIDI conversion/playback is not yet done.
User confirmed the second on-device trial is OK (display and D-pad/A/B).
The flicker was eliminated by suspending the concurrently rendering frontend
for the SSH-started test; this is not evidence of full performance/parity.

Pause controller fix: original pause treats simultaneous Z+Enter as an early
exit. SDL A previously set both. A now sends shot/Z only; Select/Back sends
Enter. Added Z-only resume regression; Linux and host ARM64 pause_test pass.
Built ARM64 executable deployed via .next then atomic rename so current play
is not interrupted. New process is required to use the fix. Full 139/139 suite
passed after direct-stage/SE corrections before this small mapping change.


## MDT-to-MIDI device checkpoint (2026-10-30)

User selected conversion from original MDT, not an external arrangement pack.
Added optional MIDI observer, SMF exporter, FluidSynth renderer sharing SDL
audio with original MDE effects, and default MIDI device launcher. All 19
MDT inputs checked without changing OPNA register/note/loop/tempo traces.
Synthetic tie/gate/pitch fixture passes on Linux and ARM64. Full suite 141/141
passes before final pan/SE-tempo edits, focused MIDI tests pass afterward.
MyMini offline render succeeds, installed SHA256 ba348da6f3ebf76e123ba2e8cce1cf3c4aee641a5405bf7699d004980fc138d0.
SSH launch failed with ALSA busy because suspended ES retained audio; normal
Ports launch succeeds, maps libfluidsynth.so.3 and owns PCM. User confirmed OK.
All 19 bounded audition SMFs exported to build/midi; package refreshed.
MIDI program mapping remains approximate; see MIDI.md for remaining musical
limitations. Overall port parity/performance/Continue history remain unfinished.

## Gameplay fixes and Standard/Challenge (2026-10-30)

Installed latest MyMini binary SHA256
143eb2ee1f6fa47342e056ef0165f500da9bcdc08313182ad0cb9633e1e9014e.
Normal launcher uses OPNA BGM again and TH01_AUDIO_MONO=1 for the single speaker.
User asked to test gameplay now and postpone remaining missing-music-layer work.

- Scene/boss reload stages 1/5/6/10/11/15/16/20 hold a fully black overlay for
  90 frames (1.5s). Boss BGM follows entrance SE/card, including final bosses:
  their first play callback presents the stage card before starting the track.
- Native bomb SE8 dispatches at cast frame 0; SE9 at explosion frame 50,
  retaining later original repeats. Framebuffer/scroll/palette trace unchanged.
- Pre-launch input ignores Escape/Enter/strike and consumes held action keys
  through release after launch. Continue consumes registration confirmation
  until neutral before accepting a fresh Yes/No choice.
- Options save on changes and are remembered across session/title recreation.
  Actual reported BGM Off issue was title_init unconditionally starting music.
  Native title wrappers now honor Off; Music Test is an explicit audition,
  stopped on return when Off. Ending no longer forcibly enables BGM.
- Start/Continue presents a native Standard/Challenge picker; held Start cannot
  confirm it, B cancels. Challenge has unlimited survival with original miss,
  bomb, score and speed behavior otherwise. It tracks raw misses minus score
  extend thresholds, including negative values. HUD is now compact M:<count>.
  Adjusted count <=10 selects good ending; higher count selects bad ending,
  independently of Continue total. Standard keeps original finite lives.
  Challenge score files are isolated in native-profile/challenge.
- OPNA voice load now writes stereo routing B4=C0 as MDRV98 does at 12C8..12D5.
  Effect pointers map to SSG C, then B, then A (driver 1EAB..1EC4), replacing
  the prior incorrect A-first override. Entire audio fidelity is not claimed.

Verification: latest Linux full suite 142/142 passed. New integrated tests run
on Linux and host-run ARM64 cover all reload stages/both routes, real 11 misses
without Game Over, real score-extend credit/no duplicate, actual ending asset
branch for both routes with contradictory Continue totals, options edited via
UI -> gameplay -> returned title -> fresh load, mono left/right preservation,
mode release and pre-launch input. Continue held-confirm regression and all
17 MDE effects pass; bomb effect PCM onset is within 50ms and dispatch keyframes
are checked. Windows preview was rebuilt/smoked for the initial batch; latest
short-HUD/Off/effect-channel follow-up is currently built/deployed for ARM64.

Audio follow-up remains open: user reports missing layers across all tracks.
Solo REIMU channels were rendered in build/audio-layers: FM5/6 are intentionally
silent in the first 12s (zero sequenced notes), but FM5 is audible in a 60s
render. Do not treat their initial rest as a decoder failure. Host-only original
MDRV98 reference capture is prepared by native/prepare_mdrv_reference.py using
copies of assets and the user's DOSBox-X with DX-CAPTURE /A. The corrected
20-second capture is build/driver-reference/captures/play_001.wav; play_000.wav
came from an initially malformed short stub and must not be used as reference.
Original-driver reference vs native waveform/layer comparison has not yet been
performed. No CPU emulator is part of the shipped game.
## Challenge crash and recorded-audio source (2026-10-30)

Latest device binary SHA256:
10e86373592e131736a1e34f8dbc3c06c658c430c4a790a2c1852cf61a170400.
User reported sixth Challenge miss crash. Device log is authoritative:
HUD icon background was not captured. Challenge was still trying to restore
finite-life icon snapshots after virtual stock refill. HudIconsBackend now has
hide_lives enabled only for Challenge, skipping both capture and restore of
finite-life icons; M: counter remains. Standard icon behavior unchanged.
Diagnostic fatal messages now include missing id/quarter. Tests cover zero
captured stock -> fictitious refill -> decrement, and Challenge exhausted
stock through regular-stage progression/boss entry and another miss. ARM64
hud_icons_test and integrated user_regressions_test pass. Full Linux regression
is running at this checkpoint (session 69497); verify terminal result before
claiming final full-suite success.

Also explicitly bgm_stop before TOTLE so faded boss voices do not remain under
result sounds. User's perceived volume jump still needs audible retest; do not
claim complete audio parity from this change alone.

User requested finding WAV instead of fixing synthesis. Found public album:
https://downloads.khinsider.com/game-soundtracks/album/touhou-reiiden-highly-responsive-to-prayers.-pc-9801
The site labels it PC-98 gamerip, 16 FLAC tracks (~196 MB). This labeling is not
independent verification against original gameplay. Title FLAC downloaded from
the album's explicit FLAC link, converted to build/recorded-bgm/REIMU.wav:
44100 Hz stereo PCM16, 205.4269387755102 seconds. Decoded PCM is bit-identical
to the downloaded FLAC (numpy array_equal). Source remains under
build/recorded-bgm/source/01-A-Sacred-Lot.flac. No audio-file BGM runtime exists
yet and no WAV has been installed on MyMini. Remaining tracks not downloaded.
Python preparation script native/prepare_recorded_bgm.py currently gets HTTP
403 fetching album HTML via urllib; do not silently retry forever. The initial
track page and direct FLAC fetched normally with PowerShell Invoke-WebRequest.
Conversion dependencies are local-only build/audio-tools (soundfile/numpy),
require escalated execution to read due sandbox-created-file permissions.

Pending: finish WAV acquisition/playback with correct track mapping/loop points,
retained SE and independent volume/fade, and retest TOTLE/Challenge on device.
The user has prioritized testing gameplay now. Do not restart or interrupt their
current run for audio work. Windows binary predates this crash patch.
Final verification: session 69497 completed successfully, Linux 142/142 passed
in 38.92 seconds. Updated device hash matches local ARM64 artifact. User audible
and post-sixth-miss confirmation remains pending.

## Persistent Challenge status badge (2026-10-30, builds in progress)

User reports VRAM/HUD damage intermittently hides MISS. Removed Challenge's
page-0/page-1 text painting. Framebuffer now has a compact StatusOverlay,
composited in screen coordinates after palette/scroll conversion and before
full-screen text/transition overlays. New native/challenge_hud.hpp renders a
small opaque dark M:<signed count> badge at (128,0), minimum 48x16, fixed bright
text with mint/orange edge for <=10/>10. It does not alter VRAM or capture slots.
StageSession refreshes it before every blocking presentation and after step;
value is cached to avoid per-frame glyph allocation. Only Challenge gameplay
and waiting-for-shot show it. Transition black masks intentionally cover it.

Added regression checks: original VRAM unchanged; badge crop byte-identical
after both pages are overwritten, palette made white and display scroll=392;
full black transition still covers badge. Real HUD screenshot remains produced
by user_regressions_test in build/{linux,arm64}/challenge-test-profile.
Current builds: Linux full session 79438, ARM64 stage_preview/user_regressions
session 92812; neither was terminal at this entry. MyMini current process 31529
is playing; do not interrupt it. Atomic update is for next launch only.
WAV/remaining audio follow-up from previous checkpoint remains pending.
HUD build verification: Linux full build 79438 and ARM64 build 92812 are
terminal success. ARM64 user_regressions_test 45403 passed including VRAM/
palette/scroll corruption and black-mask coverage. Rendered challenge-hud.bmp
visually inspected: compact high-contrast M:10 remains legible in top stock row.
Updated MyMini atomically for next launch, SHA256
dea4caf54a8cd938e6b0d90b9b746bd5fbb082343407b1c241331efb5cc24500
(local hash matches remote); package rebuilt. Existing run was not stopped.
Full Linux CTest was dispatched after build completion; verify its handle/result
before claiming the final suite passed. Current Windows binary is older.

Final badge verification: Linux CTest session 30706 completed, 142/142 passed
in 36.90 seconds. ARM64 integrated tests also passed. Updated artifact is on
MyMini for the next launch; real-device user feedback on badge remains pending.

## Player lives option applies only to Standard (2026-10-30)

StageSession normalizes credit_lives_extra to the default 2 in Challenge,
regardless of the saved Player option. This effective value initializes live
stock, saved stock and Continue/ending resident metadata, so life-based bonuses
and verdict resources cannot vary with a Standard-only setting. The original
menu configuration is preserved for the next Standard run; rank/BGM/bombs are
not changed. Challenge remains unlimited with signed miss/extend accounting.

Linux and ARM64 options_session_test pass for all five menu values in both
modes: Standard starts with value+2, Challenge always with 4 underlying lives;
Challenge bonus and initial miss credit remain identical across values.
Linux integrated user_regressions_test also passes. Only focused checks were
rerun for this change; do not claim a new full-suite run. Device updated via
atomic rename, current run unmodified, package refreshed. Windows is older.
### 2026-10-30 recorded WAV BGM
The user requested recorded BGM instead of further MDT synth repairs. Acquired all
16 tracks of the PC-98 gamerip album from KHInsider; direct source links retained
in recorded-bgm-sources.json and build/recorded-bgm/downloads.json. Album labeling
is provenance, not proof of bit-exact original hardware playback. No recordings
are redistributed via Git. Host preparation: download_recordings.py and
find_recorded_loops.py (soundfile/numpy in build/audio-tools).

RecordedMusic loads one stereo PCM16 44.1kHz recording at a time, retains only the
prefix through its loop end, interpolates to SDL's 48kHz output, and mixes at 50%
for SE headroom. Loop points come from spectral repetition and waveform alignment,
not original MDRV loop metadata; audible seam accuracy still needs listening on
hardware. No callback file IO or WAV decoding. Original MDT silently supplies
tempo/fade timing; native MDE retains SE, with BGM fade independent of SE gain.
ST6 aliases ST0, SHUGEN aliases SYUGEN; INIT remains silent. Mono output retained.
PortMaster defaults to TH01_WAV_DIR=$gamedir/music. TH01_AUDIO_BACKEND=opna or midi
remain explicit fallbacks. Only TH01's install is updated.

Validation: all 18 names/aliases audible, repeated loop playback, fade-to-silence,
stop and silent INIT pass on Linux and ARM64 under host QEMU with the target
sysroot libraries. SDL mono WAV title-to-stage smoke passes. All 142 existing
CTest checks pass. Await installation/hash verification and hardware listening.

Acquisition/deployment note: ArkOS ffmpeg and libsndfile decoding produced PCM
mismatches for a small subset compared with the host PCM (the host PCM's MD5
matches the FLAC STREAMINFO checksum for the inspected track). Do not silently
accept those decodes. Final installation uses byte-for-byte host WAV copies for
any mismatch and checks all 16 complete WAV SHA256 values with sha256sum before
activation. This is a preparation issue; runtime only reads PCM WAV via SDL.

Final WAV activation blocked (2026-10-30): all 16 recordings and the ARM64 build
are staged on MyMini, but the live binary and launcher were NOT replaced. The
15 other WAV file hashes match the host. SYUGEN host SHA256 is
601ebbb83cd21698f79b7c093af30faee81287563115999ce55109b76fa712f0;
staged device SHA256 is
d2293995a5ea3e67f2185ec76f947f4b2749641a88744b7ea9c71961f1f607e7.
All 1MiB chunk hashes of the inspected /tmp copy match except chunk 7; a base64
readback found one differing byte at file offset 7426032 (device 0, host 64).
Cause is unresolved; do not claim bit-exact device installation. Repeated raw,
gzip and libsndfile paths did not establish a fully matching final file.
Automatic approval review rejected changing the manifest to accept the mismatch
and activating it without explicit user authorization. Do not bypass that block.
Either resolve the mismatch with genuine matching validation, or obtain explicit
approval for this concrete discrepancy before activation. Existing launcher is
unchanged and still OPNA; new launcher/binary have .next names. Offline package
build/th01-mymini-test.tar.gz is ready with correct host WAV files.

### 2026-10-30 WAV activation completed after explicit approval
The user explicitly approved installing the WAV pack with the known one-byte
SYUGEN discrepancy. Live install now uses recorded WAV BGM and original MDE SE,
mono enabled, via /roms2/ports/Touhou 1 Native.sh. All 16 tracks were checked
against approved checksums. No further checksum exception was accepted.
LEGEND and ST3 required rechecking/re-copy preparation; file-cache read results
were intermittent. Final verification used fsync + POSIX_FADV_DONTNEED per file
and at most three checksum reads, requiring the actual expected digest for every
file (host digest for 15, the explicitly approved digest for SYUGEN). Cause of
intermittent reads is unresolved; no claim of hardware fault or universal fix.
Device recorded_music_test passed all recordings/aliases, repeated loop, fade,
stop and INIT. Binary/launcher activated atomically and hashes match host:
- binary a6720256f780fd6cc4bec4fa2a0daf800cc4af83191db323a066a963810824b1
- launcher 52e69ae0ce6873dc08bda1bf46c0a4697ba492d18deea25b269b704436fc68b0
Original host checksums, approved installed checksums and approval note remain
inside music/. Temporary FLACs, alternate WAVs, transferred archives and /tmp
music test files removed. Game was not launched; user should open from Ports to
hear the update. Offline package build/th01-mymini-test.tar.gz contains correct
host WAVs and the same binary/launcher. No original assets or TH08 install changed.

### 2026-10-30 user requested music revert
Restored OPNA/MDT BGM as the default on MyMini and in the local PortMaster
launcher. TH01_WAV_DIR is explicitly unset. Mono output, original SE and all
gameplay fixes are retained. The current binary retains dormant WAV support,
but no recording is selected. Device launcher bash -n passes and SHA256 matches:
2c1bf28b23296f5b40e81e536886a8ec2b2398ee42484f6eec71a0a9acc3dc3a.
No game was running at the time. WAV files remain inactive on the device for
reversible recovery; no original assets or profiles were removed. Offline
PortMaster archive rebuilt without the unused music folder and with OPNA default.

### 2026-10-30 original-driver audio investigation (PC only)
User requested trying register-log/solo/mono diagnosis after reverting the WAV
trial. See AUDIO_DIAGNOSTICS.md for reproducible methods, proof and limits.
Verified REIMU L/R are identical; mono energy ratio 1.0, so no phase cancellation
for this title excerpt. Captured original MDRV98 register commands on host using
a DOS COM trampoline harness in DOSBox-X, aligning to Timer B processing (not
all IRQs). Capture 62676 bytes, 477 ticks, no overflow; fixture uses 476 complete
ticks. Original driver files unchanged; no CPU emulation shipped to the device.

Fixed native MDT channel assignment: physical FM/SSG channels are packed by type
in table order, not low ID bits. REIMU's IDs 0x41/0x42 must occupy SSG A/B, not
B/C, leaving C for ordinary SE. MDE's C-then-B allocation remains unchanged.
Also matched original default master attenuation 8 for FM carriers and rhythm,
including rhythm's +1 step. First-note FM TL mismatch +8 and rhythm 61 vs54 now
match original commands. The ignored B0 register bits 6/7 are normalized when
comparing feedback/algorithm.

Added mdt_trace and original-driver oracle fixtures/test: all 130 FM note states
and 476 SSG/rhythm tick states match, on Linux and host-run ARM64. 143/143 Linux
CTest checks passed (40.19s). ARM64 all 17 MDE effects plus continuing BGM passed.
WAV before/after: build/audio-diagnostic/native-before.wav, native-after.wav.
No source changes were made while builds were running. No production ARM game
binary, device installation, launcher or offline PortMaster package was updated
in this investigation. MyMini still runs the previous OPNA build. Remaining:
full songs, later LFO/loop cases, FM5/6 beyond their rests and PCM gain calibration;
these fixes alone do not establish that all missing-layer complaints are solved.

### 2026-10-30 deployed original-driver audio fixes
User requested applying the confirmed parser fixes to the game. Built ARM64
stage_preview against the fixed mdt_player and atomically installed it as
/roms2/ports/th01-native/th01-native after upload SHA256 matched the PC.
Live and local binary SHA256:
41efbb3f53a74a3ee96066179612a5f43f22c787c3d0f8b4877ec19b40c533df.
Launcher remains OPNA by default, WAV unset and mono on. No game was running at
preflight. No ES interruption or remote game launch was performed. Validation
from the preceding investigation remains: 143 Linux tests, ARM64 driver oracle
(130 FM note states +476 SSG/rhythm ticks) and all 17 MDE effects passed. No further
source changes since those checks. Updated local PortMaster binary/package,
excluding unused music/ recordings. User should launch normally through Ports
for listening; full soundtrack timbre/gain correctness is not yet established.

### 2026-10-30 deployed second audio correction
User reported channels still sounded wrong/uneven after the first update.
Continued original-driver comparison and implemented FM gate dispatch on the
note-on tick, same-pitch tie coalescing before quantization, FM/SSG output scale
calibration (FM x4 before common gain), and SSG-only AC coupling. The original
stream packing fix and MDRV master attenuation remain. No WAV backend selected.
See AUDIO_DIAGNOSTICS.md second investigation for evidence and limitations.

Validated: 144/144 Linux CTest passed (41.33s); ARM64 oracle (REIMU476 ticks,
ST0338, LEGEND376), RMS balance, all17 SE and all19 MDT MIDI observer tests pass.
Manual state comparisons ST0/LEGEND report zero differences in captured windows.
Final30s renders of16 distinct songs have zero clipped samples, peak14108/32768.
This does not establish full-song/hardware-exact audio, and rhythm calibration
still has residual differences against the reference core.

Installed atomically on MyMini after exact upload SHA256 verification:
e844871db42d65eb2b9e390c736b7c98624578bd64818efe126b64cd06ca96a6.
No game was running at preflight; did not launch remotely or interrupt ES.
Previous binary retained on PC at
build/audio-balance/th01-native-before-second-fix (SHA41efbb3f...).
Local PortMaster archive updated without unused WAV recordings. User should
reopen through Ports to listen. Launcher remains OPNA, WAV unset, mono enabled.

### 2026-10-30 mode layout, immediate BGM toggle, TOTLE exit fade
Applied three user requests together and installed on MyMini:
- Mode selector now uses REIIDEN3 graphics + original OP_WIN color-key frame and
  REIIDEN2 palette, with centered fullwidth STANDARD/CHALLENGE at y296/316.
  Removed the oversized black rectangle, descriptions and extra title text.
  Visually inspected build/mode-standard.bmp; both labels fit the original frame.
- Options BGM Off->On immediately stops/loads/plays REIMU and synchronizes the
  current-mode state. Previously only Off was immediate; returning On before
  leaving Options could leave bgm_mode_cur unchanged and playback stopped.
  Regression covers Off/On/Off and no restart on idle frames.
- TOTLE acknowledgement now fades the palette smoothly to black over30 frames
  (nominal0.5s), then continues the existing next-scene flow automatically.
  Score/rolling/SE/input cadence retained; test frame total changes245->275,
  exactly30 monotonic fade frames ending black. Screenshot saved on first fade
  frame, not after it is fully black. No original DOS sources were modified.

Validation: 144/144 Linux CTest passed (43.17s). ARM64 title/options BGM regression
and TOTLE tests passed under host QEMU. Mode input regression remains passing.
Atomic device binary install verified against host SHA256:
987b4d37a54eee2ded413f66f61f798ffda2ff6c6e54afae61d31f1b52c1d2f6.
No game running at preflight; did not remote-launch or interrupt ES. Existing
music fixes, mono and OPNA launcher retained. Previous binary saved on PC at
build/th01-before-ui-update. Local PortMaster archive updated (unused music/
recordings excluded). User should reopen normally from Ports.

### 2026-10-30 cancel mode selection without title reset
Kept the existing TitleMenu alive while the mode picker is open. Cancel/B now
calls resume_menu(), restoring its action state and consuming the held cancel
until neutral. Framebuffer, selection and settings remain intact; no constructor,
intro animation, HIT KEY prompt or music restart. Start/Continue no longer fade
BGM on merely opening the picker; stage_preview commits the fade only after
mode confirmation. Regression checks no extra intro waits, identical menu pages,
held-B consumption, fresh navigation and Continue after cancel.
144/144 Linux tests passed (41.09s); ARM64 title/menu regression passed on host.
Installed atomically on MyMini after upload/live SHA256 matched:
d75c205103d51cceed81c4544d763df9c2d1e3bb4176af9c8133c364370683e3.
No game running at preflight, no remote launch. Previous binary retained on PC
at build/th01-before-mode-back-fix. Local PortMaster binary/archive updated,
unused music/ recordings excluded. Prior audio, Options BGM toggle, mode layout
and30-frame TOTLE fade fixes retained.

### 2026-10-30 physical Start context fix
User clarified the reported close/crash is pressing the device's physical Start
to confirm the highlighted Start/Option item, not cancelling the mode picker.
Found physical Start unconditionally mapped to Escape by both GPTOKEYB and SDL
controller input. The main menu treats Escape as Quit.

Mapped GPTOKEYB Start to dedicated F1 (verified f1 key name in device mapper).
SDL polling combines F1 and native controller START then routes by context:
Enter, clearing legacy Escape, in title/options/music/mode menus; Escape only
in gameplay/pause. Menu context switches before entering StageSession and before
recreating title after gameplay. Keyboard Escape retains original behavior.
A remains Z only, avoiding simultaneous Z+Enter pause-exit behavior.
Regression covers physical Start dismiss/held-input barrier, Start selection,
Options entry, and gameplay pause flags. ARM64 title and pause tests passed;
144/144 Linux tests passed (38.95s).

Installed binary and mapping after both uploaded hashes matched host:
- binary 99b186a63d7c74a65883b033a45ef4a5dd07868c15f256dfcd1017faaa43d6fe
- th01.gptk edc061e5930ae444d07ba6708e793c37ba157496bcee67076721a9be6dd143af
Did not kill or relaunch game/mapper. User must reopen via Ports for the new
mapping. Previous binary/mapping retained on PC at build/th01-before-physical-
start-fix and build/th01-gptk-before-physical-start-fix. Local PortMaster package
updated, recordings excluded. All earlier audio/UI/BGM/TOTLE/cancel fixes kept.

### 2026-10-30 single Start source on PortMaster
User reported one physical Start dismissing PRESS KEY and confirming again.
The SDL controller START path and GPTOKEYB F1 path could both represent that press
with asynchronous arrivals/releases. Added start_button_down source arbitration.
PortMaster launcher exports TH01_START_KEY_ONLY=1, selecting only mapped F1 for
Start in all contexts. Desktop/no-launcher still supports native controller START.
Existing title/mode held-input release barriers are retained. Context routing
remains Enter in menus, Escape in gameplay. Other input mappings unchanged.
Regression feeds delayed dual-source press/release/bounce events: the initial
press only dismisses PRESS KEY; only a second fresh mapped press selects Start.
Five related Linux tests passed (10.71s): title, pause, real_pause, user regressions
and SDL stage smoke. ARM64 title/controller regression passed on host. Launcher
normalized LF, bash -n passes; live gptokeyb mapping remains start=f1.
Installed binary and launcher after upload hashes matched host:
- binary c0a6eefe8406a96d8254b6ae96d864a44e62a3f63202ecb1ae5d403683ca4c6b
- launcher 1bc72bedfab837c1b19d2b549552dee9110714cef166fed802472cb524dfca83
No game running at preflight, no remote launch/kill. User must reopen through
Ports for the new launcher flag. Prior binary/launcher saved on PC at
build/th01-before-single-start and build/launcher-before-single-start.sh.
Local PortMaster package updated excluding unused music/ recordings.

### 2026-10-30 MyMini cleanup requested by user
Inspected only /roms2/ports/th01-native, its launcher and /tmp/th01*.
Removed the inactive recorded-music directory after verifying no running TH01
process had a nonempty TH01_WAV_DIR. Deleted38 WAV/loop/manifest/approval files,
568750795 logical bytes (about547MiB allocated). Port folder now about16MiB.
A game process was active, so left the entire assets directory unchanged rather
than remove the candidate unused DOS executables/boot files during play.
No unrelated port, source game, save, settings, profile, license or log.txt was
deleted. No /tmp/th01* diagnostic files remained. Runtime binary, mapping and
launcher hashes still match the current c0a6.../edc061.../1bc72... installation.
No game/mapper process was stopped or relaunched. PC recorded sources/diagnostic
artifacts retained; current PortMaster archive already excludes music/.

### 2026-10-30 Challenge HUD and dedicated bad-ending captions
Replaced presentation-space M badge with native two-page graphics rendering in
original Player/lives HUD field (x40..223, y0..15). FONT.ROM infinity at x48,
signed fullwidth miss number at x80; exceptionally long counts use the original
halfwidth font to stay inside the field. Redraw before waits/frame presentation
repairs the field after boss VRAM operations; palette fades/scroll now affect it
as they do original HUD. No presentation overlay is used for the miss counter.
Challenge ending result now carries an explicit mode flag into EndingSequence.
Bad branches keep original route-specific pictures/music/typing and verdict
handoff but say "Try 10 misses or less!" / "Challenge - Bad Ending". Standard
and good-ending captions remain original. Eligibility still adjusted misses<=10.
Tests check both VRAM pages, clearing shorter/signed numbers, no overlay, palette
fade, all eight route/mode/ending combinations and absence of No-Continue text
in Challenge bad ending. Initial implementation passed144/144 Linux tests and
ARM64 user regression + ending sequence under host QEMU. Final shortened text
passed both affected Linux tests; final ARM64 build/check follows before deploy.
Final shortened captions passed ARM64 ending_sequence_test for all8 branches.
Installed binary on MyMini after matching upload SHA256:
f709508e0c38571c2897860ce02533b9478bfc7a89ab24a7119795f7a9793862.
Prior binary retained locally in build/th01-before-challenge-hud-ending.
PortMaster package/staging updated; no launcher, mapping, music or save changed.
No game was running at pre-deployment check; no game was remotely launched.

### 2026-10-30 Correct Miss HUD background and bomb alignment
Removed solid color clearing entirely. ChallengeHud snapshots/restores the
original lives-field pixels separately for both graphics pages, like score HUD
background capture. Capture hooks into native_hud_lives_init; draws remain
transparent FONT.ROM glyphs at x128 (infinity), x152 (signed fullwidth count),
y0, aligned with bombs at x128,y16. Original Player label remains visible.
Long values use original halfwidth glyphs, compressed only for >9 characters to
keep every signed int32 digit within x128..223 without touching score labels.
Capture invalidates for new scene/background/boss loads; no counter drawing
occurs before HUD initialization. No black fill/presentation overlay is used.
Regression checks patterned background survives outside glyphs, shorter signed
values fully restore old digits, both pages match and palette fade affects HUD.
Five affected Linux tests passed. Inspected actual session challenge-hud.bmp:
Player infinity/count above bombs, background visible behind all glyphs.
ARM64 user regression passed under host QEMU. Updated MyMini via verified
.next upload then atomic rename, SHA256:
0a6b4ba8ece5ef8da22235d90ffa084ef192768854784adba26a3a535cceacc4.
Prior f709... binary retained locally build/th01-before-transparent-miss-hud.
PortMaster package updated. No mapping, launcher, audio, saves or settings
changed, no remote game launch. No game process at pre-deployment check.

### 2026-10-30 Bomb interruption leaves HUD inverted / screen scrolled
Reproduced by clearing a regular stage at bomb frame88: previous code failed
assertion that display_scroll_y==0 at bonus screen. Bomb's normal frame140
restores the clean backing page, scroll and palette; stage clear instead called
resume_after_miss(), forcing bomb_frame=200 and skipping that cleanup entirely.
Stage-clear handoff now restores visible graphics from original page1, resets
scroll and clears bomb damage before bonus presentation. Existing stage-palette
restore remains. No HUD overlay/reinitialization or stock/score changes added.
Regression covers Standard and Challenge, interruptions at42/64/88/136 and a
normally completed bomb at140, checks HUD labels/border match clean backing
page and remain intact into stage2 after fresh bonus-confirm input.
Validation: five Linux tests passed (user regressions, original synthetic/real
bomb traces, regular session and route handoff). ARM64 user regressions passed
under host QEMU, including all10 bomb-clear mode/timing cases.
Installed MyMini binary SHA256 verified before/after activation:
92052f6b189f1b17a0a674c7014758f5e7abb99699ba5ebf70ce61fa708f457d.
Backup previous binary build/th01-before-bomb-clear-repair; package updated.
Game PID19076 was active at read-only check; did not stop or relaunch it.
New binary takes effect on next launch. Save/settings/mapping/music unchanged.

### 2026-10-30 Original ZUN Soft startup and verified Sariel death fade
Added native ZunSoftIntro using original TOUHOU.DAT packed BFNT logo patterns,
B,R,G palette, master.lib SinTable7/SinTable8 and th01/zunsoft.cpp animation:
four clipped discs,50 rotating stars, pattern/wave keyframes,231 two-VSync
frames, fade-in/out and skip. Plays only on initial ordinary startup, before
title/BGM, not when returning to menu or launching a direct-stage diagnostic.
Held skip input is consumed until release before title input can act.

Sariel native_sariel_black_out_step previously decremented zero RGB4 channels
into negative values and masked them back to15, producing bright color flashes.
Now clamps each channel to0. Final-death observer checks every presented fade
sample is nonincreasing in all16x3 channels and ends fully black, on both full
boss passes;50,000 clear bonus, attack state trace and ending handoff remain.
Observer explicitly starts at final defeat, excluding the next entrance's fade-in.

Validation before installation (explicit user requirement):21 focused Linux tests
passed, including all Sariel subsystems/controller/session, title and intro;
ARM64 intro, full Sariel and ending-session tests passed under host QEMU.
Inspected intro keyframes70/100/165 and Sariel death202/220 screenshots.
Ran full sariel_test directly on idle MyMini CPU before replacing the game:
all phases/two forms, monotonic death fade and clear/cleanup checks passed;
state trace d75a078ee951e49b matches host/ARM64. Final-black BMP SHA256 matches:
50a3c7edcfc801f720d50dfde94a8a4d5b2bff67646b5e2510374ddbce499175.
SDL dummy actual startup->intro->title smoke passed (679 presentations).

Installed after those checks, binary SHA256 verified before/after atomic rename:
bafaf29ae9ff6a477dbd5458e648a51adfbef52d4f253b6243b77753af7eebbf.
Backup previous9205... binary: build/th01-before-zunsoft-sariel-fade.
PortMaster package updated. TOUHOU.DAT already present/verified on MyMini;
no game data, music, input mapping, launcher, settings or saves changed.
TH01/TH04 were idle at pretest check; no interactive game was remotely started.

## TH01-only MyMini headphone routing (2026-10-31)

Added native/portmaster/audio-route.sh, sourced by TH01 launcher only.
Verified physical jack polarity with user: unplugged Headphone Jack=on,
plugged=off on rockchip RK817 card0. Launcher explicitly enables inversion.
Helper polls every0.5s: inserted uses Playback Path=HP(2), unplugged SPK(1),
unknown signal leaves route unchanged. Mixer volume and game PCM unchanged.
Background helper stops on launcher exit/signals; restores original path only
if the current path still belongs to helper. No persistent service/settings.

Mock tests pass insert/unplug/unknown/restore and bit-identical staging/remote
SHA256 verified. On actual idle device plugged=off, helper selected HP2 and
restored original2 on exit. Audible speaker isolation and physical hot-unplug
while playing are pending user confirmation. No interactive game launched.
Installed at192.168.1.7, helper SHA256
41e773192b7a7bdadc213b02166503763c40772686a48a9eea92bc28383a5ff3;
launcher f822b6720d227e8e0314d0956321805e6e3913adcc9f0334ff3bfaa0f0c40ed1.
PC package updated; previous device launcher build/th01-headphone-before/.
TH01 executable, assets, saves, TH04/TH08 and system services untouched.

## Boss Prac, OTHER.DAT and renamed package (2026-10-31)

Installed directory /roms2/ports/th01port; launcher Touhou 1 - HRtP.sh.
Folder rename preserved all profile checksums, executable remains th01-native
for controller mapping. Headphone helper stays scoped to renamed launcher.
Route labels after Shingyoku are now MAKAI / HELL, fullwidth game font.

BOSS PRAC inserted between Continue and Option. Mode -> rank -> four-row
scrolling seven-boss list, locked names shown, B returns one level retaining
selection. Existing press/release/Start handling preserved. BossId maps all
seven native bosses to original stage and route. Practice reuses StageSession
with SessionPurpose::boss_practice; normal/default API behavior unchanged.
Both practice modes start BOMBS_MAX5. Standard starts LIVES_MAX6 regardless
of Option; Challenge keeps infinite-life/miss rules and ignores life Option.
Fresh-session Retry resets stocks, score, misses, actors and seed. Start pause
has Resume/Retry/Boss List; B/Start backs out of pause to fight. Clear/death
returns Retry(default)/Boss List, without score registration, Continue, TOTLE,
next stage or ending. Shingyoku keeps original defeat animation while skipping
route choice only in practice. Sariel must clear both forms.

Normal boss defeat emits one event per boss stage including runs with Continue.
Host persists separate mode x rank x boss unlocks in native-profile/OTHER.DAT.
No unlock from practice or developer --stage starts; no retroactive inference
from old scores. Practice skips scoredat_load to avoid creating original score
files. Existing normal/challenge score directories and reiiden.cfg retained.
OTHER.DAT v1 is24 bytes: TH01OTHR magic, little-endian version1, eight seven-bit
masks, FNV32 checksum. Atomic sibling-temp rename. Missing file starts locked;
invalid/future file retained, logged/error shown in practice menu, not replaced.
Write failure is reported without crash. File is first created on normal clear.

Validation:56 unlock combinations, reopen/isolation, corrupt/unwritable cases,
locked selection, held input/back, max stocks for all7 bosses in both modes,
Challenge repeated misses, Standard game over, pause Resume/B/Retry, no score
files created, and full phase-aware clears for all7 practice bosses passed.
Normal Mima-after-Continue emits exactly one correctly grouped unlock event.
11 focused normal session/options/user regressions passed; final menu/practice
and both-route tests passed, route screenshot inspected with labels in bounds.
Actual MyMini CPU ran the complete Boss Prac test (including both Sariel forms)
before replacement, all assertions passed. Private test profile was in /tmp,
removed afterward; production unlocks were not seeded with test data.
Device/staging binary SHA256 f8469b2b9356c9c9424d7c3d8b950628a72f81873b25010a6f7aae5bc86133e0.
Previous binary build/th01-before-boss-practice/th01-native (bafaf29a...).
Device SHA verified; original profile checksums identical before/after install.
No interactive SDL game remotely launched; physical UI playtest remains pending.
Evidence logs build/practice-*.log and build/boss-practice-*.log.

Ports gamelist TH01 path/name updated to Touhou 1 - HRtP.sh / Touhou 1 - HRtP, preserving playcount and lastplayed. Idle EmulationStation restarted via its existing es-restart wrapper; frontend active afterward. Previous XML backed up on PC alongside previous binary.

## Smaller Boss Prac UI and menu Back music continuity (2026-10-31)

Mode box192x104, difficulty192x144, boss list256x176 with20px row spacing
and short LOCK indicators; result/pause boxes240px wide. Rendered mode/rank/list
using real assets and inspected mode/list BMPs; all labels fit.
Removed unconditional reimu.mdt load/play when leaving practice via Back.
Menu-only navigation now emits no transport commands. Title track is restarted
once when returning from an actual boss attempt to the boss list; subsequent B
back to rank/mode/title leaves that same playback running. BGM Off honored.
ARM build passed, idle device atomic install + checksum and PC staging updated:
e053cf910fa99f4fcdcc4e9adb0f109d06980164ed6690967336fb038a7f732d.
Previous binary build/th01-before-small-practice/th01-native. No gameplay,
OTHER.DAT, original save, launcher or route changes. Audible UI test pending.

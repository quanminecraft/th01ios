param([string]$CompilerRoot='C:\msys64\ucrt64')
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$output=Join-Path $repo 'build\windows'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$nativeSources=@('stage_session','random','shot_backend','player_backend','input_backend','card_backend','obstacle_backend','math_backend','pellet_backend','item_backend','item_motion','extend_backend','stage_tick','regular_frame','timer_backend','text_animation','invincibility_backend','laser_backend','hud_icons','hud_score','hud_layout','bomb_backend','miss_backend','pause_backend')
$originalSources=@('main/player/orb','main/player/shot','main/player/player','hardware/input','main/stage/card','main/stage/stageobj','math/vector','main/bullet/pellet','main/bullet/pellet_s','main/stage/item','main/extend','main/stage/timer','main/player/inv_spr','main/bullet/laser_s','main/player/bomb','hardware/grp_text')
$arguments=@('-O2','-std=c++17','-fsigned-char','-DGAME=1','-DTH01_NATIVE_ORB_PHYSICS','-DTH01_NATIVE_PLAYER_ACTIONS',"-I$repo","-I$CompilerRoot\include\SDL2",(Join-Path $PSScriptRoot 'stage_preview.cpp'))
$nativeSources+=@('boss_progress','bonus_screen','title_menu','mdt_player','music_synth','sdl_music','sdl_midi','third_party/ymfm/ymfm_opn','third_party/ymfm/ymfm_ssg','third_party/ymfm/ymfm_adpcm')
$nativeSources+=@('boss_entity_backend','boss_hit_backend','hp_backend','singyoku_backend')
$nativeSources+='defeat_backend'
$nativeSources+='gameover_backend'
$nativeSources+=@('ending_picture','ending_palette','ending_verdict','ending_playback')
$nativeSources+=@('continue_backend','scoredat_backend','regist_backend')
$nativeSources+=@('sariel_gameplay','sariel_backend','elis_gameplay','elis_backend','yuugenmagan_gameplay','yuugenmagan_backend','totle_transition','totle_backend','mima_gameplay','mima_backend','konngara_gameplay','konngara_backend','konngara_entrance','kikuri_gameplay','kikuri_backend','kikuri_ripple','missile_backend','particle_backend','shape_backend','line_backend')
$originalSources+=@('main/boss/b10m','main/bullet/line','main/boss/b10j','main/boss/b15m','main/boss/b15j','main/boss/b20j','main/boss/b20m','main/bullet/missile','main/particle')
$originalSources+='main/boss/defeat'
$originalSources+='main/player/gameover'
$originalSources+=@('formats/scoredat','hiscore/regist')
$originalSources+=@('main/boss/boss','main/boss/b05','main/hud/hp')
$arguments+=@($nativeSources | ForEach-Object {Join-Path $PSScriptRoot "$_.cpp"})
$arguments+=@($originalSources | ForEach-Object {Join-Path $repo "th01/$_.cpp"})
$arguments+=@("-L$CompilerRoot\lib",'-lSDL2','-Wl,--stack,16777216','-o',(Join-Path $output 'stage_preview.exe'))
& (Join-Path $CompilerRoot 'bin\g++.exe') @arguments
if($LASTEXITCODE -ne 0) {throw 'Native stage preview build failed'}
foreach($dll in @('SDL2.dll','libstdc++-6.dll','libgcc_s_seh-1.dll','libwinpthread-1.dll')) {Copy-Item -LiteralPath (Join-Path $CompilerRoot "bin\$dll") -Destination $output -Force}
@'
@echo off
cd /d "%~dp0"
stage_preview.exe "..\assets\th01-ja"
'@ | Set-Content -LiteralPath (Join-Path $output 'TH01 Native Stage 1.cmd') -Encoding ascii
Write-Output "Built native both-route stages 1-20 prototype in $output"
@'
@echo off
cd /d "%~dp0"
stage_preview.exe "..\assets\th01-ja" --stage 5
'@ | Set-Content -LiteralPath (Join-Path $output 'TH01 Native Singyoku.cmd') -Encoding ascii

foreach($route in @('makai','jigoku')) {
    $launcher="@echo off`r`ncd /d `"%~dp0`"`r`nstage_preview.exe `"..\assets\th01-ja`" --stage 6 --route $route`r`n"
    [IO.File]::WriteAllText((Join-Path $output "TH01 Native Stage 6 $route.cmd"),$launcher,[Text.Encoding]::ASCII)
}
$introLauncher="@echo off`r`ncd /d `"%~dp0`"`r`nstage_preview.exe `"..\assets\th01-ja`" --stage 10 --route jigoku`r`n"
[IO.File]::WriteAllText((Join-Path $output 'TH01 Native Mima.cmd'),$introLauncher,[Text.Encoding]::ASCII)
$kikuriLauncher="@echo off`r`ncd /d `"%~dp0`"`r`nstage_preview.exe `"..\assets\th01-ja`" --stage 15 --route jigoku`r`n"
[IO.File]::WriteAllText((Join-Path $output 'TH01 Native Kikuri.cmd'),$kikuriLauncher,[Text.Encoding]::ASCII)
$konngaraLauncher="@echo off`r`ncd /d `"%~dp0`"`r`nstage_preview.exe `"..\assets\th01-ja`" --stage 20 --route jigoku`r`n"
[IO.File]::WriteAllText((Join-Path $output 'TH01 Native Konngara.cmd'),$konngaraLauncher,[Text.Encoding]::ASCII)
$yuugenmaganLauncher="@echo off`r`ncd /d `"%~dp0`"`r`nstage_preview.exe `"..\assets\th01-ja`" --stage 10 --route makai`r`n"
[IO.File]::WriteAllText((Join-Path $output 'TH01 Native YuugenMagan.cmd'),$yuugenmaganLauncher,[Text.Encoding]::ASCII)
$elisLauncher="@echo off`r`ncd /d `"%~dp0`"`r`nstage_preview.exe `"..\assets\th01-ja`" --stage 15 --route makai`r`n"
[IO.File]::WriteAllText((Join-Path $output 'TH01 Native Elis.cmd'),$elisLauncher,[Text.Encoding]::ASCII)
$sarielLauncher="@echo off`r`ncd /d `"%~dp0`"`r`nstage_preview.exe `"..\assets\th01-ja`" --stage 20 --route makai`r`n"
[IO.File]::WriteAllText((Join-Path $output 'TH01 Native Sariel.cmd'),$sarielLauncher,[Text.Encoding]::ASCII)

$fullLauncher="@echo off`r`ncd /d `"%~dp0`"`r`nstage_preview.exe `"..\assets\th01-ja`"`r`n"
[IO.File]::WriteAllText((Join-Path $output 'TH01 Native.cmd'),$fullLauncher,[Text.Encoding]::ASCII)

Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'third_party\ymfm\LICENSE') -Destination (Join-Path $output 'ymfm-LICENSE.txt') -Force

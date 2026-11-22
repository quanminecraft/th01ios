param([string]$CompilerRoot = 'C:\msys64\ucrt64')
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = Join-Path $repoRoot 'build\windows'
$compiler = Join-Path $CompilerRoot 'bin\g++.exe'
if (!(Test-Path -LiteralPath $compiler)) { throw 'MSYS2 UCRT64 g++ not found' }
New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null
$common = @('-O2','-std=c++17','-fsigned-char',"-I$repoRoot")
foreach ($target in @('archive_test','render_test','grp_test','stage_test','asset_check','font_rom_test','orb_test','shot_test','shot_real_test','input_test','input_actions_test','card_test','fullwidth_text_test','hud_score_test','hud_layout_test','item_spawn_test','card_animation_test','obstacle_test','actor_dispatch_test','player_physics_test','player_actions_test','invincibility_test','laser_test','pellet_test','bomb_test','pause_test','miss_test','extend_test','hud_icons_test','timer_test','text_animation_test','math_test','sprite_preview')) {
	$buildArgs = $common + @((Join-Path $PSScriptRoot "$target.cpp"),'-o',(Join-Path $buildRoot "$target.exe"))
	if ($target -in @('orb_test','shot_test','shot_real_test','input_test','input_actions_test','card_test','fullwidth_text_test','hud_score_test','hud_layout_test','item_spawn_test','card_animation_test','obstacle_test','actor_dispatch_test','player_physics_test','player_actions_test','invincibility_test','laser_test','pellet_test','bomb_test','pause_test','miss_test','extend_test','hud_icons_test','timer_test','text_animation_test')) {
		$buildArgs += @('-DTH01_NATIVE_ORB_PHYSICS',(Join-Path $PSScriptRoot 'random.cpp'),(Join-Path $repoRoot 'th01\main\player\orb.cpp'))
	}
	if ($target -in @('shot_test','shot_real_test','input_actions_test','card_test','fullwidth_text_test','hud_score_test','hud_layout_test','item_spawn_test','card_animation_test','obstacle_test','actor_dispatch_test','player_physics_test','player_actions_test','invincibility_test','laser_test','pellet_test','bomb_test','pause_test','miss_test','extend_test','hud_icons_test','timer_test','text_animation_test')) {
		$buildArgs += @('-DGAME=1',(Join-Path $PSScriptRoot 'shot_backend.cpp'),(Join-Path $repoRoot 'th01\main\player\shot.cpp'))
	}
	if ($target -in @('input_actions_test','card_test','fullwidth_text_test','hud_score_test','hud_layout_test','item_spawn_test','card_animation_test','obstacle_test','actor_dispatch_test','player_physics_test','player_actions_test','invincibility_test','laser_test','pellet_test','bomb_test','pause_test','miss_test','extend_test','hud_icons_test','timer_test','text_animation_test')) {
		$buildArgs += @('-DTH01_NATIVE_PLAYER_ACTIONS',(Join-Path $PSScriptRoot 'player_backend.cpp'),(Join-Path $repoRoot 'th01\main\player\player.cpp'))
	}
	if ($target -in @('input_test','input_actions_test','card_test','fullwidth_text_test','hud_score_test','hud_layout_test','item_spawn_test','card_animation_test','obstacle_test','actor_dispatch_test','player_physics_test','player_actions_test','invincibility_test','laser_test','pellet_test','bomb_test','pause_test','miss_test','extend_test','hud_icons_test','timer_test','text_animation_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'input_backend.cpp'),(Join-Path $repoRoot 'th01\hardware\input.cpp'))
	}
	if ($target -in @('card_test','pause_test','miss_test','fullwidth_text_test','hud_score_test','hud_layout_test','item_spawn_test','card_animation_test','obstacle_test','actor_dispatch_test','extend_test','hud_icons_test','timer_test','text_animation_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'card_backend.cpp'),(Join-Path $repoRoot 'th01\main\stage\card.cpp'))
	}
	if ($target -in @('obstacle_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'obstacle_backend.cpp'),(Join-Path $repoRoot 'th01\main\stage\stageobj.cpp'))
	}
	if ($target -in @('math_test','hud_layout_test','laser_test','pellet_test','bomb_test','pause_test','miss_test','extend_test','hud_icons_test','timer_test','text_animation_test','actor_dispatch_test','item_spawn_test')) {
		$buildArgs += @('-DTH01_NATIVE_ORB_PHYSICS',(Join-Path $PSScriptRoot 'math_backend.cpp'),(Join-Path $repoRoot 'th01\math\vector.cpp'))
	}
	if ($target -in @('pellet_test','bomb_test','pause_test','miss_test','hud_layout_test','extend_test','hud_icons_test','timer_test','text_animation_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'pellet_backend.cpp'),(Join-Path $repoRoot 'th01\main\bullet\pellet.cpp'),(Join-Path $repoRoot 'th01\main\bullet\pellet_s.cpp'))
	}
	if ($target -in @('item_spawn_test','hud_icons_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'item_backend.cpp'),(Join-Path $PSScriptRoot 'item_motion.cpp'),(Join-Path $repoRoot 'th01\main\stage\item.cpp'))
	}
	if ($target -in @('extend_test','miss_test','hud_icons_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'extend_backend.cpp'),(Join-Path $repoRoot 'th01\main\extend.cpp'))
	}
	if ($target -eq 'actor_dispatch_test') {
		$buildArgs += @((Join-Path $PSScriptRoot 'stage_tick.cpp'),(Join-Path $PSScriptRoot 'regular_frame.cpp'))
	}
	if ($target -in @('timer_test','miss_test','text_animation_test','hud_layout_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'timer_backend.cpp'),(Join-Path $PSScriptRoot 'text_animation.cpp'),(Join-Path $repoRoot 'th01\main\stage\timer.cpp'))
	}
	if ($target -in @('invincibility_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'invincibility_backend.cpp'),(Join-Path $repoRoot 'th01\main\player\inv_spr.cpp'))
	}
	if ($target -in @('laser_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'laser_backend.cpp'),(Join-Path $repoRoot 'th01\main\bullet\laser_s.cpp'))
	}
	if ($target -in @('hud_icons_test','miss_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'hud_icons.cpp'))
	}
	if ($target -in @('fullwidth_text_test','pause_test','hud_score_test','hud_layout_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $repoRoot 'th01\hardware\grp_text.cpp'))
	}
	if ($target -in @('hud_score_test','hud_layout_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'hud_icons.cpp'),(Join-Path $PSScriptRoot 'hud_score.cpp'))
	}
	if ($target -eq 'hud_layout_test') {
		$buildArgs += @((Join-Path $PSScriptRoot 'hud_layout.cpp'))
	}
	if ($target -in @('bomb_test','pause_test','miss_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'bomb_backend.cpp'),(Join-Path $repoRoot 'th01\main\player\bomb.cpp'))
	}
	if ($target -eq 'miss_test') {
		$buildArgs += @((Join-Path $PSScriptRoot 'miss_backend.cpp'))
	}
	if ($target -in @('pause_test','actor_dispatch_test')) {
		$buildArgs += @((Join-Path $PSScriptRoot 'pause_backend.cpp'))
	}
	if ($target -eq 'sprite_preview') {
		$buildArgs += @("-I$CompilerRoot\include\SDL2","-L$CompilerRoot\lib",'-lSDL2')
	}
	& $compiler @buildArgs
	if ($LASTEXITCODE -ne 0) { throw "Build failed: $target" }
}
foreach ($library in @('SDL2.dll','libstdc++-6.dll','libgcc_s_seh-1.dll','libwinpthread-1.dll')) {
	Copy-Item -LiteralPath (Join-Path $CompilerRoot "bin\$library") -Destination $buildRoot
}
foreach ($target in @('math_test','archive_test','render_test','grp_test','stage_test','font_rom_test','orb_test','shot_test','input_test','input_actions_test','card_test','fullwidth_text_test','hud_score_test','hud_layout_test','item_spawn_test','card_animation_test','obstacle_test','actor_dispatch_test','player_physics_test','player_actions_test','invincibility_test','laser_test','pellet_test','bomb_test','pause_test','miss_test','extend_test','hud_icons_test','timer_test','text_animation_test')) {
	& (Join-Path $buildRoot "$target.exe")
	if ($LASTEXITCODE -ne 0) { throw "Test failed: $target" }
}
$assetRoot = Join-Path $repoRoot 'build\assets\th01-ja'
if (Test-Path -LiteralPath $assetRoot) {
	& (Join-Path $buildRoot 'bomb_test.exe') $assetRoot
	if ($LASTEXITCODE -ne 0) { throw 'Real-asset bomb integration failed' }
	& (Join-Path $buildRoot 'shot_real_test.exe') $assetRoot
	if ($LASTEXITCODE -ne 0) { throw 'Real-asset shot integration failed' }
	& (Join-Path $buildRoot 'player_actions_test.exe') $assetRoot
	if ($LASTEXITCODE -ne 0) { throw 'Real-asset player action integration failed' }
	& (Join-Path $buildRoot 'obstacle_test.exe') $assetRoot
	if ($LASTEXITCODE -ne 0) { throw 'Real-asset obstacle integration failed' }
	if (Test-Path -LiteralPath (Join-Path $assetRoot 'FONT.ROM')) {
		& (Join-Path $buildRoot 'card_animation_test.exe') $assetRoot
		if ($LASTEXITCODE -ne 0) { throw 'Real-asset card animation integration failed' }
		& (Join-Path $buildRoot 'text_animation_test.exe') $assetRoot
		if ($LASTEXITCODE -ne 0) { throw 'Real-asset text animation integration failed' }
		& (Join-Path $buildRoot 'hud_layout_test.exe') $assetRoot
		if ($LASTEXITCODE -ne 0) { throw 'Real-asset HUD layout integration failed' }
		& (Join-Path $buildRoot 'miss_test.exe') $assetRoot
		if ($LASTEXITCODE -ne 0) { throw 'Real-asset miss integration failed' }
		& (Join-Path $buildRoot 'pause_test.exe') $assetRoot
		if ($LASTEXITCODE -ne 0) { throw 'Real-asset pause integration failed' }
	}
}
@'
@echo off
cd /d "%~dp0"
sprite_preview.exe "..\assets\th01-ja" --asset STAGE0.DAT
'@ | Set-Content -LiteralPath (Join-Path $buildRoot 'TH01 Native Asset Preview.cmd') -Encoding ascii
Write-Output "Built native asset preview in $buildRoot (not a playable game yet)."

#include "stage_session.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "regular_frame.hpp"
#include "stage_tick.hpp"
#include "hud_layout.hpp"
#include "stage_pellet_binding.hpp"
#include "item_runtime.hpp"
#include "invincibility_backend.hpp"
#include "laser_backend.hpp"
#include "miss_backend.hpp"
#include "pause_backend.hpp"
#include "text_animation.hpp"
#include "obstacle_backend.hpp"
#include "bonus_screen.hpp"
#include "singyoku_backend.hpp"
#include "mima_gameplay.hpp"
#include "defeat_backend.hpp"
#include "gameover_backend.hpp"
#include "continue_backend.hpp"
#include "regist_backend.hpp"
#include "totle_backend.hpp"
#include "ending_playback.hpp"
#include "challenge_hud.hpp"
#include "practice_menu.hpp"
#include <cctype>
#include <chrono>
#include <thread>
namespace th01_native {
namespace {
bool session_active=false;
struct SessionLease {
	SessionLease() {if(session_active) throw std::logic_error("Original global gameplay supports one active session");session_active=true;}
	~SessionLease() {session_active=false;}
};
SessionHost checked(SessionHost host) {if(!host.poll || !host.present_wait) throw std::invalid_argument("Session host input/presentation is incomplete");return host;}
struct SessionAssets {
	std::filesystem::path directory;
	SpriteSet stg,miko,action48,action32,kuji1,kuji2,background,boss_background,gameover_background,totle_screen,numbers;
	std::vector<Entry> archive;
	FontRom font;
	HudMask mask;
	Scene scene;
	StageLayout layout;
	explicit SessionAssets(const std::filesystem::path& root):directory(root),archive(load_archive(root)),font(read_file(root/"FONT.ROM")),mask(read_hud_mask(read_file(root/"MASK.GRF"))) {
		stg=read_sprites(asset(archive,"STG.PTN"));miko=read_sprites(asset(archive,"MIKO.PTN"));
		action48=read_sprites(asset(archive,"MIKO_AC.BOS"));action32=read_sprites(asset(archive,"MIKO_AC2.BOS"));
		kuji1=read_sprites(asset(archive,"KUZI1.GRC"));kuji2=read_sprites(asset(archive,"KUZI2.GRC"));
		background=read_grp(read_file(root/"ST0.GRP"));scene=read_scene(asset(archive,"STAGE0.DAT"));layout=scene[0];
		boss_background=read_grp(read_file(root/"BOSS1.GRP"));
		gameover_background=read_grp(read_file(root/"GAME_O.GRP"));
		totle_screen=read_grp(read_file(root/"CLEAR3.GRP"));numbers=read_sprites(asset(archive,"NUMB.PTN"));
	}
	void load_scene(unsigned index) {
		if(index>6) throw std::out_of_range("Scene outside original route data");
		background=read_grp(read_file(directory/("ST"+std::to_string(index)+".GRP")));
		scene=read_scene(asset(archive,"STAGE"+std::to_string(index)+".DAT"));
	}
};
Framebuffer initial_framebuffer(const SpriteSet& background) {
	Framebuffer fb;fb.palette=background.palette;
	for(unsigned page=0;page<2;++page) fb.blit(page,0,0,background.images.at(0),false);
	return fb;
}
CardEvents rendered_events(CardRenderer& renderer) {CardEvents events;events.renderer=&renderer;return events;}
}
struct StageSession::Impl {
	SessionLease lease;
	SessionHost host;
 SessionPurpose purpose;
 int unlocked_stage=-1;
 SessionState practice_pause_result=SessionState::playing;
	SessionAssets assets;
	Framebuffer fb;
	RegularFrameState frame;
	std::int8_t saved_lives=4;
	std::int8_t saved_bombs=3;
	std::uint16_t continues=0;
	std::array<std::uint16_t,4> scene_continues{};
	unsigned active_scene=0;
	std::int16_t bgm_counter=0;
	unsigned stage_id=0;
	std::array<std::uint32_t,4> scene_bonuses{};
	bool boss_defeat_pending=false;
	unsigned music_fade_requests=0;
 std::vector<GameAudioCommand> audio_commands;
 std::vector<unsigned> extra_sound_commands;
 bool bgm_enabled=true;std::string current_music;
 ChallengeRules challenge;
 ChallengeHud challenge_display;
 bool stage_intro=false,defer_boss_play=false,boss_play_pending=false,stage_card_done=false;
 bool blocked_escape=false,blocked_enter=false,blocked_strike=false;
	route_t route=ROUTE_MAKAI;
	SessionState state=SessionState::waiting_for_shot;
	ShotBackend shots;
	PlayerBackend player;
	CardRenderer renderer;
	CardEvents events;
	HudIconsBackend hud;
	ExtendBackend extend;
	TimerBackend timer;
	HudLayoutBackend hud_layout;
	BombBackend bomb;
	MissBackend miss;
	PauseBackend pause;
	BossEntityBackend boss_entities;
	SingyokuBackend singyoku;
	BossHitBackend boss_hit;
	DefeatBackend defeat;
	GameOverBackend gameover;
	ContinueResident continue_resident;
	ScoredatBackend score_files;
	RegistBackend registration;
	ContinueBackend continue_ui;
	TotleBackend totle;
	ShotBackendBinding shot_binding;
	PlayerBackendBinding player_binding;
	CardBackendBinding card_binding;
	HudIconsBinding hud_binding;
	ExtendBackendBinding extend_binding;
	TimerBackendBinding timer_binding;
	HudLayoutBinding layout_binding;
	StagePelletBinding pellet_binding;
	StageItemBinding item_binding;
	HudScoreBinding score_binding;
	PelletSpeedBinding speed_binding;
	BombBackendBinding bomb_binding;
	MissBackendBinding miss_binding;
	PauseBackendBinding pause_binding;
	HarryUpAnimationBinding harryup_binding;
	StageTimingBinding timing_binding;
	BossEntityBinding boss_entity_binding;
	SingyokuBinding singyoku_binding;
	BossHitBinding boss_hit_binding;
	DefeatBinding defeat_binding;
	GameOverBinding gameover_binding;
	ScoredatBinding score_files_binding;
	RegistBinding registration_binding;
	ContinueBinding continue_binding;
	TotleBinding totle_binding;
	RegularFrameHooks hooks;
	std::unique_ptr<MimaGameplay> mima;
    std::unique_ptr<YuugenmaganGameplay> yuugenmagan;
	std::unique_ptr<KikuriGameplay> kikuri;
    std::unique_ptr<ElisGameplay> elis;
	std::unique_ptr<KonngaraGameplay> konngara;
 std::unique_ptr<SarielGameplay> sariel;
	Impl(const std::filesystem::path& root,SessionHost supplied,std::uint32_t seed,unsigned starting_stage,std::filesystem::path profile,route_t starting_route,std::optional<GameOptions> options,SessionPurpose purpose_):host(checked(std::move(supplied))),purpose(purpose_),assets(root),fb(initial_framebuffer(assets.background)),
		shots{fb,assets.stg,{}},player{shots,assets.miko,assets.action48,assets.action32,{},native_hud_bombs_render,{}},renderer{fb,assets.stg,assets.font,{}},events(rendered_events(renderer)),
		hud{fb,frame.lives,{}},extend{frame.lives,saved_lives,native_hud_lives_render},timer{fb},hud_layout{fb,assets.mask},
		bomb{fb,assets.kuji1,assets.kuji2,[this]() {wait(1);}},miss{[this](unsigned n) {wait(n);},bgm_counter},pause{[this]() {return host.poll();},[this](unsigned n) {wait(n);},continues},
		boss_entities{fb,assets.archive,{}},singyoku{[this](unsigned n) {wait(n);},[this]() {++music_fade_requests;emit_audio("bgm_fade","");},[this]() {record_boss_clear();singyoku_defeat_animate_and_select_route();boss_defeat_pending=true;},{}},
		boss_hit{frame.frame_rand,frame.lives,hud_hp_decrement,[this]() {if(sariel) sariel->show_palette();else if(konngara) konngara->show_palette();else if(kikuri) kikuri->show_palette();else if(elis) elis->show_palette();else if(yuugenmagan) yuugenmagan->show_palette();else z_palette_set_all_show(singyoku.boss_palette);}},
		defeat{fb,assets.font,singyoku.boss_palette,route,[this]() {return host.poll();},purpose==SessionPurpose::boss_practice},
		gameover{saved_bombs},
		score_files{profile.empty() ? root.parent_path()/"native-profile" : std::move(profile),{}},
		registration{[this]() {return host.poll();},[this](unsigned n) {wait(n);},[this]() {wait(0);}},
		continue_ui{fb,assets.font,continue_resident,continues,[this]() {return host.poll();},[this](unsigned n) {wait(n);},[this]() {++music_fade_requests;emit_audio("bgm_stop","");}},
		totle{fb,assets.totle_screen,assets.numbers,[this]() {return host.poll();},[this](unsigned n) {wait(n);},[this](unsigned ms) {std::this_thread::sleep_for(std::chrono::milliseconds(ms));}},
		shot_binding(shots),player_binding(player),card_binding(events),hud_binding(hud),extend_binding(extend),timer_binding(timer),layout_binding(hud_layout),
		pellet_binding(events),item_binding(events),score_binding(events),speed_binding(player),bomb_binding(bomb,player),miss_binding(miss),pause_binding(pause),
		harryup_binding(timer,assets.font,[this](unsigned n) {wait(n);}),timing_binding([this](unsigned n) {wait(n);}),
		boss_entity_binding(boss_entities),singyoku_binding(singyoku),boss_hit_binding(boss_hit),defeat_binding(defeat),gameover_binding(gameover),
		score_files_binding(score_files),registration_binding(registration),continue_binding(continue_ui),totle_binding(totle) {
		shots.sound_dispatch=[this](unsigned id){if(bgm_enabled&&host.sound)host.sound(id);};
		if(starting_stage<1 || starting_stage>20) throw std::invalid_argument("Start stage must be 1..20");
		if(starting_route!=ROUTE_MAKAI && starting_route!=ROUTE_JIGOKU) throw std::invalid_argument("Unknown route");route=starting_route;
		frame.frame_rand=seed;native_random_seed=seed;score=0;rank=RANK_NORMAL;rem_bombs=3;extend_next=1;native_point_value=0;native_pellet_speed=0;
		if(options) {
			const auto& settings=*options;
			if(settings.rank<0||settings.rank>3||settings.bgm_mode<0||settings.bgm_mode>1||settings.credit_bombs<0||settings.credit_bombs>5||settings.credit_lives_extra<0||settings.credit_lives_extra>4)throw std::invalid_argument("Invalid game options");
			rank=settings.rank;bgm_enabled=settings.bgm_mode!=0;rem_bombs=saved_bombs=settings.credit_bombs;
   const auto extra_lives=settings.mode==GameMode::challenge?GameOptions{}.credit_lives_extra:settings.credit_lives_extra;
			frame.lives=saved_lives=extra_lives+2;
			continue_resident.credit_lives_extra=extra_lives;
   challenge.mode=settings.mode;
   hud.hide_lives=challenge.mode==GameMode::challenge;
   hud.challenge_init=[this](){challenge_display.capture(fb);};
		}
  if(purpose==SessionPurpose::boss_practice) {
   if(!boss_for_stage(starting_stage,route))throw std::invalid_argument("Practice requires a boss stage");
   frame.lives=saved_lives=LIVES_MAX;rem_bombs=saved_bombs=BOMBS_MAX;
  }
		if(purpose==SessionPurpose::normal&&scoredat_load()==0) {hud.hiscore=scoredat_hiscore_get();scoredat_free();}
		cardcombo_cur=0;cardcombo_max=0;card_flip_cycle=irand()%CARD_FLIP_CYCLE_INITIAL_MAX;stage_num=1;stage_cleared=false;test_damage=false;mode_test=false;end_flag=ES_NONE;
		player_is_hit=false;player_invincible=false;player_invincibility_time=0;player_deflecting=false;bomb_damaging=false;bomb_frame=0;
		pellet_interlace=false;Pellets.native_reset_all();reset_item_storage();for(auto& laser:shootout_lasers) laser={};
		for(int i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;Shots.left[i]=0;Shots.top[i]=0;}
		page_access(0);load_cards(assets.layout);if(cards.count) (void)(irand()%cards.count);capture_card_backgrounds(renderer);
		for(unsigned page=0;page<2;++page) render_layout(fb,assets.layout,assets.stg,page);
		load_obstacles(assets.layout);first_stage_in_scene=true;timer_init_for(0,ROUTE_MAKAI);native_hud_layout_render();
		orb_in_portal=false;orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;
		orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;orb_rotation_frame=0;
		player_left=PLAYER_LEFT_START;invincibility_sprites_update_and_render(false);
		hooks.timer=timer_tick_and_put;hooks.invincibility=invincibility_sprites_update_and_render;hooks.shootout_lasers=update_shootout_lasers;hooks.pause_menu=[this](){return purpose==SessionPurpose::boss_practice?practice_pause():pause_menu();};hooks.score_extend=score_extend_update_and_render;
		hooks.boss_update=[this]() {if(stage_id==4) singyoku_main();else if(stage_id==9 && yuugenmagan) yuugenmagan->update();else if(stage_id==9 && route==ROUTE_JIGOKU && mima) mima->update();else if(stage_id==14 && elis) elis->update();else if(stage_id==14 && route==ROUTE_JIGOKU && kikuri) kikuri->update();else if(stage_id==19 && route==ROUTE_JIGOKU && konngara) konngara->update();else if(stage_id==19 && route==ROUTE_MAKAI && sariel) sariel->update();else throw std::logic_error("Boss gameplay is not ported");};
  extend.threshold=[this](){challenge.extend();};
		emit_audio("bgm_load","INIT.MDT");emit_audio("bgm_play","");emit_audio("se_load","ZIGOKU.MDE");
		if(starting_stage==1) {transition_hold();animate_stage_title();begin_life();}
		if(starting_stage==5) {stage_id=4;start_boss();}
		else if(starting_stage==10) {stage_id=9;active_scene=1;if(route==ROUTE_JIGOKU) start_mima();else start_yuugenmagan();}
		else if(starting_stage==15) {stage_id=14;active_scene=2;if(route==ROUTE_JIGOKU) start_kikuri();else start_elis();}
		else if(starting_stage==20) {stage_id=19;active_scene=3;if(route==ROUTE_JIGOKU) start_konngara();else start_sariel();}
		else if(starting_stage>=2) {stage_id=starting_stage-1;active_scene=stage_id/5;assets.load_scene(active_scene==0?0:active_scene*2-1+(route==ROUTE_JIGOKU));reload_current_stage(true);}
	}
 void record_boss_clear() {
  if(purpose!=SessionPurpose::normal||unlocked_stage==int(stage_id))return;
  auto boss=boss_for_stage(stage_id+1,route);if(!boss)return;
  unlocked_stage=int(stage_id);if(host.boss_defeated)host.boss_defeated(*boss,challenge.mode,unsigned(rank));
 }
 bool practice_pause() {
  PracticeChoiceMenu menu(fb,assets.font,"PAUSED",{"RESUME","RETRY","BOSS LIST"},0);
  std::optional<unsigned> choice;
  while(!(choice=menu.step(host.poll())))host.present_wait(menu.framebuffer(),1);
  paused=false;set_input_keys({});input_reset_sense();
  if(*choice==0)return false;
  practice_pause_result=*choice==1?SessionState::practice_retry:SessionState::practice_exit;
  return true;
 }
	void emit_audio(const std::string& operation,const std::string& name) {
		if(!bgm_enabled)return;
  if(operation=="bgm_play"&&defer_boss_play) {
   if(stage_id>=19)animate_stage_title();
   else {boss_play_pending=true;return;}
  }
  if(operation=="bgm_fade" && sariel && sariel->state().phase==99) {
   GameAudioCommand transition{"se_transition",""};
   audio_commands.push_back(transition);
   if(host.audio)host.audio(transition);
  }
		GameAudioCommand command{operation,name};audio_commands.push_back(command);if(host.audio)host.audio(command);
		if(operation=="bgm_load")current_music=name;
		if(operation=="bgm_stop")current_music.clear();
	}
	void ensure_stage_music() {
		if(!bgm_enabled||stage_id>=19)return;
		std::string name;
		if(stage_id==4)name="positive.mdt";
		else if(stage_id==9)name="LEGEND.mdt";
		else if(stage_id==14)name=route==ROUTE_MAKAI?"kami2.mdt":"kami.mdt";
		else name="ST"+std::to_string(stage_id<5?0:(stage_id/5)*2-1+(route==ROUTE_JIGOKU))+".MDT";
		if(name!=current_music) {emit_audio("bgm_load",name);emit_audio("bgm_play","");}
	}
	void flush_boss_sounds() {
		if(!konngara)return;
		for(unsigned id:konngara->take_sound_commands()) {extra_sound_commands.push_back(id);if(bgm_enabled&&host.sound)host.sound(id);}
	}
 void refresh_challenge_hud() {
  if(challenge.mode!=GameMode::challenge||(state!=SessionState::playing&&state!=SessionState::waiting_for_shot)) {
   fb.status_overlay={};return;
  }
  auto value=challenge.displayed_misses();
  {
   challenge_display.draw(fb,assets.font,value);
  }
 }
	void wait(unsigned n) {flush_boss_sounds();refresh_challenge_hud();host.present_wait(fb,n);}
 void transition_hold() {
  if(!stage_resets_game_state(stage_id))return;
  emit_audio("bgm_stop","");stage_intro=true;stage_card_done=false;
  if(stage_is_boss(stage_id))defer_boss_play=true;
  auto overlay=std::move(fb.text_overlay);fb.text_overlay.assign(640*400,0xff000000u);
  try {wait(90);}catch(...) {fb.text_overlay=std::move(overlay);stage_intro=false;throw;}
  fb.text_overlay=std::move(overlay);stage_intro=false;
 }
	void animate_stage_title() {
  if(stage_card_done&&stage_id>=19)return;
  stage_intro=true;
		if(stage_resets_game_state(stage_id))scene_title_text_animate(fb,assets.font,[this](unsigned n){wait(n);});
		stage_number_text_animate(fb,assets.font,stage_id+1,[this](unsigned n){wait(n);});
  stage_intro=false;defer_boss_play=false;stage_card_done=true;
  if(boss_play_pending){boss_play_pending=false;emit_audio("bgm_play","");}
	}
	void begin_life() {
        if(stage_id<19)ensure_stage_music();
		page_access(0);player_reset();ptn_put_8(player_left,player_top,PTN_MIKO_L);ptn_put_8(orb_cur_left,orb_cur_top,PTN_ORB);
		input_lr=INPUT_NONE;input_shot=false;input_ok=false;paused=false;hud_score_and_cardcombo_render();bomb_doubletap_frame=BOMB_DOUBLETAP_WINDOW*3;
		obstacles_update_and_render(true);set_input_keys({});input_reset_sense();if(player_invincibility_time>1) player_invincible=true;
  auto held=host.poll();blocked_escape=held.escape;blocked_enter=held.enter;blocked_strike=held.strike;
		if(stage_id==4) singyoku_main();else if(stage_id==9 && yuugenmagan) yuugenmagan->update();else if(stage_id==9 && mima) mima->update();else if(stage_id==14 && elis) elis->update();else if(stage_id==14 && kikuri) kikuri->update();else if(stage_id==19 && konngara) konngara->update();else if(stage_id==19 && sariel) sariel->update();
	}
	void begin_play() {input_shot=false;native_random_seed=frame.frame_rand;bomb_doubletap_frame=BOMB_DOUBLETAP_WINDOW;first_stage_in_scene=false;state=SessionState::playing;}
	void advance_stage() {
		stage_cleared=false;player_is_hit=false;++bgm_counter;++stage_id;
		if(native_pellet_speed<0) native_pellet_speed=0;
		if(stage_id==4) {start_boss();return;}
		if(stage_is_boss(stage_id)) {if(stage_id==9 && route==ROUTE_MAKAI) start_yuugenmagan();else if(stage_id==9 && route==ROUTE_JIGOKU) start_mima();else if(stage_id==14 && route==ROUTE_MAKAI) start_elis();else if(stage_id==14 && route==ROUTE_JIGOKU) start_kikuri();else if(stage_id==19 && route==ROUTE_JIGOKU) start_konngara();else if(stage_id==19 && route==ROUTE_MAKAI) start_sariel();else state=SessionState::boss_pending;return;}
		Pellets.unput_and_reset_nonclouds();Shots.unput_and_reset();
		player_invincible=false;player_invincibility_time=0;
		invincibility_sprites_update_and_render(false);
		// Rebuild the settled background and stage objects. Inter-stage entrance
		// text and first-scene entrance are supplied below.
		fb.text_overlay.clear();fb.display_scroll_y=0;
		// The original page-1 restore retains the HUD. Its cached life/bomb
		// renderers assume those pixels survive between regular stages.
		const auto& background=assets.background.images.at(0);
		for(unsigned page=0;page<2;++page) for(int y=64;y<400;++y) for(int x=0;x<640;++x)
			fb.put(page,x,y,background.color.at(y*640+x));
		assets.layout=assets.scene.at(stage_id%5);load_cards(assets.layout);
		if(cards.count) (void)(irand()%cards.count);
		capture_card_backgrounds(renderer);
		for(unsigned page=0;page<2;++page) render_layout(fb,assets.layout,assets.stg,page);
		load_obstacles(assets.layout);stage_num=stage_id+1;
		page_access(0);ptn_put_8(player_left,player_top,PTN_MIKO_L);items_render();
		animate_stage_title();
		timer_init_for(stage_id,route);native_hud_layout_render();cardcombo_max=0;
		orb_in_portal=false;bomb_frame=0;frame.frame_since_start_of_binary=0;
		orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;
		orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;
		player_deflecting=false;bomb_damaging=false;cardcombo_cur=0;
		begin_life();begin_play();
	}
	void start_boss() {
        transition_hold();challenge_display.reset();
        sariel.reset();
		Pellets.unput_and_reset_nonclouds();Shots.unput_and_reset();reset_item_storage();
		// The original switches/reloads MAIN at boss stages, so transient
		// bullet/cloud/laser storage starts fresh while resident stock persists.
		pellet_interlace=false;Pellets.native_reset_all();for(auto& laser:shootout_lasers) laser={};
		player_invincible=false;player_invincibility_time=0;invincibility_sprites_update_and_render(false);
		load_cards({});load_obstacles({});renderer.backgrounds.clear();
		fb.text_overlay.clear();fb.display_scroll_y=0;fb.palette=assets.boss_background.palette;
		for(unsigned page=0;page<2;++page) fb.blit(page,0,0,assets.boss_background.images.at(0),false);
		for(unsigned i=0;i<16;++i) {bomb.current[i].c.r=((fb.palette[i]>>16)&255)/17;bomb.current[i].c.g=((fb.palette[i]>>8)&255)/17;bomb.current[i].c.b=(fb.palette[i]&255)/17;}
		bomb.stage=bomb.current;singyoku.boss_palette=bomb.current;singyoku_load();
		stage_num=5;first_stage_in_scene=true;player_left=PLAYER_LEFT_START;
		animate_stage_title();
		timer_init_for(4,ROUTE_MAKAI);native_hud_layout_render();
		cardcombo_cur=cardcombo_max=0;orb_in_portal=false;bomb_frame=0;frame.frame_since_start_of_binary=0;
		orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;
		orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;
		orb_rotation_frame=0;player_deflecting=false;bomb_damaging=false;begin_life();begin_play();
	}
	    void start_yuugenmagan(){
        transition_hold();challenge_display.reset();
        sariel.reset();
        mima.reset();yuugenmagan.reset();kikuri.reset();elis.reset();konngara.reset();
        pellet_interlace=false;Pellets.native_reset_all();Shots.unput_and_reset();reset_item_storage();
        for(auto& laser:shootout_lasers)laser={};
        player_invincible=false;player_invincibility_time=0;invincibility_sprites_update_and_render(false);
        load_cards({});load_obstacles({});renderer.backgrounds.clear();hud.backgrounds.clear();timer.captured.fill(false);
        auto background=read_grp(read_file(assets.directory/"BOSS2.GRP"));set_background(background);
        bomb.stage=bomb.current;singyoku.boss_palette=bomb.current;
        yuugenmagan=std::make_unique<YuugenmaganGameplay>(fb,assets.archive,[this](){++music_fade_requests;emit_audio("bgm_fade","");},[this](){record_boss_clear();boss_defeat_animate();boss_defeat_pending=true;});
        yuugenmagan->load();stage_num=10;first_stage_in_scene=true;player_left=PLAYER_LEFT_START;
        animate_stage_title();timer_init_for(9,ROUTE_MAKAI);native_hud_layout_render();
        cardcombo_cur=cardcombo_max=0;orb_in_portal=false;bomb_frame=0;player_deflecting=false;bomb_damaging=false;frame.frame_since_start_of_binary=0;
        orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;
        orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;orb_rotation_frame=0;
        begin_life();begin_play();
    }
    void start_mima() {
        transition_hold();challenge_display.reset();
        sariel.reset();
        mima.reset();yuugenmagan.reset();pellet_interlace=false;Pellets.native_reset_all();Shots.unput_and_reset();reset_item_storage();
        for(auto& laser:shootout_lasers) laser={};
        player_invincible=false;player_invincibility_time=0;invincibility_sprites_update_and_render(false);
        load_cards({});load_obstacles({});renderer.backgrounds.clear();hud.backgrounds.clear();timer.captured.fill(false);
        fb.text_overlay.clear();fb.display_scroll_y=0;for(unsigned page=0;page<2;++page) fb.clear(page,0);
        fb.palette.fill(0xff000000u);for(auto& color:bomb.current.colors) color.set(0,0,0);
        mima=std::make_unique<MimaGameplay>(fb,assets.archive,frame.frame_rand,[this](unsigned n) {wait(n);},[](unsigned ms) {std::this_thread::sleep_for(std::chrono::milliseconds(ms));},[this]() {++music_fade_requests;emit_audio("bgm_fade","");},[this]() {record_boss_clear();boss_defeat_animate();boss_defeat_pending=true;});
        mima->load();
        auto background=read_grp(read_file(assets.directory/"BOSS3.GRP"));set_background(background);bomb.stage=bomb.current;singyoku.boss_palette=bomb.current;
        stage_num=10;first_stage_in_scene=true;player_left=PLAYER_LEFT_START;
        animate_stage_title();
        timer_init_for(9,ROUTE_JIGOKU);native_hud_layout_render();
        cardcombo_cur=cardcombo_max=0;orb_in_portal=false;bomb_frame=0;
        player_deflecting=false;bomb_damaging=false;frame.frame_since_start_of_binary=0;
        orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;
        orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;orb_rotation_frame=0;
        begin_life();begin_play();
    }
    void start_elis(){
        transition_hold();challenge_display.reset();
        sariel.reset();
        mima.reset();yuugenmagan.reset();kikuri.reset();elis.reset();konngara.reset();pellet_interlace=false;Pellets.native_reset_all();Shots.unput_and_reset();reset_item_storage();
        for(auto& laser:shootout_lasers)laser={};player_invincible=false;player_invincibility_time=0;invincibility_sprites_update_and_render(false);
        load_cards({});load_obstacles({});renderer.backgrounds.clear();hud.backgrounds.clear();timer.captured.fill(false);
        auto background=read_grp(read_file(assets.directory/"BOSS5.GRP"));set_background(background);bomb.stage=bomb.current;singyoku.boss_palette={};
        elis=std::make_unique<ElisGameplay>(fb,assets.archive,[this](unsigned n){wait(n);},[](unsigned ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));},[this](){++music_fade_requests;emit_audio("bgm_fade","");},[this](){record_boss_clear();boss_defeat_animate();boss_defeat_pending=true;});elis->load();
        stage_num=15;first_stage_in_scene=true;player_left=PLAYER_LEFT_START;animate_stage_title();timer_init_for(14,ROUTE_MAKAI);native_hud_layout_render();
        cardcombo_cur=cardcombo_max=0;orb_in_portal=false;bomb_frame=0;player_deflecting=false;bomb_damaging=false;frame.frame_since_start_of_binary=0;
        orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;orb_rotation_frame=0;
        begin_life();begin_play();
    }
    void start_kikuri() {
        transition_hold();challenge_display.reset();
        sariel.reset();
        mima.reset();yuugenmagan.reset();kikuri.reset();elis.reset();pellet_interlace=false;Pellets.native_reset_all();Shots.unput_and_reset();reset_item_storage();
        for(auto& laser:shootout_lasers) laser={};
        player_invincible=false;player_invincibility_time=0;invincibility_sprites_update_and_render(false);
        load_cards({});load_obstacles({});renderer.backgrounds.clear();hud.backgrounds.clear();timer.captured.fill(false);
        auto background=read_grp(read_file(assets.directory/"BOSS4.GRP"));set_background(background);bomb.stage=bomb.current;
        // Original Kikuri defeat fades to black; hit palette lives in its core.
        singyoku.boss_palette={};
        kikuri=std::make_unique<KikuriGameplay>(fb,assets.archive,[this](unsigned n) {wait(n);},[this]() {++music_fade_requests;emit_audio("bgm_fade","");},[this]() {record_boss_clear();boss_defeat_animate();boss_defeat_pending=true;});
        kikuri->load();fb.clear(0,0);
        stage_num=15;first_stage_in_scene=true;player_left=PLAYER_LEFT_START;
        animate_stage_title();timer_init_for(14,ROUTE_JIGOKU);native_hud_layout_render();
        cardcombo_cur=cardcombo_max=0;orb_in_portal=false;bomb_frame=0;player_deflecting=false;bomb_damaging=false;frame.frame_since_start_of_binary=0;
        orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;
        orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;orb_rotation_frame=0;
        begin_life();begin_play();
    }
    void start_konngara() {
        transition_hold();challenge_display.reset();
        sariel.reset();
        mima.reset();yuugenmagan.reset();kikuri.reset();elis.reset();konngara.reset();pellet_interlace=false;Pellets.native_reset_all();Shots.unput_and_reset();reset_item_storage();
        for(auto& laser:shootout_lasers)laser={};player_invincible=false;player_invincibility_time=0;invincibility_sprites_update_and_render(false);
        load_cards({});load_obstacles({});renderer.backgrounds.clear();hud.backgrounds.clear();timer.captured.fill(false);fb.text_overlay.clear();fb.display_scroll_y=0;page_access(0);
        konngara=std::make_unique<KonngaraGameplay>(fb,assets.directory,[this](unsigned n){wait(n);},[this](){++music_fade_requests;emit_audio("bgm_fade","");},[this](const std::string& op,const std::string& name){emit_audio(op,name);});konngara->load();
        stage_num=20;first_stage_in_scene=true;player_left=PLAYER_LEFT_START;
        animate_stage_title();timer_init_for(19,ROUTE_JIGOKU);native_hud_layout_render();
        cardcombo_cur=cardcombo_max=0;orb_in_portal=false;bomb_frame=0;player_deflecting=false;bomb_damaging=false;frame.frame_since_start_of_binary=0;
        orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;orb_rotation_frame=0;
        begin_life();begin_play();
    }
    void start_sariel() {
        transition_hold();challenge_display.reset();
        sariel.reset();mima.reset();yuugenmagan.reset();kikuri.reset();elis.reset();konngara.reset();
        pellet_interlace=false;Pellets.native_reset_all();Shots.unput_and_reset();reset_item_storage();
        for(auto& laser:shootout_lasers)laser={};player_invincible=false;player_invincibility_time=0;invincibility_sprites_update_and_render(false);
        load_cards({});load_obstacles({});renderer.backgrounds.clear();hud.backgrounds.clear();timer.captured.fill(false);fb.text_overlay.clear();fb.display_scroll_y=0;page_access(0);
        stage_num=20;first_stage_in_scene=true;player_left=PLAYER_LEFT_START;
        sariel=std::make_unique<SarielGameplay>(fb,assets.archive,assets.directory,[this](unsigned n){wait(n);},[](unsigned ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));},[this](){load_cards({});load_obstacles({});renderer.backgrounds.clear();},[this](){first_stage_in_scene=true;native_hud_layout_render();hud_score_and_cardcombo_render();},[this](const std::string& op,const std::string& asset){emit_audio(op,asset);if(op=="bgm_fade")++music_fade_requests;});
        sariel->load();bomb.stage=bomb.current;singyoku.boss_palette={};
        animate_stage_title();timer_init_for(19,ROUTE_MAKAI);native_hud_layout_render();
        cardcombo_cur=cardcombo_max=0;orb_in_portal=false;bomb_frame=0;player_deflecting=false;bomb_damaging=false;frame.frame_since_start_of_binary=0;
        orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;orb_rotation_frame=0;
        begin_life();begin_play();
    }
    void set_background(const SpriteSet& background) {
        challenge_display.reset();
		fb.text_overlay.clear();fb.display_scroll_y=0;fb.palette=background.palette;
		for(unsigned page=0;page<2;++page) fb.blit(page,0,0,background.images.at(0),false);
		for(unsigned i=0;i<16;++i) {bomb.current[i].c.r=((fb.palette[i]>>16)&255)/17;bomb.current[i].c.g=((fb.palette[i]>>8)&255)/17;bomb.current[i].c.b=(fb.palette[i]&255)/17;}
	}
	void after_ending() {
		// Original MAIN waits two seconds before handing its resident result
		// to FUUIN. Preserve the last gameplay picture during this wait.
		wait(120);
		EndingResult result;result.score=score;
		result.highest=std::max(score,continue_resident.score_highest);
		result.rank=rank;result.extra_lives=continue_resident.credit_lives_extra;
		result.makai=route==ROUTE_MAKAI;
		end_flag=result.makai?ES_MAKAI:ES_JIGOKU;
		scene_continues.at(active_scene)=continues;
		for(unsigned i=0;i<4;++i) {result.scene_continues[i]=scene_continues[i];result.continues+=scene_continues[i];}
  if(challenge.mode==GameMode::challenge) {result.good_ending_override=challenge.good_ending();result.challenge=true;}
		konngara.reset();sariel.reset();
		fb.text_overlay.clear();fb.display_scroll_y=0;
		// FUUIN's game_init/graph_start clears the visible page after MAIN
		// exits. Leave no boss background around the centered ending picture.
		fb.clear(0,0);fb.palette.fill(0xff000000u);
		VerdictHost ending;
		ending.load=[this](const char* filename) {
			std::string name=filename;for(auto& c:name)c=std::toupper(static_cast<unsigned char>(c));
			return read_grp(read_file(assets.directory/name));
		};
		ending.wait=[this](unsigned n) {wait(n);};
		ending.text=[this](int x,int y,int fx,const char* text) {assets.font.shift_jis(fb,0,x,y,fx,text);};
		ending.input=[this](bool reset) {
			set_input_keys(host.poll());if(reset) input_reset_sense();else input_sense(false);
			return std::array<bool,2>{bool(input_ok),bool(input_shot)};
		};
		ending.random=[]() {return unsigned(irand());};
		ending.registration_colors=[this]() {
			// Registration uses the common palette backend, so transfer the
			// verdict's half-bright palette before setting its two text colors.
			for(unsigned i=0;i<16;++i) {
				bomb.current[i].c.r=(fb.palette[i]>>16&255)/17;
				bomb.current[i].c.g=(fb.palette[i]>>8&255)/17;
				bomb.current[i].c.b=(fb.palette[i]&255)/17;
			}
			z_palette_set_show(3,7,15,7);z_palette_set_show(7,15,15,15);
		};
		ending.registration=[this](std::uint32_t value,int stage,std::uint16_t cleared_route) {
			state=SessionState::score_registration;page_access(0);
			regist_menu(static_cast<score_t>(value),stage,cleared_route);
		};
		ending.resident_clear=[this]() {
			score=0;continue_resident.score=continue_resident.score_highest=0;
			continues=0;scene_continues.fill(0);stage_id=active_scene=0;end_flag=ES_NONE;
		};
		play_ending(fb,result,std::move(ending),[this](const char* operation,const char* name) {
			emit_audio(std::string("bgm_")+operation,name);
		});
		state=SessionState::title_pending;
	}
 void gameover_fade_in() {
		if(stage_id==4) singyoku_free();else if(stage_id==9) {mima.reset();yuugenmagan.reset();}else if(stage_id==14) {kikuri.reset();elis.reset();}else if(stage_id==19) {konngara.reset();sariel.reset();}
		set_background(assets.gameover_background);auto target=fb.palette;fb.palette.fill(0xff000000u);
		// Original black-in: sixteen VSync steps, then 10 ms per step.
		for(unsigned step=1;step<=16;++step) {
			wait(1);for(unsigned i=0;i<16;++i) {unsigned c=0xff000000u;for(unsigned shift:{0u,8u,16u}) c|=std::min(step,(target[i]>>shift&255)/17)*17<<shift;fb.palette[i]=c;}
			wait(0);std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
 }
 void present_practice_result() {
  if(purpose!=SessionPurpose::boss_practice)return;
  auto show_score=[this]() {
   const auto label="SCORE "+std::to_string(score);
   assets.font.ascii(fb,0,320-int(label.size())*4,368,15,label);
   wait(90);
  };
  emit_audio("bgm_stop","");
  if(state==SessionState::practice_cleared) {
   totle.automatic_result=show_score;
   totle_animate(stage_id+1);
   totle.automatic_result={};
  } else if(state==SessionState::practice_failed) {
   gameover_fade_in();
   show_score();
  }
 }
 void after_gameover() {
  state=SessionState::score_registration;
  gameover_fade_in();
		regist_menu(score,stage_id+1,stage_id<5 ? 0xd08e : route==ROUTE_MAKAI ? 0x8296 : 0x6e92);
		state=SessionState::continue_prompt;continue_resident.score=score;continue_resident.point_value=native_point_value;
		if(continue_menu()) restart_after_continue();else state=SessionState::title_pending;
	}
	void restart_after_continue() {
		score=continue_resident.score;frame.lives=saved_lives=continue_resident.rem_lives;
		if(scoredat_load()==0) {hud.hiscore=scoredat_hiscore_get();scoredat_free();}
		rem_bombs=saved_bombs;native_point_value=continue_resident.point_value;extend_next=1;
		reload_current_stage(false);
	}
	void reload_current_stage(bool scene_music) {
		extend_next=score/SCORE_PER_EXTEND+1;
		stage_cleared=false;player_is_hit=false;paused=false;test_damage=false;boss_defeat_pending=false;
		pellet_interlace=false;Pellets.native_reset_all();reset_item_storage();for(auto& laser:shootout_lasers) laser={};
		for(int i=0;i<8;++i) {Shots.moving[i]=false;Shots.decay_frame[i]=0;}
		native_random_seed=frame.frame_rand;card_flip_cycle=irand()%CARD_FLIP_CYCLE_INITIAL_MAX;
		bgm_counter=scene_music ? 0 : 1;frame.frame_since_start_of_binary=0;cardcombo_cur=cardcombo_max=0;
		player_invincible=false;player_invincibility_time=0;player_deflecting=false;bomb_damaging=false;bomb_frame=0;
		invincibility_sprites_update_and_render(false);hud.backgrounds.clear();timer.captured.fill(false);
		if(stage_id==4) {start_boss();return;}
		if(stage_id==9 && route==ROUTE_MAKAI) {start_yuugenmagan();return;}
        if(stage_id==9 && route==ROUTE_JIGOKU) {start_mima();return;}
		if(stage_id==14 && route==ROUTE_MAKAI) {start_elis();return;}
        if(stage_id==14 && route==ROUTE_JIGOKU) {start_kikuri();return;}
		if(stage_id==19 && route==ROUTE_JIGOKU) {start_konngara();return;}
        if(stage_id==19 && route==ROUTE_MAKAI) {start_sariel();return;}
		set_background(assets.background);bomb.stage=bomb.current;assets.layout=assets.scene.at(stage_id%5);
  transition_hold();
		page_access(0);load_cards(assets.layout);if(cards.count) (void)(irand()%cards.count);capture_card_backgrounds(renderer);
		for(unsigned page=0;page<2;++page) render_layout(fb,assets.layout,assets.stg,page);
		load_obstacles(assets.layout);first_stage_in_scene=true;stage_num=stage_id+1;
		animate_stage_title();
		timer_init_for(stage_id,route);native_hud_layout_render();player_left=PLAYER_LEFT_START;
		orb_in_portal=false;orb_cur_left=orb_prev_left=ORB_LEFT_START;orb_cur_top=orb_prev_top=ORB_TOP_START;
		orb_force_new(ORB_FORCE_START,OF_IMMEDIATE);orb_velocity_x=ORB_VELOCITY_X_START;
		orb_rotation_frame=0;
		begin_life();state=SessionState::waiting_for_shot;
	}
	void enter_next_scene() {
		if(stage_id!=4 && stage_id!=9 && stage_id!=14) throw std::logic_error("Post-boss scene handoff is not ported");
		scene_continues.at(active_scene)=continues;++stage_id;active_scene=stage_id/5;continues=scene_continues.at(active_scene);
		assets.load_scene(active_scene*2-1+(route==ROUTE_JIGOKU));
		if(native_pellet_speed<0) native_pellet_speed=0;
		reload_current_stage(true);
	}
	SessionState step(InputKeys keys) {
		if(state==SessionState::waiting_for_shot) {
   blocked_escape|=keys.escape;blocked_enter|=keys.enter;blocked_strike|=keys.strike;
   keys.escape=keys.enter=keys.strike=false;
			set_input_keys(keys);input_sense(false);player_unput_update_render();wait(1);++bomb_frame;
			if(input_shot) {set_input_keys({});input_reset_sense();begin_play();}
		} else if(state==SessionState::playing) {
   auto consume=[](bool& key,bool& blocked){if(blocked){if(!key)blocked=false;key=false;}};
   consume(keys.escape,blocked_escape);consume(keys.enter,blocked_enter);consume(keys.strike,blocked_strike);
			auto result=regular_frame(frame,stage_id,keys,hooks);
			if((konngara && konngara->state().cleared) || (sariel && sariel->state().cleared)) {record_boss_clear();Pellets.native_reset_all();for(auto& laser:shootout_lasers)laser={};state=purpose==SessionPurpose::boss_practice?SessionState::practice_cleared:SessionState::ending_pending;}else if(boss_defeat_pending) {state=purpose==SessionPurpose::boss_practice?SessionState::practice_cleared:SessionState::boss_defeated;}
			else if(result==FrameResult::quit) state=purpose==SessionPurpose::boss_practice?practice_pause_result:SessionState::quit;
			else if(result==FrameResult::stage_cleared) {
				resume_after_miss();
                // A last-card clear can skip bomb frame 140. Its inversion and
                // scroll must be removed before the bonus screen and next stage
                // reuse the HUD; page 1 is the game's clean backing page.
                fb.pages[0]=fb.pages[1];fb.display_scroll_y=0;bomb_damaging=false;
                z_palette_set_all_show(native_bomb_stage_palette());
				auto bonus=stage_bonus_values(stage_timer,cardcombo_max,frame.lives,rem_bombs,stage_id+1);
				render_stage_bonus(fb,assets.font,stage_id+1,bonus,[this](unsigned n) {wait(n);});
				score+=bonus.total;hud_score_and_cardcombo_render();score_extend_update_and_render();
				scene_bonuses.at(stage_id%5)=bonus.total;
				input_reset_sense();input_shot=true;input_ok=true;
				state=SessionState::stage_cleared;
			}
			else if(result==FrameResult::hit) {
    if(challenge.mode==GameMode::challenge) {
     challenge.miss();if(frame.lives<=0){frame.lives=1;saved_lives=1;}
    }
				if(resume_after_miss()) {begin_life();begin_play();} else {
					++bgm_counter;player_gameover_animate();Shots.unput_and_reset();Pellets.unput_and_reset_nonclouds();extend_next=1;state=purpose==SessionPurpose::boss_practice?SessionState::practice_failed:SessionState::game_over;
				}
			}
		} else if(state==SessionState::stage_cleared) {
			set_input_keys(keys);input_sense(false);
			if(!(input_shot && input_ok)) advance_stage();
		} else if(state==SessionState::ending_pending) {
			after_ending();
		} else if(state==SessionState::game_over) {
			after_gameover();
		} else if(state==SessionState::boss_defeated) {
			z_vsync_wait_and_scrollup(0);resume_after_miss();emit_audio("bgm_stop","");if(stage_id==4) singyoku_free();else {mima.reset();yuugenmagan.reset();kikuri.reset();elis.reset();}totle_animate(stage_id+1);state=SessionState::next_scene_pending;
		} else if(state==SessionState::next_scene_pending) {
			enter_next_scene();
		}
		// Observer histories are per-tick; callbacks and actor state persist.
  refresh_challenge_hud();
		events.hud.clear();events.items.clear();events.pellets.clear();return state;
	}
};
StageSession::StageSession(const std::filesystem::path& root,SessionHost host,std::uint32_t seed,unsigned starting_stage,std::filesystem::path profile,route_t starting_route,std::optional<GameOptions> options,SessionPurpose purpose):impl(std::make_unique<Impl>(root,std::move(host),seed,starting_stage,std::move(profile),starting_route,options,purpose)) {}
ChallengeRules StageSession::challenge_rules() const {return impl->challenge;}
StageSession::~StageSession()=default;
void StageSession::present_practice_result() {impl->present_practice_result();}
SessionState StageSession::step(const InputKeys& keys) {auto state=impl->step(keys);impl->flush_boss_sounds();return state;}
SarielGameplayState StageSession::sariel_state() const {if(!impl->sariel)throw std::logic_error("Sariel is not active");return impl->sariel->state();}
std::vector<GameAudioCommand> StageSession::take_audio_commands(){std::vector<GameAudioCommand> out;out.swap(impl->audio_commands);return out;}
SessionState StageSession::state() const {return impl->state;}
unsigned StageSession::stage_number() const {return impl->stage_id+1;}
route_t StageSession::selected_route() const {return impl->route;}
const Framebuffer& StageSession::framebuffer() const {return impl->fb;}
KikuriGameplayState StageSession::kikuri_state() const {if(!impl->kikuri) throw std::logic_error("Kikuri is not active");return impl->kikuri->state();}
KonngaraGameplayState StageSession::konngara_state() const {if(!impl->konngara) throw std::logic_error("Konngara is not active");return impl->konngara->state();}
MimaGameplayState StageSession::mima_state() const {if(!impl->mima) throw std::logic_error("Mima is not active");return impl->mima->state();}
std::vector<unsigned> StageSession::take_sound_commands() {std::vector<unsigned> out;out.swap(impl->shots.sound_commands);impl->flush_boss_sounds();out.insert(out.end(),impl->extra_sound_commands.begin(),impl->extra_sound_commands.end());impl->extra_sound_commands.clear();return out;}
}

namespace th01_native {
YuugenmaganGameplayState StageSession::yuugenmagan_state() const {if(!impl->yuugenmagan)throw std::logic_error("YuugenMagan is not active");return impl->yuugenmagan->state();}
}

namespace th01_native {
ElisGameplayState StageSession::elis_state() const {if(!impl->elis)throw std::logic_error("Elis is not active");return impl->elis->state();}
}

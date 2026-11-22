#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if defined(__APPLE__) && TARGET_OS_IPHONE
#define TH01_IOS_BUILD 1
#include "ios/ios_paths.h"
#include "ios/touch_controls.hpp"
#else
#define TH01_IOS_BUILD 0
#define SDL_MAIN_HANDLED
#endif
#include <SDL.h>
#include "stage_session.hpp"
#include "title_menu.hpp"
#include "zunsoft_intro.hpp"
#include "sdl_music.hpp"
#include "mode_menu.hpp"
#include "practice_menu.hpp"
#include "controller_input.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include <cctype>
#include "framebuffer_dump.hpp"
#include <chrono>
#include <iostream>
#include <thread>
using namespace th01_native;
struct WindowClosed {};
int main(int argc,char** argv) {
	try {
#if TH01_IOS_BUILD
		const std::filesystem::path docs_dir(ios_documents_dir());
		std::filesystem::path launch_assets;
		for(const auto& candidate : {docs_dir/"assets", docs_dir/"th01-ja", docs_dir, std::filesystem::path(ios_bundle_assets_dir())}) {
			if(!candidate.empty() && std::filesystem::exists(candidate/"FONT.ROM")) {
				launch_assets=candidate;
				break;
			}
		}
		if(launch_assets.empty()) {
			std::error_code ec;
			std::filesystem::create_directories(docs_dir/"assets", ec);
			SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_INFORMATION,
				"TH01 Native - Missing Assets",
				"Copy your Touhou 1 assets folder (containing FONT.ROM and TOUHOU.DAT) into:\nFiles -> On My iPhone -> TH01 Native -> assets\nThen reopen the app.",
				nullptr);
			return 0;
		}
#else
  const std::filesystem::path launch_assets=argc>=2?std::filesystem::path(argv[1]):std::filesystem::absolute(argv[0]).parent_path()/"assets";
#endif
		unsigned limit=0,starting_stage=1;bool autoplay=false,unpaced=false,direct_stage=false;std::string dump;
		route_t starting_route=ROUTE_MAKAI;
		for(int i=2;i<argc;++i) {
			std::string arg=argv[i];
			if(arg=="--autoplay") autoplay=true;
			else if(arg=="--unpaced") unpaced=true;
			else if(arg=="--route" && i+1<argc) {std::string value=argv[++i];if(value=="makai") starting_route=ROUTE_MAKAI;else if(value=="jigoku") starting_route=ROUTE_JIGOKU;else throw std::invalid_argument("Unknown route: "+value);}
			else if((arg=="--frames" || arg=="--dump" || arg=="--stage") && i+1<argc) {if(arg=="--frames") limit=std::stoul(argv[++i]);else if(arg=="--stage") {starting_stage=std::stoul(argv[++i]);direct_stage=true;}else dump=argv[++i];}
			else throw std::invalid_argument("Unknown or incomplete option: "+arg);
		}
		if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_TIMER|SDL_INIT_AUDIO)) throw std::runtime_error(SDL_GetError());
		struct Cleanup {~Cleanup() {SDL_Quit();}} cleanup;
#if TH01_IOS_BUILD
		const Uint32 window_flags=SDL_WINDOW_FULLSCREEN|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE;
#else
		const Uint32 window_flags=SDL_WINDOW_RESIZABLE;
#endif
		std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)> window(SDL_CreateWindow("Touhou 1 - HRtP - Z shot / X strike / Esc pause",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,960,600,window_flags),SDL_DestroyWindow);
		if(!window) throw std::runtime_error(SDL_GetError());
		std::unique_ptr<SDL_Renderer,decltype(&SDL_DestroyRenderer)> renderer(SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_ACCELERATED),SDL_DestroyRenderer);
		if(!renderer) renderer.reset(SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_SOFTWARE));
		if(!renderer || SDL_RenderSetLogicalSize(renderer.get(),640,400)) throw std::runtime_error(SDL_GetError());
		std::unique_ptr<SDL_Texture,decltype(&SDL_DestroyTexture)> texture(SDL_CreateTexture(renderer.get(),SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,640,400),SDL_DestroyTexture);
		if(!texture) throw std::runtime_error(SDL_GetError());
		std::unique_ptr<SDL_GameController,decltype(&SDL_GameControllerClose)> pad(nullptr,SDL_GameControllerClose);
		auto connect=[&]() {if(pad && !SDL_GameControllerGetAttached(pad.get())) pad.reset();if(!pad) for(int i=0;i<SDL_NumJoysticks();++i) if(SDL_IsGameController(i)) {pad.reset(SDL_GameControllerOpen(i));if(pad) break;}};
		connect();
#if TH01_IOS_BUILD
		TouchControls touch_controls;
#endif
		SdlMusic music(launch_assets);
  bool menu_controls=!direct_stage;
  auto* start_source=SDL_getenv("TH01_START_KEY_ONLY");
  bool mapped_start_only=start_source&&std::string(start_source)=="1";
		auto poll=[&]() {
			music.check();
			SDL_Event event;while(SDL_PollEvent(&event)) {
				if(event.type==SDL_QUIT) throw WindowClosed{};
				if(event.type==SDL_CONTROLLERDEVICEADDED || event.type==SDL_CONTROLLERDEVICEREMOVED) connect();
#if TH01_IOS_BUILD
				touch_controls.handle_event(event);
#endif
			}
			auto* k=SDL_GetKeyboardState(nullptr);if(k[SDL_SCANCODE_Q]) throw WindowClosed{};
			InputKeys keys;keys.left=k[SDL_SCANCODE_LEFT];keys.right=k[SDL_SCANCODE_RIGHT];keys.up=k[SDL_SCANCODE_UP];keys.down=k[SDL_SCANCODE_DOWN];keys.shot=k[SDL_SCANCODE_Z];keys.strike=k[SDL_SCANCODE_X];keys.escape=k[SDL_SCANCODE_ESCAPE];keys.enter=k[SDL_SCANCODE_RETURN];keys.num_up=k[SDL_SCANCODE_KP_8];keys.num_down=k[SDL_SCANCODE_KP_2];keys.num_left=k[SDL_SCANCODE_KP_4];keys.num_right=k[SDL_SCANCODE_KP_6];
			bool mapped_start=k[SDL_SCANCODE_F1],controller_start=false;
			if(pad) {
				auto button=[&](SDL_GameControllerButton b) {return SDL_GameControllerGetButton(pad.get(),b)!=0;};
				auto x=SDL_GameControllerGetAxis(pad.get(),SDL_CONTROLLER_AXIS_LEFTX),y=SDL_GameControllerGetAxis(pad.get(),SDL_CONTROLLER_AXIS_LEFTY);
				keys.left|=button(SDL_CONTROLLER_BUTTON_DPAD_LEFT)||x<-12000;keys.right|=button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT)||x>12000;
				keys.up|=button(SDL_CONTROLLER_BUTTON_DPAD_UP)||y<-12000;keys.down|=button(SDL_CONTROLLER_BUTTON_DPAD_DOWN)||y>12000;
				keys.shot|=button(SDL_CONTROLLER_BUTTON_A);keys.strike|=button(SDL_CONTROLLER_BUTTON_B);controller_start=button(SDL_CONTROLLER_BUTTON_START);keys.enter|=button(SDL_CONTROLLER_BUTTON_BACK);
			}
#if TH01_IOS_BUILD
			{
				static bool prev_touch_pause=false;
				auto ts=touch_controls.state();
				keys.left|=ts.left;keys.right|=ts.right;keys.up|=ts.up;keys.down|=ts.down;
				keys.shot|=ts.shot;keys.strike|=ts.strike;
				bool pause_edge=ts.pause && !prev_touch_pause;
				prev_touch_pause=ts.pause;
				if(pause_edge && !menu_controls) keys.escape=true;
			}
#endif
   apply_start_button(keys,start_button_down(mapped_start,controller_start,mapped_start_only),menu_controls);
			return keys;
		};
		auto deadline=std::chrono::steady_clock::now();auto last_immediate=deadline-std::chrono::seconds(1);unsigned presented=0;
		auto present=[&](const Framebuffer& fb,unsigned count) {
			if(!count && !unpaced && std::chrono::steady_clock::now()-last_immediate<std::chrono::milliseconds(16)) {SDL_Delay(1);return;}
			for(unsigned i=0;i<std::max(1u,count);++i) {
				(void)poll();auto pixels=fb.rgba(0);
				if(SDL_UpdateTexture(texture.get(),nullptr,pixels.data(),640*4) || SDL_RenderClear(renderer.get()) || SDL_RenderCopy(renderer.get(),texture.get(),nullptr,nullptr)) throw std::runtime_error(SDL_GetError());
#if TH01_IOS_BUILD
				touch_controls.render(renderer.get());
#endif
				SDL_RenderPresent(renderer.get());++presented;last_immediate=std::chrono::steady_clock::now();
				// Provisional host cadence; original hardware timing needs validation.
				if(count && !unpaced) {deadline+=std::chrono::nanoseconds(16666667);auto now=std::chrono::steady_clock::now();if(now>deadline+std::chrono::milliseconds(250)) deadline=now;std::this_thread::sleep_until(deadline);}
			}
		};
#if TH01_IOS_BUILD
		std::filesystem::path asset_root=launch_assets,profile=docs_dir/"native-profile";
#else
		std::filesystem::path asset_root=launch_assets,profile=asset_root.parent_path()/"native-profile";
#endif
		FontRom font(read_file(asset_root/"FONT.ROM"));
  BossProgress progress(profile);
		auto load_title=[&](const char* filename) {std::string name=filename;for(auto& c:name)c=std::toupper(static_cast<unsigned char>(c));return read_grp(read_file(asset_root/name));};
		TitleHost title_host{load_title,present,[](unsigned ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));},[&](const char* operation,const char* name){music.command(operation,name);}};
		SessionHost game_host{poll,present,[&](const GameAudioCommand& command){music.command(command.operation,command.asset);},[&](unsigned id){music.effect(id);}};
  game_host.boss_defeated=[&](BossId boss,GameMode mode,unsigned difficulty){
   if(!direct_stage&&!progress.defeated(mode,difficulty,boss))std::cerr<<progress.error()<<": OTHER.DAT\n";
  };
		std::unique_ptr<TitleMenu> title;
		std::unique_ptr<StageSession> session;
		unsigned continue_stage=1;route_t continue_route=ROUTE_MAKAI;
  std::optional<GameOptions> remembered_settings;
        bool consume_intro_key=false;
		if(direct_stage)session=std::make_unique<StageSession>(asset_root,game_host,1,starting_stage,profile,starting_route);
        else {
            ZunSoftIntro intro(read_file(asset_root/"TOUHOU.DAT"));
            while(intro.step(poll()))present(intro.framebuffer(),2);
            consume_intro_key=intro.skipped();
            title=std::make_unique<TitleMenu>(font,profile,title_host);
        }
  auto practice_background=[&](){
   auto image=load_title("REIIDEN3.GRP");Framebuffer back;back.palette=load_title("REIIDEN2.GRP").palette;back.blit(0,0,0,image.images.at(0),false);return back;
  };
  auto practice=[&](GameOptions settings){
   auto back=practice_background();BossPracticeMenu menu(back,font,progress,unsigned(settings.rank));
   while(!menu.cancelled()) {
    menu_controls=true;
    if(!menu.ready()){menu.step(poll());present(menu.framebuffer(),1);continue;}
    auto chosen=menu.selected();
    if(!progress.unlocked(menu.mode(),menu.difficulty(),chosen)){menu.resume();continue;}
    settings.mode=menu.mode();settings.rank=menu.difficulty();
    const auto entry=boss_entries().at(unsigned(chosen));
    bool retry=true;
    while(retry) {
     retry=false;menu_controls=false;music.command("bgm_stop");
     session.reset();
     session=std::make_unique<StageSession>(asset_root,game_host,1,entry.stage,settings.mode==GameMode::challenge?profile/"challenge":profile,entry.route,settings,SessionPurpose::boss_practice);
     for(;;){
      auto state=session->step(poll());session->take_sound_commands();session->take_audio_commands();
      if(state==SessionState::practice_retry){retry=true;break;}
      if(state==SessionState::practice_exit||state==SessionState::quit||state==SessionState::title_pending)break;
      if(state==SessionState::practice_cleared||state==SessionState::practice_failed){
       session->present_practice_result();
       break;
      }
     }
     session.reset();
    }
    music.command("bgm_stop");menu.resume();
    if(settings.bgm_mode){music.command("bgm_load","reimu.mdt");music.command("bgm_play");}
   }
   menu_controls=true;
   // Menu-only Back keeps the current title track at its existing position.
   title->resume_menu();
  };
		unsigned ticks=0,title_auto_ticks=0;
		while(!limit || ticks<limit) {
            auto keys=poll();
            if(consume_intro_key){if(!any_menu_input(keys))consume_intro_key=false;keys={};}
            if(autoplay) {
    if(title) {keys={};keys.shot=title_auto_ticks==0||title_auto_ticks==2;++title_auto_ticks;}
    else {keys.shot=true;keys.left=(ticks/90)%2==0;keys.right=!keys.left;}
   }
			++ticks;
			if(title) {
				auto action=title->step(keys);
				SDL_SetWindowTitle(window.get(),"TH01 port - Start / Continue / Boss Prac / Option");
				if(action==TitleAction::quit)break;
    if(action==TitleAction::boss_practice){if(progress.has_completion())practice(title->options());else title->resume_menu();continue;}
				if(action==TitleAction::new_game || action==TitleAction::continue_game) {
					auto settings=title->options();
     if(!autoplay) {
      // The press selecting Start is consumed before mode confirmation.
      auto back=load_title("REIIDEN3.GRP");Framebuffer mode_back;mode_back.palette=load_title("REIIDEN2.GRP").palette;mode_back.blit(0,0,0,back.images.at(0),false);
      auto frame=load_title("OP_WIN.GRP").images.at(0);
      for(unsigned i=0;i<frame.color.size();++i)frame.opaque[i]=frame.color[i]!=15;
      mode_back.blit(0,0,0,frame,true);
      ModeMenu menu(mode_back,font);
      while(!menu.step(poll()))present(menu.framebuffer(),1);
      if(menu.was_cancelled()) {title->resume_menu();continue;}
      settings.mode=menu.mode();
     }
     menu_controls=false;music.command("bgm_fade");title.reset();
     remembered_settings=settings;
					session=std::make_unique<StageSession>(asset_root,game_host,ticks,
					 action==TitleAction::new_game?1:continue_stage,settings.mode==GameMode::challenge?profile/"challenge":profile,
					 action==TitleAction::new_game?ROUTE_MAKAI:continue_route,settings);
				}
				continue;
			}
			auto state=session->step(keys);session->take_sound_commands();session->take_audio_commands();
			if(state==SessionState::quit || state==SessionState::title_pending) {
				if(direct_stage)break;
				continue_stage=session->stage_number();continue_route=session->selected_route();session.reset();
				menu_controls=true;title=std::make_unique<TitleMenu>(font,profile,title_host,remembered_settings);title_auto_ticks=0;continue;
			}
			if(state==SessionState::stage_cleared || state==SessionState::game_over || state==SessionState::boss_pending) {
				SDL_SetWindowTitle(window.get(),"TH01 native - Q closes");if(keys.escape)break;present(session->framebuffer(),1);
			}else SDL_SetWindowTitle(window.get(),("TH01 native stage "+std::to_string(session->stage_number())+" - Z shot / X strike / Esc pause").c_str());
		}
		if(!dump.empty())dump_bmp(dump,title?title->framebuffer():session->framebuffer());
		std::cout<<"Stage host ticks="<<ticks<<" presentations="<<presented<<" audio_frames="<<music.rendered_frames()<<" state="<<(session?int(session->state()):-1)<<'\n';

	} catch(const WindowClosed&) {return 0;} catch(const std::exception& e) {
		std::cerr<<e.what()<<'\n';
#if TH01_IOS_BUILD
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"TH01 Native Error",e.what(),nullptr);
#endif
		return 1;
	}
}

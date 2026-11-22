#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "assets.hpp"
#include "framebuffer.hpp"
#include "grp.hpp"
#include "stage.hpp"
#include "grf.hpp"
#include <iostream>
#include <memory>

int main(int argc,char** argv) {
	try {
		if(argc<2) throw std::runtime_error("Usage: sprite_preview ASSET_DIR [--asset STG.PTN] [--dump output.bmp] [--frames N]");
		std::string name="STG.PTN",dump;
		unsigned limit=0,layout=0;
		for(int i=2;i<argc;++i) {
			std::string option=argv[i];
			if(i+1>=argc) throw std::runtime_error("Missing option value");
			if(option=="--asset") name=argv[++i];
			else if(option=="--dump") dump=argv[++i];
			else if(option=="--frames") limit=std::stoul(argv[++i]);
			else if(option=="--layout") {layout=std::stoul(argv[++i]);if(layout>=4) throw std::runtime_error("Layout must be 0..3");}
			else throw std::runtime_error("Unknown option: "+option);
		}
		auto archive=th01_native::load_archive(argv[1]);
		std::vector<std::string> names;
		for(auto& e:archive) if(e.name.size()>4) {
			auto ext=e.name.substr(e.name.size()-4);
			if(ext==".PTN" || ext==".GRC" || ext==".BOS") names.push_back(e.name);
			if(e.name.substr(0,5)=="STAGE" && e.name[5]<'7') names.push_back(e.name);
		}
		for(auto& f:std::filesystem::directory_iterator(argv[1]))
			if(f.path().extension()==".GRP") names.push_back(f.path().filename().string());
		auto selected=std::find(names.begin(),names.end(),name);
		if(selected==names.end()) throw std::runtime_error("Unknown sprite asset");
		std::size_t index=selected-names.begin();
		auto framebuffer=std::make_unique<th01_native::Framebuffer>();
		if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_TIMER)) throw std::runtime_error(SDL_GetError());
		struct Quit { ~Quit() { SDL_Quit(); } } quit;
		using Window=std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)>;
		using Renderer=std::unique_ptr<SDL_Renderer,decltype(&SDL_DestroyRenderer)>;
		using Texture=std::unique_ptr<SDL_Texture,decltype(&SDL_DestroyTexture)>;
		Window window(SDL_CreateWindow("TH01 native sprite preview",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,960,600,SDL_WINDOW_RESIZABLE),SDL_DestroyWindow);
		if(!window) throw std::runtime_error(SDL_GetError());
		Renderer renderer(SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC),SDL_DestroyRenderer);
		if(!renderer) renderer.reset(SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_SOFTWARE));
		if(!renderer) throw std::runtime_error(SDL_GetError());
		if(SDL_RenderSetLogicalSize(renderer.get(),640,400)) throw std::runtime_error(SDL_GetError());
		Texture texture(SDL_CreateTexture(renderer.get(),SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,640,400),SDL_DestroyTexture);
		if(!texture) throw std::runtime_error(SDL_GetError());
		std::vector<SDL_GameController*> controllers;
		for(int i=0;i<SDL_NumJoysticks();++i) if(SDL_IsGameController(i)) {
			if(auto* pad=SDL_GameControllerOpen(i)) controllers.push_back(pad);
		}
		struct Pads { std::vector<SDL_GameController*>& pads; ~Pads() { for(auto* pad:pads) SDL_GameControllerClose(pad); } } pads{controllers};
		bool running=true,dirty=true;
		unsigned frame=0;
		std::vector<std::uint32_t> pixels;
		while(running) {
			SDL_Event event;
			while(SDL_PollEvent(&event)) {
				int direction=0;
				if(event.type==SDL_QUIT) running=false;
				if(event.type==SDL_KEYDOWN && !event.key.repeat) {
					auto key=event.key.keysym.sym;
					if(key==SDLK_ESCAPE) running=false;
					if(key==SDLK_RIGHT || key==SDLK_DOWN) direction=1;
					if(key==SDLK_LEFT || key==SDLK_UP) direction=-1;
				}
				if(event.type==SDL_CONTROLLERBUTTONDOWN) {
					if(event.cbutton.button==SDL_CONTROLLER_BUTTON_BACK) running=false;
					if(event.cbutton.button==SDL_CONTROLLER_BUTTON_DPAD_RIGHT) direction=1;
					if(event.cbutton.button==SDL_CONTROLLER_BUTTON_DPAD_LEFT) direction=-1;
				}
				if(direction) {
					index=(index+names.size()+direction)%names.size();
					dirty=true;
				}
			}
			if(dirty) {
				bool stage=names[index].substr(0,5)=="STAGE";
				bool background=names[index].substr(names[index].size()-4)==".GRP";
				auto background_name=stage ? "ST"+names[index].substr(5,1)+".GRP" : names[index];
				auto set=(background || stage) ? th01_native::read_grp(th01_native::read_file(std::filesystem::path(argv[1])/background_name))
					: th01_native::read_sprites(th01_native::asset(archive,names[index]));
				framebuffer->palette=set.palette;
				framebuffer->clear(0,0);
				int left=(background || stage) ? 0 : 8,top=(background || stage) ? 0 : 8,row_height=0;
				for(auto& sprite:set.images) {
					if(left+sprite.width>640) {left=8;top+=row_height+8;row_height=0;}
					if(top+sprite.height>400) break;
					framebuffer->blit(0,left,top,sprite,true,set.monochrome ? 7 : 15);
					left+=sprite.width+8;
					row_height=std::max(row_height,sprite.height);
				}
				if(stage) {
					auto scene=th01_native::read_scene(th01_native::asset(archive,names[index]));
					auto objects=th01_native::read_sprites(th01_native::asset(archive,"STG.PTN"));
					th01_native::render_layout(*framebuffer,scene[layout],objects);
					auto player=th01_native::read_sprites(th01_native::asset(archive,"MIKO.PTN"));
					framebuffer->blit(0,304,368,player.images[0]);
					framebuffer->blit(0,600,280,objects.images[3]);
					auto hud=th01_native::read_hud_mask(th01_native::read_file(std::filesystem::path(argv[1])/"MASK.GRF"));
					framebuffer->blit(0,0,0,hud);
				}
				pixels=framebuffer->rgba(0);
				auto title="TH01 native asset preview | "+names[index]+" | Arrows: asset | Esc: quit";
				SDL_SetWindowTitle(window.get(),title.c_str());
				if(!dump.empty()) {
					auto* surface=SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(),640,400,32,640*4,SDL_PIXELFORMAT_ARGB8888);
					if(!surface) throw std::runtime_error(SDL_GetError());
					int status=SDL_SaveBMP(surface,dump.c_str());
					SDL_FreeSurface(surface);
					if(status) throw std::runtime_error(SDL_GetError());
				}
				if(SDL_UpdateTexture(texture.get(),nullptr,pixels.data(),640*4)) throw std::runtime_error(SDL_GetError());
				std::cout << names[index] << ": " << set.images.size() << " sprites rendered\n";
				dirty=false;
			}
			SDL_SetRenderDrawColor(renderer.get(),0,0,0,255);
			SDL_RenderClear(renderer.get());
			if(SDL_RenderCopy(renderer.get(),texture.get(),nullptr,nullptr)) throw std::runtime_error(SDL_GetError());
			SDL_RenderPresent(renderer.get());
			SDL_Delay(1);
			if(limit && ++frame>=limit) running=false;
		}
	} catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}

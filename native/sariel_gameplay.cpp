#include "sariel_backend.hpp"
#include "sariel_gameplay.hpp"
namespace th01_native {
struct SarielGameplay::Impl {
 SarielBackend backend;SarielBinding binding;bool loaded=false;
 Impl(Framebuffer& fb,const std::vector<Entry>& archive,const std::filesystem::path& assets,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay,std::function<void()> reset,std::function<void()> hud,std::function<void(const std::string&,const std::string&)> audio):backend{fb,archive,assets,std::move(wait),std::move(delay),std::move(reset),std::move(hud),std::move(audio)},binding(backend){}
 ~Impl(){if(loaded)sariel_original::sariel_free();}
};
SarielGameplay::SarielGameplay(Framebuffer& fb,const std::vector<Entry>& archive,const std::filesystem::path& assets,std::function<void(unsigned)> wait,std::function<void(unsigned)> delay,std::function<void()> reset,std::function<void()> hud,std::function<void(const std::string&,const std::string&)> audio):impl(std::make_unique<Impl>(fb,archive,assets,std::move(wait),std::move(delay),std::move(reset),std::move(hud),std::move(audio))){}
SarielGameplay::~SarielGameplay()=default;
void SarielGameplay::load(){if(impl->loaded){sariel_original::sariel_free();impl->loaded=false;}sariel_original::native_sariel_reset();impl->backend.missing_images.clear();try{sariel_original::sariel_entrance(0);sariel_original::sariel_load_and_init();impl->loaded=true;}catch(...){sariel_original::sariel_free();throw;}}
void SarielGameplay::update(){if(!impl->loaded)throw std::logic_error("Sariel gameplay not loaded");sariel_original::sariel_main();}
void SarielGameplay::show_palette(){sariel_original::boss_palette_show();}
SarielGameplayState SarielGameplay::state()const{auto& b=impl->backend;return{sariel_original::boss_hp,sariel_original::boss_phase,sariel_original::boss_phase_frame,b.pattern,b.patterns_done,sariel_original::game_cleared};}
}

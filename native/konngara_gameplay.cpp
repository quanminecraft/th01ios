#include "konngara_backend.hpp"
#include "konngara_gameplay.hpp"
namespace th01_native {
struct KonngaraGameplay::Impl {
    KonngaraEntrance graphics;KonngaraBackend backend;std::unique_ptr<KonngaraBinding> binding;bool loaded=false;
    Impl(Framebuffer& fb,const std::filesystem::path& assets,std::function<void(unsigned)> wait,std::function<void()> fade,std::function<void(const std::string&,const std::string&)> audio):graphics{fb,assets,[](){},std::move(wait),[](const std::string&){},[](){}},backend{graphics} {
        if(audio){graphics.music_load=[audio](const std::string& name){audio("bgm_load",name);};graphics.music_play=[audio](){audio("bgm_play","");};}graphics.release_boss=[](){konngara_original::konngara_free();};graphics.music_stop=[audio](){if(audio)audio("bgm_stop","");};graphics.music_fade=std::move(fade);binding=std::make_unique<KonngaraBinding>(backend);
    }
    ~Impl(){if(loaded)konngara_original::konngara_free();}
};
KonngaraGameplay::KonngaraGameplay(Framebuffer& fb,const std::filesystem::path& assets,std::function<void(unsigned)> wait,std::function<void()> fade,std::function<void(const std::string&,const std::string&)> audio):impl(std::make_unique<Impl>(fb,assets,std::move(wait),std::move(fade),std::move(audio))){}
KonngaraGameplay::~KonngaraGameplay()=default;
void KonngaraGameplay::load(){if(impl->loaded){konngara_original::konngara_free();impl->loaded=false;}try{konngara_original::konngara_load_and_entrance(0);konngara_original::konngara_init();impl->loaded=true;}catch(...){konngara_original::konngara_free();throw;}}
void KonngaraGameplay::update(){if(!impl->loaded)throw std::logic_error("Konngara gameplay not loaded");konngara_original::konngara_main();}
void KonngaraGameplay::show_palette(){konngara_original::boss_palette_show();}
KonngaraGameplayState KonngaraGameplay::state() const{return{konngara_original::boss_hp,konngara_original::boss_phase,konngara_original::boss_phase_frame,impl->backend.pattern,impl->backend.patterns_done,impl->graphics.game_cleared};}
std::vector<unsigned> KonngaraGameplay::take_sound_commands(){std::vector<unsigned> out;out.swap(impl->graphics.sound_commands);return out;}
}

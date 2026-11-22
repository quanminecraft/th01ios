#pragma once
#include "framebuffer.hpp"
#include "input_backend.hpp"
#include "mima_gameplay.hpp"
#include "yuugenmagan_gameplay.hpp"
#include "kikuri_gameplay.hpp"
#include "elis_gameplay.hpp"
#include "konngara_gameplay.hpp"
#include "sariel_gameplay.hpp"
#include "game_options.hpp"
#include "boss_practice.hpp"
#include <optional>
#include "th01/route.hpp"
#include <filesystem>
#include <functional>
#include <memory>
namespace th01_native {
struct SessionHost {
	std::function<InputKeys()> poll;
	std::function<void(const Framebuffer&,unsigned)> present_wait;
 std::function<void(const GameAudioCommand&)> audio;
 std::function<void(unsigned)> sound;
 std::function<void(BossId,GameMode,unsigned)> boss_defeated;
};
enum class SessionState { waiting_for_shot,playing,stage_cleared,game_over,quit,boss_pending,boss_defeated,score_registration,continue_prompt,title_pending,next_scene_pending,ending_pending,practice_cleared,practice_failed,practice_retry,practice_exit };
// Covers both routes and all seven bosses, route/TOTLE/Continue and regular scenes.
// Original ending/verdict/score registration return to a native title boundary.
class StageSession {
	struct Impl;
	std::unique_ptr<Impl> impl;
public:
	StageSession(const std::filesystem::path& assets,SessionHost host,std::uint32_t seed=1,unsigned starting_stage=1,std::filesystem::path profile={},route_t starting_route=ROUTE_MAKAI,std::optional<GameOptions> options={},SessionPurpose purpose=SessionPurpose::normal);
	~StageSession();
	StageSession(const StageSession&)=delete;
	StageSession& operator=(const StageSession&)=delete;
	SessionState step(const InputKeys& keys);
 void present_practice_result();
	SessionState state() const;
	unsigned stage_number() const;
	route_t selected_route() const;
	const Framebuffer& framebuffer() const;
	MimaGameplayState mima_state() const;
    YuugenmaganGameplayState yuugenmagan_state() const;
	KikuriGameplayState kikuri_state() const;
    ElisGameplayState elis_state() const;
	KonngaraGameplayState konngara_state() const;
 SarielGameplayState sariel_state() const;
 std::vector<GameAudioCommand> take_audio_commands();
 ChallengeRules challenge_rules() const;
	std::vector<unsigned> take_sound_commands();
};
}

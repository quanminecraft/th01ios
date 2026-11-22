#include "gameover_backend.hpp"
namespace {th01_native::GameOverBackend* active=nullptr;}
namespace th01_native {
GameOverBinding::GameOverBinding(GameOverBackend& value):previous(active) {active=&value;}
GameOverBinding::~GameOverBinding() {active=previous;}
}
void native_gameover_save_bombs(int bombs) {if(!active) throw std::runtime_error("Game Over saved stock is not bound");active->saved_bombs=bombs;}

#include "extend_backend.hpp"
namespace {
th01_native::ExtendBackend* active=nullptr;
th01_native::ExtendBackend& backend() {
	if(!active) throw std::runtime_error("Native life extend state is not bound");
	return *active;
}
}
namespace th01_native {
ExtendBackendBinding::ExtendBackendBinding(ExtendBackend& value):previous(active) {active=&value;}
ExtendBackendBinding::~ExtendBackendBinding() {active=previous;}
}
std::int16_t& native_extend_lives() {return backend().lives;}
std::int8_t& native_extend_saved_lives() {return backend().saved_lives;}
void hud_lives_put(int previous) {
	if(!backend().lives_hud) throw std::runtime_error("Native lives HUD is not bound");
	backend().lives_hud(previous);
}
void native_extend_threshold() {if(backend().threshold)backend().threshold();}

#include "item_backend.hpp"
std::uint16_t native_point_value=0;
namespace th01_native {
void reset_item_storage() {
	for(auto& item:items_bomb) item={};
	for(auto& item:items_point) item={};
}
}

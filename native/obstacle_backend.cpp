#include "obstacle_backend.hpp"

th01_native::ObstacleState obstacles;
namespace th01_native {
void load_obstacles(const StageLayout& layout) {
	if(layout.obstacles.size()>200) throw std::runtime_error("Too many obstacles for stage grid");
	obstacles={};obstacles.count=static_cast<std::int16_t>(layout.obstacles.size());
	for(auto& object:layout.obstacles) {
		if(!((object.type>=1 && object.type<=13) || (object.type>=17 && object.type<=21)))
			throw std::runtime_error("Invalid obstacle type");
		obstacles.left.push_back(object.left);obstacles.top.push_back(object.top);
		obstacles.type.push_back(static_cast<obstacle_type_t>(object.type));
	}
	obstacles.frame.resize(layout.obstacles.size());
	for(auto& frame:obstacles.frame) frame.v=0;
	turret_fire_update_and_render_or_reset(0,true);
	portal_enter_update_and_render_or_reset(0,true);
}
}

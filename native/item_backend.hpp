#pragma once
#include "card_backend.hpp"
#include "th01/main/stage/item_state.hpp"
#include <utility>
extern item_t items_bomb[ITEM_BOMB_COUNT];
extern item_t items_point[ITEM_POINT_COUNT];
void items_bomb_add(int card);
void items_point_add(int card);
namespace th01_native {
void reset_item_storage();
class StageItemBinding {
	CardEvents& events;
	decltype(CardEvents::item_dispatch) previous;
public:
	explicit StageItemBinding(CardEvents& target):events(target),previous(std::move(target.item_dispatch)) {
		events.item_dispatch=[](unsigned card,bool bomb) {
			if(card>=unsigned(cards.count)) throw std::out_of_range("Item source card");
			if(bomb) items_bomb_add(int(card));else items_point_add(int(card));
		};
	}
	~StageItemBinding() {events.item_dispatch=std::move(previous);}
	StageItemBinding(const StageItemBinding&)=delete;
	StageItemBinding& operator=(const StageItemBinding&)=delete;
};
}

#define item_add(item, from_card_slot) { \
	item.flag = IF_SPLASH; \
	item.left = cards.left[from_card_slot]; \
	item.top = cards.top[from_card_slot]; \
	item.unknown_zero = 0; \
	item.velocity_y = 2; \
	item.flag_state.splash_radius = 4; \
}

void items_bomb_add(int from_card_slot)
{
	for(int i = 0; i < ITEM_BOMB_COUNT; i++) {
		if(items_bomb[i].flag == IF_FREE) {
			item_add(items_bomb[i], from_card_slot);
			return;
		}
	}
}


void items_point_add(int from_card_slot)
{
	for(int i = 0; i < ITEM_POINT_COUNT; i++) {
		if(items_point[i].flag == IF_FREE) {
			item_add(items_point[i], from_card_slot);
			return;
		}
	}
}


#undef item_add

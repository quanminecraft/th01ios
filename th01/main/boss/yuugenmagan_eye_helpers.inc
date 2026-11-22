// For [eye_predicate], pass a macro that takes a single eye_flag_t parameter.
#define eyes_foreach_if(eye_predicate, method) { \
	if(eye_predicate(     EF_WEST)) {      eye_west.method(); } \
	if(eye_predicate(     EF_EAST)) {      eye_east.method(); } \
	if(eye_predicate(EF_SOUTHWEST)) { eye_southwest.method(); } \
	if(eye_predicate(EF_SOUTHEAST)) { eye_southeast.method(); } \
	if(eye_predicate(    EF_NORTH)) {     eye_north.method(); } \
}

inline void eyes_locked_unput_and_put_then_track(eye_flag_t eye_flag) {
	#define eye_flag_is_set(bit) ( \
		eye_flag & bit \
	)

	// Unblitting/rendering first, and *then* tracking? Depending on when the
	// eye's cel is updated within the entity's lock, this order introduces a
	// variable tracking latency from 1 to 4 frames. Which isn't all too bad,
	// and might even be considered somewhat realistic. Certainly not ZUN quirk
	// territory.
	eyes_foreach_if(eye_flag_is_set, locked_unput_and_put_8);
	eyes_foreach_if(eye_flag_is_set, track_player);

	#undef eye_flag_is_set
}

#define eyes_set_image(eye_flag, cel) { \
	if(eye_flag &      EF_WEST) {      eye_west.set_image(cel); } \
	if(eye_flag &      EF_EAST) {      eye_east.set_image(cel); } \
	if(eye_flag & EF_SOUTHWEST) { eye_southwest.set_image(cel); } \
	if(eye_flag & EF_SOUTHEAST) { eye_southeast.set_image(cel); } \
	if(eye_flag &     EF_NORTH) {     eye_north.set_image(cel); } \
}

inline void yuugenmagan_ent_load(void) {
	eye_west.load("boss2.bos", 0);
	eye_east.metadata_assign(eye_west);
	eye_southwest.metadata_assign(eye_west);
	eye_southeast.metadata_assign(eye_west);
	eye_north.metadata_assign(eye_west);
}

inline void yuugenmagan_ent_free(void) {
	bos_entity_free(0);
}
// --------


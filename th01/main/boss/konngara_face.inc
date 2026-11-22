void face_direction_set_and_put(face_direction_t fd_new)
{
	if(!face_direction_can_change || (face_direction == fd_new)) {
		return;
	}
	page_access(1);	head_put(fd_new);
	page_access(0);	head_put(fd_new);
	if(face_expression == FE_AIM) {
		page_access(1);	face_aim_put(fd_new);
		page_access(0);	face_aim_put(fd_new);
	} else if(face_expression != FE_NEUTRAL) {
		page_access(1);	face_put(face_expression, fd_new);
		page_access(0);	face_put(face_expression, fd_new);
	}
	face_direction = fd_new;
}

void face_expression_set_and_put(face_expression_t fe_new)
{
	if(face_expression == fe_new) {
		return;
	}
	if(fe_new == FE_AIM) {
		page_access(1);	face_aim_put(face_direction);
		page_access(0);	face_aim_put(face_direction);
	} else if(fe_new != FE_NEUTRAL) {
		page_access(1);	face_put(fe_new, face_direction);
		page_access(0);	face_put(fe_new, face_direction);
	} else {
		page_access(1);	head_put(face_direction);
		page_access(0);	head_put(face_direction);
	}
	face_expression = fe_new;
}


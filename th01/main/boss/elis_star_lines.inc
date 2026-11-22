// Draws a line from [angle_1] to [angle_2] on the star circle around Elis.
void near starcircle_line_put(
	unsigned char angle_1, unsigned char angle_2, vc_t col
)
{
	screen_x_t p1_x = polar_x(form_center_x(F_GIRL), BIGCIRCLE_RADIUS, angle_1);
	screen_y_t p1_y = polar_y(form_center_y(F_GIRL), BIGCIRCLE_RADIUS, angle_1);
	screen_x_t p2_x = polar_x(form_center_x(F_GIRL), BIGCIRCLE_RADIUS, angle_2);
	screen_y_t p2_y = polar_y(form_center_y(F_GIRL), BIGCIRCLE_RADIUS, angle_2);
	graph_r_line(p1_x, p1_y, p2_x, p2_y, col);
}

void near starcircle_line_unput(
	unsigned char angle_1, unsigned char angle_2
)
{
	screen_x_t p1_x = polar_x(form_center_x(F_GIRL), BIGCIRCLE_RADIUS, angle_1);
	screen_y_t p1_y = polar_y(form_center_y(F_GIRL), BIGCIRCLE_RADIUS, angle_1);
	screen_x_t p2_x = polar_x(form_center_x(F_GIRL), BIGCIRCLE_RADIUS, angle_2);
	screen_y_t p2_y = polar_y(form_center_y(F_GIRL), BIGCIRCLE_RADIUS, angle_2);
	graph_r_line_unput(p1_x, p1_y, p2_x, p2_y);
}


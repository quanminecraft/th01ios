void near wand_lower_both(void)
{
	page_access(1);	wand_lowered_put();
	page_access(0);	wand_lowered_put();
}

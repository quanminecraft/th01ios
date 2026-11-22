#ifdef TH01_NATIVE_ORB_PHYSICS
bool16 bomb_deals_damage(std::uint32_t frame)
#else
bool16 bomb_deals_damage(unsigned long frame)
#endif
{
	if((rem_lives == 0) && (bomb_damaging == true) && ((frame % 50) == 0)) {
		return true;
	}
	return false;
}

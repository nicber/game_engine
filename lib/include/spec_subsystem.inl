template <typename T>
bool specialized_subsystem::accepts(entity& ent)
{
	return dynamic_cast<T*>(&ent);
}
#include <game/gameobjects.h>

#include <game/actor/actorlogic.h>

ac_list ac_actors;

i32 ac_load(const m_obj* obj, u32 count)
{
	ac_destroy();
	for (u32 i = 0; i < count; i++)
		ac_create(&obj[i]);
}

void ac_destroy()
{

}

void ac_create(const m_obj* obj)
{
	ac_actor new_actor;
	ac_make(&new_actor, obj);
	ac_list_add(&ac_actors, &new_actor);
}

void ac_update_objects(u32 delta)
{
	ac_list_node* cur = ac_actors.first;
	while (cur)
	{
		if (ac_update(&cur->actor, delta))
			cur = ac_list_drop(&ac_actors, cur);
		else
			cur = cur->next;
	}
}

ac_actor* ac_get_player()
{
	SDL_assert(ac_actors.first->actor.type == AC_PLAYER);
	return &ac_actors.first->actor;
}

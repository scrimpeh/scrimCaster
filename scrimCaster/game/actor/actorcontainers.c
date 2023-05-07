#include <game/actor/actorcontainers.h>

ac_list_node* ac_list_add(ac_list* list, ac_actor* ac)
{
	// In theory, I could also directly invoke the constructor here
	// rather than doing a redundant copy
	ac_list_node* node = SDL_malloc(sizeof(ac_list_node));
	SDL_memcpy(&node->actor, ac, sizeof(ac_actor));
	node->prev = list->last;
	node->next = NULL;
	if (!list->first)
		list->first = node;
	else
		list->last->next = node;
	list->last = node;
	list->count++;
}

ac_list_node* ac_list_drop(ac_list* list, ac_list_node* node)
{
	if (!node->prev)
		list->first = node->next;
	else
		node->prev->next = node->next;

	if (!node->next)
		list->last = node->prev;
	else
		node->next->prev = node->prev;

	list->count--;
	ac_list_node* next = node->next;
	SDL_free(node);
	return next;
}

void ac_list_clear(ac_list* list)
{
	while (list->first)
		ac_list_drop(list, list->first);
}
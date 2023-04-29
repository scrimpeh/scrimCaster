#include <actorcontainers.h>

void ActorListMake(ActorList* al, u32 capacity)
{
	al->count = 0;
	al->capacity = capacity;
	al->first = al->last = NULL;
}

//Note that the underlying Actors are -not- freed!
void ActorListDestroy(ActorList* al)
{
	ActorNode* node = al->first;
	ActorNode* to_free;

	while (node)
	{
		to_free = node;
		SDL_free(to_free->content);
		SDL_free(to_free);
		node = node->next;
	}
	al->first = al->last = NULL;
	al->count = 0;
}

//Adds a new actor to the list and returns it for editing
Actor* ActorListNew(ActorList* al)
{
	if (al->count >= al->capacity)
		return NULL;

	Actor* a = SDL_malloc(sizeof(Actor));
	ActorNode* an = SDL_malloc(sizeof(ActorNode));
	an->content = a;
	an->next = NULL;
	an->prev = al->last;
	if(al->last) al->last->next = an;
	else al->first = an;
	al->last = an;

	++al->count;

	return a;
}

bool ActorListRemove(ActorList* al, Actor* a)
{
	// Removal not implemented yet
	return false;
}

//Drop an item from the list while iterating through the list
//We rely on the Node actually coming from the List, or wacky shit
//happens
ActorNode* ActorListDrop(ActorList* al, ActorNode* an)
{
	if (!an->prev)
		al->first = an->next;
	else
		an->prev->next = an->next;
	if (!an->next)
		al->last = an->prev;
	else
		an->next->prev = an->prev;

	--al->count;

	ActorNode* next = an->next;

	SDL_free(an->content);
	SDL_free(an);

	return next;
}

void ActorArrayMake(ActorArray* a, u32 count)
{
	a->actor = (Actor*)SDL_calloc(count, sizeof(Actor));
	a->count = count;
}

void ActorArrayDestroy(ActorArray* a)
{
	SDL_free(a->actor);
	a->count = 0;
}

void ActorVectorMake(ActorVector* av, u32 capacity)
{
	av->content = (Actor**)SDL_calloc(capacity, sizeof(void*));
	av->count = 0;
	av->capacity = av->content ? capacity : 0;
}

void ActorVectorDestroy(ActorVector* av)
{
	ActorVectorClear(av);
	SDL_free(av->content);
	av->content = NULL;
	av->capacity = 0;
}

void ActorVectorClear(ActorVector* av)
{
	for (u32 i = 0; i < av->count; ++i)
	{
		Actor* a = av->content[i];
		SDL_free(a);
	}
	av->count = 0;
}

Actor* ActorVectorAdd(ActorVector* av, Actor* a)
{
	if (av->count < av->capacity)
	{
		av->content[av->count++] = a;
		return a;
	}
	return NULL;
}
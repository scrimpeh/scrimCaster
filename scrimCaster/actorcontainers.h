#pragma once

#include <common.h>

#include <actor.h>

//Various containers for actor types. Presumably, should have both a fixed "array type" container (simple), and
//a linked list / variable size container (less simple)

typedef struct
{
	u32 count;
	Actor* actor;
} ActorArray;

typedef struct ActorNode
{
	Actor* content;
	struct ActorNode* next;
	struct ActorNode* prev;
} ActorNode;

typedef struct
{
	u32 count, capacity;
	ActorNode* first;
	ActorNode* last;
} ActorList;

typedef struct
{
	Actor** content;
	u32 count, capacity;
} ActorVector;

//Actor List function
void ActorListMake(ActorList* al, u32 capacity);
void ActorListDestroy(ActorList* al);
Actor* ActorListNew(ActorList* al);
bool ActorListRemove(ActorList* al, Actor* a);
ActorNode* ActorListDrop(ActorList* al, ActorNode* an);

void ActorArrayMake(ActorArray* a, u32 count);
void ActorArrayDestroy(ActorArray* a);

void ActorVectorMake(ActorVector* av, u32 capacity);
void ActorVectorDestroy(ActorVector* av);
void ActorVectorClear(ActorVector* av);
Actor* ActorVectorAdd(ActorVector* av, Actor* a);
#pragma once

#include <common.h>

#include <game/actor/actor.h>

typedef struct ac_list_node
{
	ac_actor actor;
	struct ac_list_node* next;
	struct ac_list_node* prev;
} ac_list_node;

typedef struct
{
	u32 count;
	ac_list_node* first;
	ac_list_node* last;
} ac_list;

void ac_list_clear(ac_list* list);
ac_list_node* ac_list_add(ac_list* list, ac_actor* ac);
ac_list_node* ac_list_drop(ac_list* list, ac_list_node* node);


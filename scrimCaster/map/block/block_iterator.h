#pragma once

#include <common.h>

#include <map/block/blockmap.h>
#include <map/block/block_tree.h>

// Used for retrieving elements from the block map. A block map iterator 
// is a virtual object that can either iterate through the blocks and actors in the blockmap...

// ...at one point
// ...within one rectangle
// ...at a given number of highlighted squares

// The best way is probably to use a balanced binary search tree, with the cells ordered linewise
// An AVL tree is suitable for this, even though I don't want to implement one again...

typedef struct
{
	block_tree* cell_tree;
	block_type type;
	i32 size;

	i32 i;
	block_tree* tree_cur;
	block_ref_list_entry* cur;
} block_iterator;

block_iterator* block_iterator_make_empty(block_type type);
block_iterator* block_iterator_make_pt(block_type type, i64 wx, i64 wy);
block_iterator* block_iterator_make_actor(block_type type, const ac_actor* ac);

void block_iterator_add_cell(block_iterator* iterator, block_pt pt);

void* block_iterator_next(block_iterator* iterator);

// TODO: maybe auto free
void block_iterator_free(block_iterator* iterator);

static bool block_iterator_can_add(const block_iterator* iterator);
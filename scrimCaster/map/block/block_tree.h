#pragma once

#include <common.h>

#include <map/block/blockmap.h>

// A balanced BST used for finding blocks from a grid

typedef struct block_tree
{
	struct block_tree* l;
	struct block_tree* r;
	i32 h;
	i32 mx;
	i32 my;
} block_tree;

void block_tree_free(block_tree* tree);
block_tree* block_tree_add(block_tree* tree, i32 mx, i32 my);
bool block_tree_contains(const block_tree* t, i32 mx, i32 my);
i64 block_tree_size(const block_tree* tree);
void block_tree_nodes(const block_tree* tree, block_pt* to_write);

static i8 block_tree_key_cmp(i32 mx_a, i32 my_a, const block_tree* t);
static block_tree* block_tree_create_node(i32 mx, i32 my);
static i32 block_tree_height(block_tree* tree);
static i32 block_tree_balance(block_tree* tree);

static block_tree* block_tree_rotate_l(block_tree* x);
static block_tree* block_tree_rotate_r(block_tree* y);
static i64 block_tree_nodes_inner(const block_tree* tree, block_pt* to_write, i64 pos);
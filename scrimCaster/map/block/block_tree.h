#pragma once

#include <common.h>

#include <map/block/blockmap.h>

// A balanced BST used for finding blocks from a grid

typedef struct block_tree
{
	struct block_tree* l;
	struct block_tree* r;
	block_pt pt;
	i32 h;

	struct block_tree* parent;
	bool processed;
} block_tree;

block_tree* block_tree_insert(block_tree* tree, block_pt pt);
static block_tree* block_tree_add(block_tree* parent, block_tree* tree, block_pt pt);
static block_tree* block_tree_create_node(block_tree* parent, block_pt pt);

void block_tree_free(block_tree* tree);

bool block_tree_contains(const block_tree* tree, block_pt pt);
static i8 block_tree_key_cmp(const block_tree* tree, block_pt pt);

block_tree* block_tree_next(block_tree* tree);

static i32 block_tree_height(block_tree* tree);
static i32 block_tree_balance(block_tree* tree);

static block_tree* block_tree_rotate_l(block_tree* x);
static block_tree* block_tree_rotate_r(block_tree* y);

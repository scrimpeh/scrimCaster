#include <map/block/block_tree.h>

void block_tree_free(block_tree* tree)
{
	if (!tree)
		return;
	if (tree->l)
		block_tree_free(tree->l);
	if (tree->r)
		block_tree_free(tree->r);
	SDL_free(tree);
}

block_tree* block_tree_insert(block_tree* tree, block_pt pt)
{
	return block_tree_add(NULL, tree, pt);
}

block_tree* block_tree_add(block_tree* parent, block_tree* tree, block_pt pt)
{
	if (!tree)
		return block_tree_create_node(parent, pt);

	const i8 cmp = block_tree_key_cmp(tree, pt);
	if (cmp < 0)
		tree->l = block_tree_add(tree, tree->l, pt);
	else if (cmp > 0)
		tree->r = block_tree_add(tree, tree->r, pt);
	else
		return tree;

	const i32 balance = block_tree_balance(tree);

	if (balance > 1 && block_tree_key_cmp(tree->l, pt) < 0)    // LL
		return block_tree_rotate_r(tree);

	if (balance < -1 && block_tree_key_cmp(tree->r, pt) > 0)   // RR
		return block_tree_rotate_l(tree);

	if (balance > 1 && block_tree_key_cmp(tree->l, pt) > 0)    // LR
	{
		tree->l = block_tree_rotate_l(tree->l);
		return block_tree_rotate_r(tree);
	}

	if (balance < -1 && block_tree_key_cmp(tree->r, pt) < 0)   // RL
	{
		tree->r = block_tree_rotate_r(tree->r);
		return block_tree_rotate_l(tree);
	}

	return tree;
}

static block_tree* block_tree_create_node(block_tree* parent, block_pt pt)
{
	block_tree* tree = SDL_malloc(sizeof(block_tree));
	if (!tree)
		return NULL;
	tree->l = NULL;
	tree->r = NULL;
	tree->h = 0;
	tree->pt = pt;

	tree->parent = parent;
	tree->processed = false;
	return tree;
}

bool block_tree_contains(const block_tree* tree, block_pt pt)
{
	if (!tree)
		return false;
	const i8 cmp = block_tree_key_cmp(tree, pt);
	if (cmp == 0)
		return true;
	return block_tree_contains(cmp < 0 ? tree->l : tree->r, pt);
}

static i8 block_tree_key_cmp(const block_tree* tree, block_pt pt)
{
	if (pt.y > tree->pt.y)
		return 1;
	if (pt.y < tree->pt.y)
		return -1;
	if (pt.x > tree->pt.x)
		return 1;
	if (pt.x < tree->pt.x)
		return -1;
	return 0;
}

block_tree* block_tree_next(block_tree* current)
{
	while (true)
	{
		if (current->l && !current->l->processed)
			current = current->l;
		else if (!current->processed)
		{
			current->processed = true;
			return current;
		}
		else if (current->r && !current->r->processed)
			current = current->r;
		else if (current->parent)
			current = current->parent;
		else
			return NULL;
	}
}

static i32 block_tree_height(block_tree* tree)
{
	return tree ? tree->h : 0;
}

static i32 block_tree_balance(block_tree* tree)
{
	return tree ? block_tree_height(tree->l) - block_tree_height(tree->r) : 0;
}

static block_tree* block_tree_rotate_l(block_tree* x)
{
	block_tree* y = x->r;
	block_tree* yl = y->l;

	y->l = x;
	x->r = yl;
	
	y->parent = x->parent;
	x->parent = y;
	yl->parent = x;

	x->h = SDL_max(block_tree_height(x->l), block_tree_height(x->r)) + 1;
	y->h = SDL_max(block_tree_height(y->l), block_tree_height(y->r)) + 1;

	return y;
}

static block_tree* block_tree_rotate_r(block_tree* y)
{
	block_tree* x = y->l;
	block_tree* xr = x->r;

	x->r = y;
	y->l = xr;

	x->parent = y->parent;
	y->parent = x;
	xr->parent = y;

	y->h = SDL_max(block_tree_height(y->l), block_tree_height(y->r)) + 1;
	x->h = SDL_max(block_tree_height(x->l), block_tree_height(x->r)) + 1;

	return x;
}
#include <map/block/block_tree.h>

void block_tree_free(block_tree* tree)
{
	if (tree->l)
		block_tree_free(tree->l);
	if (tree->r)
		block_tree_free(tree->r);
	SDL_free(tree);
}

i64 block_tree_size(const block_tree* tree)
{
	if (!tree)
		return 0;
	return 1 + block_tree_size(tree->l) + block_tree_size(tree->r);
}

block_tree* block_tree_add(block_tree* tree, i32 mx, i32 my)
{
	if (!tree)
		return block_tree_create_node(mx, my);

	const i8 cmp = block_tree_key_cmp(mx, my, tree);
	if (cmp < 0)
		tree->l = block_tree_add(tree->l, mx, my);
	else if (cmp > 0)
		tree->r = block_tree_add(tree->r, mx, my);
	else
		return tree;

	const i32 balance = block_tree_balance(tree);

	if (balance > 1 && block_tree_key_cmp(mx, my, tree->l) < 0)    // LL
		return block_tree_rotate_r(tree);

	if (balance < -1 && block_tree_key_cmp(mx, my, tree->r) > 0)   // RR
		return block_tree_rotate_l(tree);

	if (balance > 1 && block_tree_key_cmp(mx, my, tree->l) > 0)    // LR
	{
		tree->l = block_tree_rotate_l(tree->l);
		return block_tree_rotate_r(tree);
	}

	if (balance < -1 && block_tree_key_cmp(mx, my, tree->r) < 0)   // RL
	{
		tree->r = block_tree_rotate_r(tree->r);
		return block_tree_rotate_l(tree);
	}

	return tree;
}

bool block_tree_contains(const block_tree* tree, i32 mx, i32 my)
{
	if (!tree)
		return false;
	const i8 cmp = block_tree_key_cmp(mx, my, tree);
	if (cmp == 0)
		return true;
	return block_tree_contains(mx, my, cmp < 0 ? tree->l : tree->r);
}

static i8 block_tree_key_cmp(i32 mx_a, i32 my_a, const block_tree* t)
{
	if (my_a > t->my)
		return 1;
	if (my_a < t->my)
		return -1;
	if (mx_a > t->mx)
		return 1;
	if (mx_a < t->mx)
		return -1;
	return 0;
}

block_tree* block_tree_create_node(i32 mx, i32 my)
{
	block_tree* tree = SDL_malloc(sizeof(block_tree));
	if (!tree)
		return NULL;
	tree->l = NULL;
	tree->r = NULL;
	tree->h = 0;
	tree->mx = mx;
	tree->my = my;
	return tree;
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

	y->h = SDL_max(block_tree_height(y->l), block_tree_height(y->r)) + 1;
	x->h = SDL_max(block_tree_height(x->l), block_tree_height(x->r)) + 1;

	return x;
}

void block_tree_nodes(const block_tree* tree, block_pt* to_write)
{
	block_tree_nodes_inner(tree, to_write, 0);
}

static i64 block_tree_nodes_inner(const block_tree* tree, block_pt* to_write, i64 pos)
{
	if (!tree)
		return pos;
	pos = block_tree_nodes_inner(tree->l, to_write, pos);
	to_write[pos].x = tree->mx;
	to_write[pos].y = tree->my;
	pos = block_tree_nodes_inner(tree->r, to_write, pos + 1);
	return pos + 1;
}
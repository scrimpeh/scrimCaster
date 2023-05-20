#include <map/block/block_iterator.h>

#include <map/map.h>

block_iterator* block_iterator_make_empty(block_type type)
{
	block_iterator* iter = SDL_malloc(sizeof(block_iterator));
	if (!iter)
		return NULL;
	iter->cell_tree = NULL;
	iter->tree_cur = NULL;
	iter->cur = NULL;
	iter->type = type;
	iter->i = 0;
	iter->size = 0;
	return iter;
}

block_iterator* block_iterator_make_pt(block_type type, i64 wx, i64 wy)
{
	block_iterator* iter = block_iterator_make_empty(type);
	if (!iter)
		return NULL;

	block_pt pt;
	pt.x = wx / M_CELLSIZE;
	pt.y = wy / M_CELLSIZE;
	block_iterator_add_cell(iter, pt);
	return iter;
}

block_iterator* block_iterator_make_actor(block_type type, const ac_actor* actor)
{
	block_iterator* iter = block_iterator_make_empty(type);
	if (!iter)
		return NULL;

	block_rect rect;
	block_get_actor_rect(actor, &rect);

	block_iterator_add_cell(iter, block_get_pt(rect.x_b, rect.y_a));
	block_iterator_add_cell(iter, block_get_pt(rect.x_a, rect.y_a));
	block_iterator_add_cell(iter, block_get_pt(rect.x_a, rect.y_b));
	block_iterator_add_cell(iter, block_get_pt(rect.x_b, rect.y_b));
	return iter;
}

void block_iterator_add_cell(block_iterator* iterator, block_pt pt)
{
	if (!block_iterator_can_add(iterator))
		SDL_assert(!"Trying to add element already queried iterator!");
	iterator->cell_tree = block_tree_insert(iterator->cell_tree, pt);
}

void block_iterator_free(block_iterator* iterator)
{
	if (iterator)
	{
		block_tree_free(iterator->cell_tree);
		SDL_free(iterator);
	}
}

void* block_iterator_next(block_iterator* iterator)
{
	if (!iterator->tree_cur)
		iterator->tree_cur = iterator->cell_tree;

	skip:
    if (iterator->cur)
    	iterator->cur = iterator->cur->next;
    
    // If we reached the end of the list, find the next one
    while (!iterator->cur)
    {
		iterator->tree_cur = block_tree_next(iterator->tree_cur);
		if (!iterator->tree_cur)
			return NULL;
    	iterator->cur = block_ref_list_get(iterator->tree_cur->pt, iterator->type)->first;
    }
    
    // If we've seen the actor before, skip it
    const block_rect* pts = &iterator->cur->pts;
	block_pt pt = iterator->tree_cur->pt;
    
	// Check all points and check if we've seen them before
	block_pt ne = block_get_pt(pts->x_b, pts->y_a);
	block_pt nw = block_get_pt(pts->x_a, pts->y_a);
	block_pt sw = block_get_pt(pts->x_a, pts->y_b);
	block_pt se = block_get_pt(pts->x_b, pts->y_b);

	// Early exit, this line is technically redundant
	if (block_pt_eq(pt, nw))
		return iterator->cur->reference;

	if (block_pt_eq(pt, ne))
	{
		if (!block_pt_eq(nw, ne) && block_tree_contains(iterator->cell_tree, nw))
			goto skip;
	}
    
	if (block_pt_eq(pt, sw))
	{
		if (!block_pt_eq(nw, sw) && block_tree_contains(iterator->cell_tree, nw))
			goto skip;
		if (!block_pt_eq(ne, sw) && block_tree_contains(iterator->cell_tree, ne))
			goto skip;
	}

	if (block_pt_eq(pt, se))
	{
		if (!block_pt_eq(nw, se) && block_tree_contains(iterator->cell_tree, nw))
			goto skip;
		if (!block_pt_eq(ne, se) && block_tree_contains(iterator->cell_tree, ne))
			goto skip;
		if (!block_pt_eq(sw, se) && block_tree_contains(iterator->cell_tree, sw))
			goto skip;
	}
    
    return iterator->cur->reference;
} 

static bool block_iterator_can_add(const block_iterator* iterator)
{
	return !iterator->tree_cur;
}
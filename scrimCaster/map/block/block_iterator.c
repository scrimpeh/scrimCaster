#include <map/block/block_iterator.h>

#include <map/map.h>

block_iterator* block_iterator_make_empty()
{
	block_iterator* iter = SDL_malloc(sizeof(block_iterator));
	if (!iter)
		return NULL;
	iter->cell_tree = NULL;
	iter->tree_cur = NULL;
	iter->cur = NULL;
	iter->i = 0;
	iter->size = 0;
	return iter;
}

block_iterator* block_iterator_make_pt(i64 wx, i64 wy)
{
	block_iterator* iter = block_iterator_make_empty();
	if (!iter)
		return NULL;

	block_pt pt;
	pt.x = wx / M_CELLSIZE;
	pt.y = wy / M_CELLSIZE;
	block_iterator_add_cell(iter, pt);
	return iter;
}

block_iterator* block_iterator_make_actor(const ac_actor* actor)
{
	block_iterator* iter = block_iterator_make_empty();
	if (!iter)
		return NULL;

	block_pt pts[4];
	block_set_actor_points(actor, pts);
	block_iterator_add_cell(iter, pts[0]);
	block_iterator_add_cell(iter, pts[1]);
	block_iterator_add_cell(iter, pts[2]);
	block_iterator_add_cell(iter, pts[3]);
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
	block_tree_free(iterator->cell_tree);
	SDL_free(iterator);
}

block_ref_list_entry* block_iterator_next(block_iterator* iterator)
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
    	iterator->cur = block_ref_list_get(iterator->tree_cur->pt)->first;
    }
    
    // If we've seen the actor before, skip it
    const block_reference* cur = &iterator->cur->entry;
	block_pt pt = iterator->tree_cur->pt;
    
	// Check all points and check if we've seen them before
	// 
	// Early exit, this line is technically redundant
	if (block_pt_eq(pt, cur->nw))
		return iterator->cur;

	if (block_pt_eq(pt, cur->ne))
	{
		if (!block_pt_eq(cur->nw, cur->ne) && block_tree_contains(iterator->cell_tree, cur->nw))
			goto skip;
	}
    
	if (block_pt_eq(pt, cur->sw))
	{
		if (!block_pt_eq(cur->nw, cur->sw) && block_tree_contains(iterator->cell_tree, cur->nw))
			goto skip;
		if (!block_pt_eq(cur->ne, cur->sw) && block_tree_contains(iterator->cell_tree, cur->ne))
			goto skip;
	}

	if (block_pt_eq(pt, cur->se))
	{
		if (!block_pt_eq(cur->nw, cur->se) && block_tree_contains(iterator->cell_tree, cur->nw))
			goto skip;
		if (!block_pt_eq(cur->ne, cur->se) && block_tree_contains(iterator->cell_tree, cur->ne))
			goto skip;
		if (!block_pt_eq(cur->sw, cur->se) && block_tree_contains(iterator->cell_tree, cur->sw))
			goto skip;
	}
    
    return iterator->cur;
} 

static bool block_iterator_can_add(const block_iterator* iterator)
{
	return !iterator->tree_cur;
}
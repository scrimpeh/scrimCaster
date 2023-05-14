#include <map/block/block_iterator.h>

#include <map/map.h>

// TODO: Arbitrary tile grid retrieval using a Balanced BST
// Write used blockmap entries in list for faster clearing

block_iterator* block_iterator_make_empty()
{
	block_iterator* iter = SDL_malloc(sizeof(block_iterator));
	if (!iter)
		return NULL;
	iter->cell_tree = NULL;
	iter->cells_sorted = NULL;
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
	pt.x = wx;
	pt.y = wy;
	block_iterator_add_cell(iter, &pt);
	return iter;
}

block_iterator* block_iterator_make_actor(const ac_actor* actor)
{
	block_iterator* iter = block_iterator_make_empty();
	if (!iter)
		return NULL;

	block_pt pts[4];
	block_set_actor_points(actor, pts);
	block_iterator_add_cell(iter, &pts[0]);
	block_iterator_add_cell(iter, &pts[1]);
	block_iterator_add_cell(iter, &pts[2]);
	block_iterator_add_cell(iter, &pts[3]);
	return iter;
}

void block_iterator_add_cell(block_iterator* iterator, block_pt* pt)
{
	if (!block_iterator_can_add(iterator))
		SDL_assert(!"Trying to add element already queried iterator!");
	iterator->cell_tree = block_tree_add(iterator->cell_tree, pt->x, pt->y);
}

void block_iterator_free(block_iterator* iterator)
{
	SDL_free(iterator->cells_sorted);
	block_tree_free(iterator->cell_tree);
	SDL_free(iterator);
}

block_ref_list_entry* block_iterator_next(block_iterator* iterator)
{
	// Enumerate all elements once in in-order to have them in a flat list
	// This is not necessarily efficient, but it'll do for now
	if (!iterator->cells_sorted)
	{
		iterator->size = block_tree_size(iterator->cell_tree);
		iterator->cells_sorted = SDL_malloc(sizeof(block_pt) * iterator->size);
		block_tree_nodes(iterator->cell_tree, iterator->cells_sorted);
	}

	return NULL;

	// Get next entry
	//skip:
	//if (iterator->cur)
	//	iterator->cur = iterator->cur->next;
	//
	//// If we reached the end of the list, find the next one
	//while (!iterator->cur)
	//{
	//	if (iterator->mpos_cur.x == iterator->mx_b)
	//	{
	//		if (iterator->mpos_cur.y == iterator->my_b)
	//			return NULL;
	//		iterator->mpos_cur.x = iterator->mx_a;
	//		iterator->mpos_cur.y++;
	//	}
	//	else
	//		iterator->mpos_cur.x++;
	//	iterator->cur = block_ref_list_get(iterator->mpos_cur)->first;
	//}
	//
	//// If we've seen the actor before, skip it
	//const block_reference* cur = &iterator->cur->entry;
	//
	//if (block_pt_eq(&iterator->mpos_cur, &cur->ne) && !block_pt_eq(&cur->nw, &cur->ne))
	//	if (iterator->mpos_cur.x > iterator->mx_a)
	//		goto skip;
	//
	//if (block_pt_eq(&iterator->mpos_cur, &cur->sw) && !block_pt_eq(&cur->nw, &cur->sw))
	//	if (iterator->mpos_cur.y > iterator->my_a)
	//		goto skip;
	//
	//if (block_pt_eq(&iterator->mpos_cur, &cur->se) && !block_pt_eq(&cur->nw, &cur->se))
	//	if (iterator->mpos_cur.y > iterator->my_a || iterator->mpos_cur.x > iterator->mx_a)
	//		goto skip;
	//
	//return iterator->cur;
} 

static bool block_iterator_can_add(const block_iterator* iterator)
{
	// If we linearized the cell tree already, no more nodes can be added
	return !iterator->cells_sorted;
}
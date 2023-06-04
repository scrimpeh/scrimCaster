#include <debugutil/watch.h>

rd_watch_list* watch_watches = NULL;
static i32 watch_id = 1;

i32 watch_add_new(u32 count, ...)
{
	if (count >= RD_WATCH_VALS_MAX)
		return -1;

	const u32 argc = count * 3;
	va_list args;
	va_start(args, count);

	rd_watch new_watch = { 0 };
	new_watch.count = count;
	new_watch.id = watch_id++;
	
	for (u32 i = 0; i < count; ++i)
	{
		new_watch.vals[i].type = va_arg(args, rd_watch_type);
		new_watch.vals[i].fmt = va_arg(args, const char*);
		new_watch.vals[i].val = va_arg(args, void*);
	}
	va_end(args);

	if (!watch_check_unique(&new_watch))
		return -2;

	rd_watch_list* list = SDL_malloc(sizeof(rd_watch_list));
	list->content = new_watch;
	list->next = NULL;

	rd_watch_list* cur = watch_watches;
	if (!cur)
		watch_watches = list;
	else
	{
		while (cur->next)
			cur = cur->next;
		cur->next = list;
	}	

	return watch_id;
}

static bool watch_check_unique(const rd_watch* watch)
{
	rd_watch_list* cur = watch_watches;
	while (cur)
	{
		if (watch_equals(watch, &cur->content))
			return false;
		cur = cur->next;
	}
	return true;
}

static bool watch_equals(const rd_watch* a, const rd_watch* b)
{
	if (a->count != b->count)
		return false;
	for (i8 i = 0; i < a->count; ++i)
	{
		const rd_watch_val* a_val = &a->vals[i];
		const rd_watch_val* b_val = &b->vals[i];

		if (a_val->val != b_val->val)
			return false;
		if (a_val->fmt != b_val->fmt)
			return false;
		if (SDL_strcmp(a_val->fmt, b_val->fmt))
			return false;
	}

	return true;
}

i32 watch_remove(u32 id)
{
	rd_watch_list* cur = watch_watches;
	rd_watch_list* prev = NULL;
	while (cur)
	{
		if (cur->content.id == id)
		{
			if (prev)
				prev->next = cur->next;
			else
				watch_watches = cur->next;
			SDL_free(cur);

			return 0;
		}

		prev = cur;
		cur = cur->next;
	}
	return -1;
}

void watch_clear_all()
{
	rd_watch_list* cur = watch_watches;
	rd_watch_list* to_free;
	while (cur)
	{
		to_free = cur;
		cur = cur->next;
		SDL_free(to_free);
	}
	watch_watches = NULL;
}
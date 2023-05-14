#include <resource.h>

static resource_scope resource_scope_current = RESOURCE_SCOPE_NONE;

static resource_data_list* resources_registry[RESOURCE_SCOPE_MAX] = { 0 };

i32 resources_load(resource_scope scope)
{
	if (scope < resource_scope_current)
	{
		const char* scope_fmt_current = resources_fmt_scope(resource_scope_current);
		const char* scope_fmt_new = resources_fmt_scope(scope);
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "Wrong resource scope initialized! %s -> %s", scope_fmt_current, scope_fmt_new);
		return -1;
	}

	resource_scope_current = scope;

	SDL_Log("Reached resource level: %s", resources_fmt_scope(scope));
	return 0;
}

i32 resources_free(resource_scope scope)
{
	if (scope > resource_scope_current)
	{
		const char* scope_fmt_current = resources_fmt_scope(resource_scope_current);
		const char* scope_fmt_new = resources_fmt_scope(scope);
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "Wrong resource scope initialized! %s -> %s", scope_fmt_current, scope_fmt_new);
		return -1;
	}

	while (resource_scope_current > scope)
	{
		resource_data_list* cur = resources_registry[resource_scope_current];
		while (cur)
		{
			switch (cur->type)
			{
			case RESOURCE_MEMORY:  SDL_free(cur->data.memory);                              break;
			case RESOURCE_SURFACE: SDL_FreeSurface(cur->data.surface);                      break;
			case RESOURCE_OTHER:   cur->data.other.callback(cur->data.other.callback_data); break;
			}
			resource_data_list* to_free = cur;
			cur = cur->next;
			SDL_free(cur);
		}
		resources_registry[resource_scope_current] = NULL;
		resource_scope_current--;
	}

	SDL_Log("Reached resource level: %s", resources_fmt_scope(scope));
	return 0;
}

i32 resources_register_memory(void* memory)
{
	resource_data_list* next = resources_register_allocate();
	if (!next)
		return -1;
	next->type = RESOURCE_MEMORY;
	next->data.memory = memory;
	return 0;
}

i32 resources_register_surface(SDL_Surface* surface)
{
	resource_data_list* next = resources_register_allocate();
	if (!next)
		return -1;
	next->type = RESOURCE_SURFACE;
	next->data.surface = surface;
	return 0;
}

i32 resources_register_other(resource_free_callback callback, void* callback_data)
{
	resource_data_list* next = resources_register_allocate();
	if (!next)
		return -1;
	next->type = RESOURCE_OTHER;
	next->data.other.callback = callback;
	next->data.other.callback_data = callback_data;
	return 0;
}

static resource_data_list* resources_register_allocate()
{
	if (resource_scope_current + 1 == RESOURCE_SCOPE_MAX)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Cannot allocate new resources, already at max scope!");
		return NULL;
	}
	resource_data_list* next = SDL_malloc(sizeof(resource_data_list));
	if (!next)
		return NULL;
	next->next = resources_registry[resource_scope_current + 1];
	resources_registry[resource_scope_current + 1] = next;
	return next;
}

static const char* resources_fmt_scope(resource_scope scope)
{
	switch (scope)
	{
	case RESOURCE_SCOPE_NONE:        return "None";
	case RESOURCE_SCOPE_APPLICATION: return "Application";
	case RESOURCE_SCOPE_LEVEL:       return "Level";
	case RESOURCE_SCOPE_RENDERER:    return "Renderer";
	default:                         return "Unknown";
	}
}

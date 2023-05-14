#pragma once

#include <common.h>

#include <SDL/SDL_surface.h>

// Here's an idea. To simplify the jungle of symmetric "allocate / free" calls,
// we may simplify the procedure by automatically registering resources for free later

// The idea is to define several nested resource scopes (see below), for which various resources are registered

// When a scope is closed all resources that are registered for the scope are automatically freed again.

// The resource scope. Each scope is nested in the other. Scopes may only be initialized in the order 
// provided and only freed in the reverse order.
typedef enum
{
	RESOURCE_SCOPE_NONE = 0,
	RESOURCE_SCOPE_APPLICATION = 1,
	RESOURCE_SCOPE_LEVEL = 2,
	RESOURCE_SCOPE_RENDERER = 3,
	RESOURCE_SCOPE_MAX
} resource_scope;

// Defines the type of a resource. Used for selecting what kind of resource to free
typedef enum
{
	// Memory, allocated with SDL_malloc and freed with SDL_free
	RESOURCE_MEMORY,
	// A SDL Surface, allocated with SDL_CreateSurface (or other), and freed with SDL_FreeSurface
	RESOURCE_SURFACE,
	// Other. The consumer provides a callback for handling it
	RESOURCE_OTHER
} resource_type;

// Load all registered resources
i32 resources_load(resource_scope scope);
i32 resources_free(resource_scope scope);

typedef void (*resource_free_callback(void* callback_data));

typedef union
{
	void** memory;
	SDL_Surface** surface;
	struct
	{
		resource_free_callback* callback;
		void* callback_data;
	} other;
} resource_data;

typedef struct resource_data_list
{
	resource_type type;
	resource_data data;
	struct resource_data_list* next;
} resource_data_list;

// Registration functions for the next resource scope in preparation

i32 resources_register_memory(void* memory);
i32 resources_register_surface(SDL_Surface* surface);
i32 resources_register_other(resource_free_callback callback, void* callback_data);

static resource_data_list* resources_register_allocate();

static const char* resources_fmt_scope(resource_scope scope);
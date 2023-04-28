#pragma once

// This module is responsible for rendering debug information

#include "common.h"

#include "watch.h"

#include "SDL/SDL_video.h"

void rd_render_debug(SDL_Surface* target);

static void rd_draw_watches(SDL_Surface* target);
static void rd_print_watch_val(char* buf, const rd_watch_val* val);


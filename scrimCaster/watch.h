#pragma once

#include "common.h"

// This module facilities to let other parts of the program watch RAM values
// They are read from RAM

typedef enum
{
	WCH_I8  =  0, WCH_I16 =  1, WCH_I32  = 2, WCH_I64 =  3,
	WCH_U8  =  4, WCH_U16 =  5, WCH_U32  = 6, WCH_U64 =  7,
	WCH_X8  =  9, WCH_X16 = 10, WCH_X32 = 11, WCH_X64 = 12,
	WCH_F32 = 13, WCH_F64 = 14
} rd_watch_type;

typedef struct
{
	rd_watch_type type;
	const char* fmt;
	void* val;
} rd_watch_val;

#define RD_WATCH_VALS_MAX 8
#define RD_WATCH_VAL_STRING_LENGTH_MAX 256

typedef struct
{
	i32 id;
	u8 count;
	rd_watch_val vals[RD_WATCH_VALS_MAX];
} rd_watch;

typedef struct rd_watch_list
{
	rd_watch content;
	struct rd_watch_list* next;
} rd_watch_list;

extern rd_watch_list* rd_watches;

// Creates a debug watch on the screen. Expects triplets of the form type, desc, ptr, as arguments.
// Returns the id of the watch created, or a negative error code
i32 watch_add_new(u32 count, ...);

// Removes watch by id, returns 0 if the watch could be remove and -1 otherwise
i32 watch_remove(u32 id);

// Removes all watches
void watch_clear_all();

static bool watch_check_unique(const rd_watch* watch);
static bool watch_equals(const rd_watch* a, const rd_watch* b);
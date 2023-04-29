#include "renderdebug.h"

#include "colormap.h"
#include "renderutil.h"
#include "ttf.h"

#include "SDL/SDL_ttf.h"

#define RD_BUF_SIZE 1024
#define RD_BUF_VAL_SIZE 256

static char rd_buf[RD_BUF_SIZE];
static char rd_val_buf[RD_BUF_VAL_SIZE];

extern TTF_Font* ttf_font_debug;


void rd_render_debug(SDL_Surface* target)
{
	rd_draw_watches(target);
}

static void rd_draw_watches(SDL_Surface* target)
{
	const rd_watch_list* cur = rd_watches;
	i16 y = 2;
	while (cur)
	{
		rd_buf[0] = '\0';
		u64 pos = 0;
		const rd_watch* watch = &cur->content;
		for (u8 i = 0; i < watch->count; i++)
		{
			const rd_watch_val* val = &watch->vals[i];
			pos = SDL_strlcat(rd_buf + pos, val->fmt, RD_BUF_SIZE - pos);
			if (pos >= RD_BUF_SIZE)
				break;
			rd_print_watch_val(rd_val_buf, val);
			pos = SDL_strlcat(rd_buf + pos, rd_val_buf, RD_BUF_SIZE - pos);
			if (pos >= RD_BUF_SIZE)
				break;
		}
		r_draw_text(target, rd_buf, 2, y, ttf_font_debug, CM_GET(0xFF, 0xFF, 0xFF));
		y += TTF_FontHeight(ttf_font_debug) + 2;
		cur = cur->next;
	}
}

static void rd_print_watch_val(char* dest, const rd_watch_val* val)
{
	switch (val->type)
	{
	case WCH_I8:  SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%hhi", *(i8*)(val->val));     break;
	case WCH_I16: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%hi", *(i16*)(val->val));    break;
	case WCH_I32: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%i", *(i32*)(val->val));    break;
	case WCH_I64: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%lli", *(i64*)(val->val));    break;
	case WCH_U8:  SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%hhu", *(u8*)(val->val));     break;
	case WCH_U16: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%hu", *(u16*)(val->val));    break;
	case WCH_U32: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%u", *(u32*)(val->val));    break;
	case WCH_U64: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%llu", *(u64*)(val->val));    break;
	case WCH_X8:  SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%hhX", *(u8*)(val->val));     break;
	case WCH_X16: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%hX", *(u16*)(val->val));    break;
	case WCH_X32: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%X", *(u32*)(val->val));    break;
	case WCH_X64: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%llX", *(u64*)(val->val));    break;
	case WCH_F32: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%.2f", *(float*)(val->val));	 break;
	case WCH_F64: SDL_snprintf(dest, RD_BUF_VAL_SIZE, "%.2f", *(double*)(val->val)); break;
	default: SDL_assert(!"Invalid watch type!");
	}
}
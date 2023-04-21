#include "types.h"
#include "render.h"
#include "SDL/SDL_render.h"
#include "SDL/SDL_ttf.h"
#include "scan.h"
#include "sprite.h"
#include "map.h"
#include "maputil.h"
#include "actor.h"
#include "input.h"
#include "renderconstants.h"


//Basic rendering pipeline:
// 1.Draw floor and ceiling/sky:
// can be as complicated or simple as needed
// 2.Figure out what walls to draw and draw them
// 3.Draw World Sprites
// 4.Draw Hud Sprites 

//note that these are decoupled from the window width and height
//it should only be the case that these are smaller or equal to the window sizes
//as otherwise, space gets wasted
u16 viewport_w, viewport_h;

extern i32 displayWidth;
extern i32 displayHeight;

extern TTF_Font* fontDebug;

extern u8 viewport_x_fov;

extern SDL_Window* mainWindow;
SDL_Surface* viewportSurface = NULL;
SDL_Surface* mainWindowSurface = NULL;

extern u64 framecounter;
extern float framerate;

extern const char* app_dir;

extern Actor player;
extern Map map;
extern ActorVector levelEnemies;

extern u32* sprite_buffer;

bool show_map = false;
bool draw_crosshair = true;
extern Input input;

i32 InitializeRenderer(u16 w, u16 h, u8 col)
{
	CloseRenderer();	//close the old renderer first

	viewport_w = w;
	viewport_h = h;

	viewportSurface = SDL_CreateRGBSurface(0, viewport_w, viewport_h, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000);
	if (!viewportSurface)
	{
		SDL_LogError(SDL_LOG_PRIORITY_CRITICAL, 
			"Couldn't initialize viewport! %s", SDL_GetError());
		return -1;
	}

	if (InitializeScan(col))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,
			"Couldn't initialize map renderer! %s",
			SDL_GetError());
		return -1;
	}

	sprite_buffer = (u32*)SDL_malloc(sizeof(u32) * viewport_h);
	if (!sprite_buffer)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR,
			"Couldn't initialize sprite buffer! %s",
			SDL_GetError());
		return -1;
	}

	return 0;
}

void CloseRenderer()
{
	CloseScan();
	SDL_free(sprite_buffer);
	sprite_buffer = NULL;
	SDL_FreeSurface(viewportSurface);
	viewportSurface = NULL;
}

//Temporary function, to quickly get something visible on screen
//should be replaced by a proper solution
void TextOut(SDL_Surface* surf, const char* str, i32 x, i32 y)
{
	if (!fontDebug) 
		return;

	SDL_Rect r = { x, y, 0, 0 };

	SDL_Surface* msg = TTF_RenderText_Solid(fontDebug, str, { 0xFF, 0xFF, 0xFF, 0xFF });	
	SDL_BlitSurface(msg, NULL, surf, &r);
	SDL_FreeSurface(msg);
}

//Drawing primitives
inline void DrawPixel(const SDL_Surface* const surf, const SDL_Point p, const u32 col)
{
	*((u32*)surf->pixels + p.y * surf->w + p.x) = col;
};

inline void DrawPixelBounds(const SDL_Surface* const surf, const SDL_Point p, const u32 col)
{
	if (p.x >= 0 && p.x < surf->w && p.y >= 0 && p.y < surf->h)
		DrawPixel(surf, p, col);
};

//Draw a line from p1 to p2 in the color supplied
void DrawLine(const SDL_Surface* const surf, const SDL_Point p1, const SDL_Point p2, const u32 col)
{
	float x1 = (float)p1.x, x2 = (float)p2.x, y1 = (float)p1.y, y2 = (float)p2.y;
	float temp;

	const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
	//Swap x1 and y1, swap x2 and y2
	if (steep)
	{
		temp = y1;	
		y1 = x1;
		x1 = temp;

		temp = y2;
		y2 = x2;
		x2 = temp;
	}

	//Swap x1 and x2, swap y1 and y2
	if (x1 > x2)	
	{
		temp = x2;	
		x2 = x1;
		x1 = temp;

		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	const float dx = x2 - x1;
	const float dy = fabsf(y2 - y1);

	float err = dx / 2.0f;
	const i32 y_step = (y1 < y2) ? 1 : -1;

	i32 y = (i32)y1;
	const i32 x_max = (i32)x2;
	for (i32 x = (i32)x1; x < x_max; ++x)
	{
		const SDL_Point p_draw = { steep ? y : x, steep ? x : y };
		DrawPixel(surf, p_draw, col);
		err -= dy;
		if (err < 0)
		{
			y += y_step;
			err += dx;
		}
	}
};

//clip the line first if any of the coordinates are out of bounds
void DrawLineBounds(const SDL_Surface* const surf, const SDL_Point p1, const SDL_Point p2, const u32 col)
{
}

void DrawMap(SDL_Surface* surface)
{
	static const u16 TILESIZE = 32;
	static const u8 OFFSET = 24;
	u8 n;
	SDL_Rect tileRect[4];
	tileRect[0].y = OFFSET;
	tileRect[1].y = OFFSET;
	tileRect[2].y = OFFSET;
	tileRect[3].y = OFFSET + TILESIZE - 1;

	tileRect[0].h = TILESIZE;	//East
	tileRect[0].w = 1;
	tileRect[1].h = 1;	//North
	tileRect[1].w = TILESIZE;
	tileRect[2].h = TILESIZE;	//West
	tileRect[2].w = 1;
	tileRect[3].h = 1;	//South
	tileRect[3].w = TILESIZE;
	Cell* cellptr = map.cells;
	for (u8 i = 0; i < map.boundsY; ++i)
	{
		tileRect[0].x = OFFSET + TILESIZE - 1;
		tileRect[1].x = OFFSET;
		tileRect[2].x = OFFSET;
		tileRect[3].x = OFFSET;
		for (u8 j = 0; j < map.boundsX; ++j)
		{
			for (n = 0; n < 4; ++n)
				if (cellptr->sides[n].type)
					SDL_FillRect(surface, &tileRect[n], 0x00FFFF00 | n << 6);
			for (n = 0; n < 4; ++n) tileRect[n].x += TILESIZE;
			++cellptr;
		}
		for (n = 0; n < 4; ++n) tileRect[n].y += TILESIZE;
	}

	//Draw the player
	SDL_Rect r;
	const u16 div = CELLSIZE / TILESIZE;
	const Bounds playerBounds = GetActorBounds(PLAYER);
	r.h = 2 * i32(playerBounds.x) / div;
	r.w = 2 * i32(playerBounds.y) / div;
	r.x = i32(OFFSET + (player.x / div) - r.h / 2);
	r.y = i32(OFFSET + (player.y / div) - r.w / 2);

	SideCoords s1, s2;
	SDL_Point edge1, edge2;
	double angle1 = player.angle - (viewport_x_fov / 2);
	double angle2 = player.angle + (viewport_x_fov / 2);
	if (angle1 < 0) angle1 += 360;
	if (angle2 >= 360) angle2 -= 360;
	if (IntersectWall(&s1, { player.x, player.y }, angle1, VisibilitySolid)
		&& IntersectWall(&s2, { player.x, player.y }, angle2, VisibilitySolid))
	{
		edge1 = { (i32)s1.p_x, (i32)s1.p_y };
		edge2 = { (i32)s2.p_x, (i32)s2.p_y };

		SDL_Point start = { i32(OFFSET + (player.x / div)), i32(OFFSET + (player.y / div)) };
		edge1.x /= div;
		edge1.x += OFFSET;
		edge1.y /= div;
		edge1.y += OFFSET;
		edge2.x /= div;
		edge2.x += OFFSET;
		edge2.y /= div;
		edge2.y += OFFSET;

		DrawLine(surface, start, edge1, 0xFFFFFF);
		DrawLine(surface, start, edge2, 0xFFFFFF);
	}
	
	SDL_FillRect(surface, &r, 0xFFFF00);

	const ActorArray objs = map.levelObjs;
	for (u32 i = 0; i < objs.count; ++i)
	{
		const Actor a = objs.actor[i];
		const Bounds b = GetLevelObjBounds(a.actor_index);
		SDL_Rect r;
		r.h = i32(2 * b.y) / div;
		r.w = i32(2 * b.x) / div;
		r.x = OFFSET + i32(a.x / div) - r.h / 2;
		r.y = OFFSET + i32(a.y / div) - r.h / 2;
		SDL_FillRect(surface, &r, 0xFF00FF);
	}
	for (u32 i = 0; i < levelEnemies.count; ++i)
	{
		const Actor a = *levelEnemies.content[i];
		const Bounds b = GetLevelObjBounds(a.actor_index);
		SDL_Rect r;
		r.h = i32(2 * b.y) / div;
		r.w = i32(2 * b.x) / div;
		r.x = OFFSET + i32(a.x / div) - r.h / 2;
		r.y = OFFSET + i32(a.y / div) - r.h / 2;
		SDL_FillRect(surface, &r, 0xFF0000);
	}
	
}

void DrawCrosshair(SDL_Surface* toDraw)
{
	const SDL_Point p1 = { toDraw->w / 2, toDraw->h / 2 - 8 };
	const SDL_Point p2 = { toDraw->w / 2, toDraw->h / 2 + 8 };
	const SDL_Point p3 = { toDraw->w / 2 - 8, toDraw->h / 2 };
	const SDL_Point p4 = { toDraw->w / 2 + 8, toDraw->h / 2 };

	DrawLine(toDraw, p1, p2, 0xFFFFFF);
	DrawLine(toDraw, p3, p4, 0xFFFFFF);
}

void RenderFrame()
{
	//Do the rendering pipeline
	show_map = input.map != 0;

	// Render ceiling and floor
	SDL_Rect upper = { 0, 0, viewport_w, viewport_h / 2 };
	SDL_Rect lower = { 0, viewport_h / 2, viewport_w, viewport_h / 2 };

	SDL_FillRect(viewportSurface, &upper, 0x00A8A8C8);
	SDL_FillRect(viewportSurface, &lower, 0x00C0C0C0);

	DrawGeometry(viewportSurface);
	DrawSprites(viewportSurface);
	if (show_map)
		DrawMap(viewportSurface);
	else if (draw_crosshair) 
		DrawCrosshair(viewportSurface);
	
	
	char buf[256];
	SDL_memset(buf, 0, 256);
	SDL_snprintf(buf, 256, "FPS: %.2f | Framecount: %d", framerate, framecounter);
	TextOut(viewportSurface, buf, 4, 2);

	SDL_BlitScaled(viewportSurface, NULL, mainWindowSurface, NULL);
	
	SDL_UpdateWindowSurface(mainWindow);
}
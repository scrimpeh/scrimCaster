#include <render/lighting/smoothlight.h>

#include <map/map.h>
#include <util/mathutil.h>

static r_light_smooth_cell_brightness* r_cell_lights = NULL;

i32 r_light_smooth_init()
{
	r_cell_lights = SDL_malloc(m_map.w * m_map.h * sizeof(r_light_smooth_cell_brightness));
	if (!r_cell_lights)
		return -1;

	for (i32 y = 0; y < m_map.h; y++)
	{
		for (i32 x = 0; x < m_map.w; x++)
		{
			r_light_smooth_cell_brightness* current = &r_cell_lights[y * m_map.w + x];
			current->ne = 255;
			current->nw = 255;
			current->sw = 255;
			current->se = 255;

			if (y == 0 && x == m_map.w - 1)
				current->ne = m_get_cell(x, y)->brightness;
			else if (y == 0 && x < m_map.w - 1)
				current->ne = r_light_smooth_brightness_2(m_get_cell(x, y), m_get_cell(x + 1, y), M_EAST);
			else if (y > 0 && x == m_map.w - 1)
				current->ne = r_light_smooth_brightness_2(m_get_cell(x, y), m_get_cell(x, y - 1), M_NORTH);
			else
				current->ne = r_light_smooth_brightness_4(m_get_cell(x, y), m_get_cell(x + 1, y), m_get_cell(x + 1, y - 1), m_get_cell(x, y - 1), M_EAST);


			if (y == 0 && x == 0)
				current->nw = m_get_cell(x, y)->brightness;
			else if (y > 0 && x == 0)
				current->nw = r_light_smooth_brightness_2(m_get_cell(x, y), m_get_cell(x, y - 1), M_NORTH);
			else if (y == 0 && x > 0)
				current->nw = r_light_smooth_brightness_2(m_get_cell(x, y), m_get_cell(x - 1, y), M_WEST);
			else
				current->nw = r_light_smooth_brightness_4(m_get_cell(x, y), m_get_cell(x, y - 1), m_get_cell(x - 1, y - 1), m_get_cell(x - 1, y), M_NORTH);


			if (y == m_map.h - 1 && x == 0)
				current->sw = m_get_cell(x, y)->brightness;
			else if (y == m_map.h - 1 && x > 0)
				current->sw = r_light_smooth_brightness_2(m_get_cell(x, y), m_get_cell(x - 1, y), M_WEST);
			else if (y < m_map.h - 1 && x == 0)
				current->sw = r_light_smooth_brightness_2(m_get_cell(x, y), m_get_cell(x, y + 1), M_SOUTH);
			else
				current->sw = r_light_smooth_brightness_4(m_get_cell(x, y), m_get_cell(x - 1, y), m_get_cell(x - 1, y + 1), m_get_cell(x, y + 1), M_WEST);


			if (y == m_map.h - 1 && x == m_map.w - 1)
				current->se = m_get_cell(x, y)->brightness;
			else if (y < m_map.h - 1 && x == m_map.w - 1)
				current->se = r_light_smooth_brightness_2(m_get_cell(x, y), m_get_cell(x, y + 1), M_SOUTH);
			else if (y == m_map.h - 1 && x < m_map.w - 1)
				current->se = r_light_smooth_brightness_2(m_get_cell(x, y), m_get_cell(x + 1, y), M_EAST);
			else
				current->se = r_light_smooth_brightness_4(m_get_cell(x, y), m_get_cell(x, y + 1), m_get_cell(x + 1, y + 1), m_get_cell(x + 1, y), M_SOUTH);
		}
	}

	return 0;
}

void r_light_smooth_destroy()
{
	SDL_free(r_cell_lights);
	r_cell_lights = NULL;
}

static u8 r_light_smooth_brightness_2(const m_cell* curr, const m_cell* other, m_orientation orientation)
{
	const m_side* side_curr = m_cell_get_side(curr, orientation);
	if (r_light_smooth_block(side_curr))
		return curr->brightness;
	else
		return (curr->brightness + other->brightness) / 2;

}

static u8 r_light_smooth_brightness_4(const m_cell* a, const m_cell* b, const m_cell* c, const m_cell* d, m_orientation next)
{
	const m_side* side_a[2];
	const m_side* side_b[2];
	const m_side* side_c[2];
	const m_side* side_d[2];

	switch (next)
	{
	case M_EAST:
		side_a[0] = &a->e;
		side_b[0] = &b->n;
		side_c[0] = &c->w;
		side_d[0] = &d->s;
		side_a[1] = &b->w;
		side_b[1] = &c->s;
		side_c[1] = &d->e;
		side_d[1] = &a->n;
		break;
	case M_NORTH:
		side_a[0] = &a->n;
		side_b[0] = &b->w;
		side_c[0] = &c->s;
		side_d[0] = &d->e;
		side_a[1] = &b->s;
		side_b[1] = &c->e;
		side_c[1] = &d->n;
		side_d[1] = &a->w;
		break;
	case M_WEST:
		side_a[0] = &a->w;
		side_b[0] = &b->s;
		side_c[0] = &c->e;
		side_d[0] = &d->n;
		side_a[1] = &b->e;
		side_b[1] = &c->n;
		side_c[1] = &d->w;
		side_d[1] = &a->s;
		break;
	case M_SOUTH:
		side_a[0] = &a->s;
		side_b[0] = &b->e;
		side_c[0] = &c->n;
		side_d[0] = &d->w;
		side_a[1] = &b->n;
		side_b[1] = &c->w;
		side_c[1] = &d->s;
		side_d[1] = &a->e;
		break;
	}

	// There's 4 sides which can be blocking or not -> 16 cases
	const u8 light =
		r_light_smooth_block(side_a[0])      | r_light_smooth_block(side_a[1])      |
		r_light_smooth_block(side_b[0]) << 1 | r_light_smooth_block(side_b[1]) << 1 |
		r_light_smooth_block(side_c[0]) << 2 | r_light_smooth_block(side_c[1]) << 2 |
		r_light_smooth_block(side_d[0]) << 3 | r_light_smooth_block(side_d[1]) << 3;

	
	switch (light)
	{
	case 0x0:
		// d   c
		//  
		// a   b
		return (a->brightness + b->brightness + c->brightness + d->brightness) / 4;
	case 0x1:
		// d   c
		//   +
		// a | b
		return (a->brightness + b->brightness + c->brightness + d->brightness) / 4;
	case 0x2:
		// d   c
		//   +--
		// a   b
		return (a->brightness + b->brightness + c->brightness + d->brightness) / 4;
	case 0x3:
		// d   c
		//   +--
		// a | b
		return (a->brightness + c->brightness + d->brightness) / 3;
	case 0x4:
		// d | c
		//   +  
		// a   b
		return (a->brightness + c->brightness + d->brightness) / 3;
	case 0x5:
		// d | c
		//   +  
		// a | b
		return (a->brightness + d->brightness) / 2;
	case 0x6:
		// d | c
		//   +--
		// a   b
		return (a->brightness + b->brightness + d->brightness) / 3;
	case 0x7:
		// d | c
		//   +--
		// a | b
		return (a->brightness + d->brightness) / 2;
	case 0x8:
		// d   c
		// --+
		// a   b
		return (a->brightness + b->brightness + c->brightness + d->brightness) / 4;
	case 0x9:
		// d   c
		// --+
		// a | b
		return a->brightness;
	case 0xA:
		// d   c
		// --+--
		// a   b
		return (a->brightness + b->brightness) / 2;
	case 0xB:
		// d   c
		// --+--
		// a | b
		return a->brightness;
	case 0xC:
		// d | c
		// --+  
		// a   b
		return (a->brightness + b->brightness + c->brightness) / 3;
	case 0xD:
		// d | c
		// --+  
		// a | b
		return a->brightness;
	case 0xE:
		// d | c
		// --+--
		// a   b
		return (a->brightness + b->brightness) / 2;
	case 0xF:
		// d | c
		// --+--
		// a | b
		return a->brightness;
	}
}

static bool r_light_smooth_block(const m_side* side)
{
	return (side->type && !(side->flags & TRANSLUCENT)) || (side->flags & BLOCK_SMOOTH_LIGHT);
}


u8 r_light_smooth_get_brightness(i32 map_x, i32 map_y, m_orientation orientation, u8 x, u8 y)
{
	u8 cell_x;
	u8 cell_y;
	switch(orientation)
	{
	case M_EAST:
		cell_x = M_CELLSIZE - 1;
		cell_y = x;
		break;
	case M_NORTH:
		cell_x = x;
		cell_y = 0;
		break;
	case M_WEST:
		cell_x = 0;
		cell_y = M_CELLSIZE - x - 1;
		break;
	case M_SOUTH:
		cell_x = M_CELLSIZE - x - 1;
		cell_y = M_CELLSIZE - 1;
		break;
	default:
		cell_x = x;
		cell_y = y;
		break;
	}

	const r_light_smooth_cell_brightness* cell = &r_cell_lights[map_y * m_map.w + map_x];
	// 2-dimensional lerp brightness
	
	//if (cell_x < M_CELLSIZE / 2)
	//	return cell_y < M_CELLSIZE / 2 ? cell->nw : cell->sw;
	//else
	//	return cell_y < M_CELLSIZE / 2 ? cell->ne : cell->se;

	const float x_a = math_lerp(0, M_CELLSIZE - 1, cell_x, cell->nw, cell->ne);
	const float x_b = math_lerp(0, M_CELLSIZE - 1, cell_x, cell->sw, cell->se);
	return (u8) math_lerp(0, M_CELLSIZE - 1, cell_y, x_a, x_b);
}
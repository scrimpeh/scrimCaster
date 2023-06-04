#include <game/actor/trigger/light.h>

#include <map/map.h>
#include <render/lighting/lighting.h>
#include <util/random.h>

bool ac_logic_light_flicker(ac_actor* ac, u32 delta)
{
	const i16 mx = ac->x / M_CELLSIZE;
	const i16 my = ac->y / M_CELLSIZE;
	switch (ac->state)
	{
	case 0:
		ac->state++;
		ac->timer = 500;
		break;
	case 1:
		ac->timer -= delta;
		if (ac->timer < 0)
		{
			ac_logic_light_flicker_set_brightness(ac, 32);
			ac->timer = 32 + (random_rand() & 0x7F);
			ac->state++;
		}
		break;
	case 2:
		ac->timer -= delta;
		if (ac->timer < 0)
		{
			ac_logic_light_flicker_set_brightness(ac, 255);
			ac->state--;
			ac->timer = 1536 + (random_rand() & 0x7FF);
		}
		break;
	}
	return false;
}


static void ac_logic_light_flicker_set_brightness_cell(i16 mx, i16 my, u8 brightness)
{
	if (mx >= 0 && my >= 0 && mx < m_map.w && my < m_map.h)
		m_get_cell(mx, my)->brightness = brightness;
}

static void ac_logic_light_flicker_set_brightness(const ac_actor* ac, u8 brightness)
{
	const i16 mx = ac->x / M_CELLSIZE;
	const i16 my = ac->y / M_CELLSIZE;

	for (i8 y_offs = -1; y_offs <= 1; y_offs++)
		for (i8 x_offs = -1; x_offs <= 1; x_offs++)
			ac_logic_light_flicker_set_brightness_cell(mx + x_offs, my + y_offs, brightness);

	r_light_update(mx - 1, my - 1, mx + 1, my + 1);
}
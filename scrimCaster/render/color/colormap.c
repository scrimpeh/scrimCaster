#include <render/color/colormap.h>

cm_color cm_map(cm_color source, cm_color target, float alpha)
{
	cm_channel r = cm_mix_channel(CM_R(source), CM_R(target), alpha);
	cm_channel g = cm_mix_channel(CM_G(source), CM_G(target), alpha);
	cm_channel b = cm_mix_channel(CM_B(source), CM_B(target), alpha);
	return CM_GET(r, g, b);
}

cm_color cm_from_sdl_color(const SDL_Color* color)
{
	return CM_GET(color->r, color->g, color->b);
}

void cm_to_sdl_color(SDL_Color* color, cm_color source)
{
	color->a = 0xFF;
	color->r = CM_R(source);
	color->g = CM_G(source);
	color->b = CM_B(source);
}

static cm_channel cm_mix_channel(cm_channel source, cm_channel target, float alpha)
{
	return (cm_channel) ((1.f - alpha) * source) + (alpha * target);
}
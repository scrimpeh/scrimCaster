#include "colormap.h"

cm_color cm_map(cm_color source, cm_color target, float alpha)
{
	cm_channel r = cm_mix_channel(CM_R(source), CM_R(target), alpha);
	cm_channel g = cm_mix_channel(CM_G(source), CM_G(target), alpha);
	cm_channel b = cm_mix_channel(CM_B(source), CM_B(target), alpha);
	return CM_GET(r, g, b);
}

static cm_channel cm_mix_channel(cm_channel source, cm_channel target, float alpha)
{
	return (cm_channel)((1. - alpha) * source) + (alpha * target);
}
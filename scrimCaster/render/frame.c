#include <render/frame.h>

#include <SDL/SDL_timer.h>

bool frame_cap_rate = false;
double frame_cap_desired = 60.0;

#define TICKS_PER_FRAME (1000 / frame_cap_desired)

u64 frame_count = 0;
u64 frame_ticks = 0;

void frame_end(u32 ticks_start)
{
	watch_add_new(2, WCH_U64, "framecounter: ", &frame_count, WCH_U64, ", ticks: ", &frame_ticks);

	frame_count++;
	u32 ticks_cur = SDL_GetTicks();
	frame_ticks = ticks_cur - ticks_start;

	if (frame_cap_rate)
		frame_delay(frame_ticks);

	frame_ticks = SDL_GetTicks() - ticks_start;
}

static void frame_delay(u32 diff)
{
	if (diff < TICKS_PER_FRAME)
		SDL_Delay((u32) TICKS_PER_FRAME - diff);
}
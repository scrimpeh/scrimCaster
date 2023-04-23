#include "frame.h"

#include "SDL/SDL_timer.h"

#define TICKS_PER_FRAME (1000 / (double)desiredFramerate)

bool capFramerate = false;
u8 desiredFramerate = 60;

u64 framecounter = 0;
float framerate = 0.f;

void EndFrame(u32 startTicks)
{
	++framecounter;
	u32 curTicks = SDL_GetTicks();
	u32 diff = curTicks - startTicks;

	if (capFramerate)
		WaitForNextFrame(diff);

	diff = SDL_GetTicks() - startTicks;

	framerate = diff ? 1000 / (float)diff : 1000;
}

void WaitForNextFrame(u32 diff)
{
	if (diff < TICKS_PER_FRAME)
		SDL_Delay((u32)TICKS_PER_FRAME - diff);
}
#include "types.h"
#include "timer.h"

void MkTimer(Timer* t, u64 total, bool start, bool repeating)
{
	t->flags = start ? STARTED : PAUSED;
	t->ticks = total;
	t->total = repeating ? total : 0;
}

void StartTimer(Timer* t)
{
	t->flags = STARTED;
	t->ticks = 0;
}

void PauseTimer(Timer* t)
{
	t->flags = PAUSED;
}

void ResumeTimer(Timer* t)
{
	t->flags = STARTED;
}

void ToggleTimer(Timer* t)	//Pause or resume
{
	if (t->flags == PAUSED) t->flags = STARTED;
	else if (t->flags == STARTED) t->flags = PAUSED;
}

void StopTimer(Timer* t)
{
	t->flags = CLOSED;
}

void Tick(Timer* t)
{
	if (t->flags == STARTED)
		if (--t->ticks == -1)
		{
			t->ticks = t->total;
			if (!t->total)
				t->flags = FINISHED;
		}
}

void DestroyTimer(Timer* t)
{
	t->ticks = 0;
	t->flags = CLOSED;
	t->total = 0;
}
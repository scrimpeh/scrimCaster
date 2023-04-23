#pragma once

#include "common.h"

typedef enum TimerFlags : u32
{
	CLOSED = 0,
	STARTED = 1,
	PAUSED = 2,
	FINISHED = 3
} TimerFlags;

typedef struct Timer
{
	TimerFlags flags;	//0 if if closed, 1 if started, 2 if paused.
	u64 total;			//0 if one time, non-zero if periodic
	u64 ticks;			//How many ticks to count
} Timer;

typedef struct Ticker
{
	TimerFlags flags;	//0 if if closed, 1 if started, 2 if paused.
	u64 ticks;			//Temporary tick holder
	u64 incrementBy;		//How many ticks to increment by
} Ticker;

void MkTimer(Timer* t, u64 total, bool start, bool repeating);
void StartTimer(Timer* t);
void PauseTimer(Timer* t);
void ResumeTimer(Timer* t);
void ToggleTimer(Timer* t);	//Pause or resume
void StopTimer(Timer* t);
void Tick(Timer* t);
void DestroyTimer(Timer* t);
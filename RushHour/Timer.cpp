#include "stdafx.h"
#include "Timer.h"


Timer::Timer() {
}


Timer::~Timer() {
}

void Timer::StartTimer() {
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	_countsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	_counterStart = frequencyCount.QuadPart;
}

double Timer::GetTime() {
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - _counterStart) / _countsPerSecond;
}

double Timer::GetFrameTime() {
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - _frameTimeOld;
	_frameTimeOld = currentTime.QuadPart;

	if (tickCount < 0)
		tickCount = 0;

	return float(tickCount) / _countsPerSecond;
}
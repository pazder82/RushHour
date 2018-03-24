#pragma once
class Timer {
public:
	Timer();
	~Timer();

	void StartTimer(); // Reset Timer
	double GetTime(); // Return num of seconds since last StartTimer() call
	double GetFrameTime(); // Return num of seconds sincse last GetFrameTime() call

private:
	__int64 _counterStart = 0;
	double _countsPerSecond = 0.0;
	__int64 _frameTimeOld = 0;
	double _frameTime;
};


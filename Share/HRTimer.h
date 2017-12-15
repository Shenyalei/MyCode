#ifndef SHARE_HR_TIMER_
#define SHARE_HR_TIMER_

class HRtimer
{
public:
	static LONG64 GetFrequency();
	void Start();
	void Stop();
	LARGE_INTEGER start;
	LARGE_INTEGER stop;
	static LONG64 frequency;
};

class ScopeHRtimer
{
public:
	ScopeHRtimer() { timer.Start(); }
	~ScopeHRtimer() { timer.Stop(); }
	HRtimer timer;
};
#endif // SHARE_HR_TIMER_
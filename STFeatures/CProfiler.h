#ifndef __CPROFILER_H__
#define __CPROFILER_H__

/*
double t = (double)getTickCount();
// do something ...
t = ((double)getTickCount() - t)/getTickFrequency();
*/

class CProfiler
{
	private:
		double _tickStart;
		double _tickStop;

	public:
		CProfiler();
		~CProfiler();
		void start();
		void stop();
		double getSeconds();
		double getMilliSeconds();
};

#endif
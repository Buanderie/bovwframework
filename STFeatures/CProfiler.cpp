#include <opencv2\core\core.hpp>

#include "CProfiler.h"

#define PROF CProfiler

using namespace cv;

PROF::PROF()
{

}

PROF::~PROF()
{

}

void PROF::start()
{
	_tickStart = (double)getTickCount();
}

void PROF::stop()
{
	_tickStop = (double)getTickCount();
}

double PROF::getSeconds()
{
	return (_tickStop - _tickStart)/getTickFrequency();
}

double PROF::getMilliSeconds()
{
	return ((_tickStop - _tickStart)/getTickFrequency())*1000;
}

#undef PROF
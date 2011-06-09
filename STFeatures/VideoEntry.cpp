#include "VideoEntry.h"

#include <vector>
#include <string>

#define ENTRY CVideoEntry

using namespace std;

ENTRY::ENTRY( std::string videoFileName )
{
	_videoFileName = videoFileName;
}

ENTRY::ENTRY( std::string videoFileName, std::string className )
{
	_videoFileName = videoFileName;
	_className = className;
}

ENTRY::~ENTRY()
{

}

#undef ENTRY
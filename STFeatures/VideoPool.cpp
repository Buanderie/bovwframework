#include "VideoPool.h"

#include <vector>
#include <string>

#include "VideoEntry.h"

#define POOL CVideoPool

using namespace std;

POOL::POOL()
{

}

POOL::~POOL()
{

}

std::vector< std::string > POOL::getClassList()
{
	vector<string> returnList;
	string sName;
	bool toAdd = true;

	// Naive O(n^2) algorithm... No big deal 
	for( int i = 0; i < _videoEntries.size(); ++i )
	{
		sName = _videoEntries[i].getClassName();
		for( int j = 0; j < returnList.size(); ++j )
		{
			if( sName == returnList[j] )
			{
				toAdd = false;
				break;
			}
		}
		if( toAdd )
			returnList.push_back( sName );
	}

	return returnList;
}

std::vector<CVideoEntry>& POOL::getVideoEntries()
{
	return _videoEntries;
}

void POOL::addVideoEntry( CVideoEntry entry )
{
	_videoEntries.push_back( entry );
}

#undef POOL
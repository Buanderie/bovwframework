#ifndef __VIDEOPOOL_H__
#define __VIDEOPOOL_H__

#include <vector>
#include <string>

#include "VideoEntry.h"

class CVideoPool
{

	private:
		std::vector< CVideoEntry > _videoEntries;
		
	public:
		CVideoPool();
		virtual ~CVideoPool();

		std::vector< std::string > getClassList();
		std::vector<CVideoEntry>& getVideoEntries();
		void addVideoEntry( CVideoEntry entry );

};

#endif
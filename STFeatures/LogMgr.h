#ifndef __LOGMGR_H__
#define __LOGMGR_H__

#include "Singleton.h"

class CLogMgr : public CSingleton< CLogMgr >
{
	friend class CSingleton<CLogMgr>;

	private:

	public:
		void init();
};

#endif
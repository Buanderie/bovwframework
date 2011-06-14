#ifndef __LOGMGR_H__
#define __LOGMGR_H__

#include <vector>
#include <string>
#include <ostream>

#include "Singleton.h"

class CLogMgr : public CSingleton< CLogMgr >
{
	friend class CSingleton<CLogMgr>;

	private:
		std::vector< std::string > _messageList;

	public:
		void init();
		void addMsg( std::string sender, std::string content );
		void flushTo( std::ostream& stream );

};

#endif
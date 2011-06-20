#ifndef __MODELARCHIVER_H__
#define __MODELARCHIVER_H__

#include <string>
#include <vector>

class CModelArchiver
{
	private:
		std::vector< std::string > _classList;

	public:
		CModelArchiver();
		virtual ~CModelArchiver();

		void write( std::vector<std::string> classList,
					std::string modelFileName,
					std::string outputFileName
				);
				
		void read(	
					std::string archiveFileName,
					std::string& outputModelFileName );
		
		std::vector< std::string > getClassList(){ return _classList; }
};

#endif
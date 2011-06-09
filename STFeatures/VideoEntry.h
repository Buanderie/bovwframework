#ifndef __VIDEOENTRY_H__
#define __VIDEOENTRY_H__

#include <string>
#include <vector>

#include "STFeature.h"

class CVideoEntry
{
	private:
		std::string					_videoFileName;
		std::string					_className;
		std::vector< float >		_bowVector;
		std::vector< STFeature >	_stFeatures;
				
	public:
		CVideoEntry( std::string videoFileName );
		CVideoEntry( std::string videoFileName, std::string className );
		virtual ~CVideoEntry();

		//Getters
		std::string getFileName(){ return _videoFileName; }
		std::string getClassName(){ return _className; }
		std::vector< float >& getBoWVector(){ return _bowVector; }
		std::vector<STFeature>& getFeatures(){ return _stFeatures; }

		//Setters
		void setClassName( std::string Name ){ _className = Name; }
		void setBoWVector( std::vector< float > Vector ){ _bowVector = Vector; }
		void setFeatures( std::vector< STFeature > features ){ _stFeatures = features; }
	
};

#endif
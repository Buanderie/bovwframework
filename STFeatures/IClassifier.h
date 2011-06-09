#ifndef __ICLASSIFIER_H__
#define __ICLASSIFIER_H__

#include <string>
#include <vector>

#include "CBoVW.h"
#include "VideoPool.h"

class IClassifier
{
	protected:
		int _featureLength;
		
	public:
		virtual ~IClassifier(){}

		virtual void createModel( CVideoPool& pool, CBoVW& bow ) = 0;
		virtual void saveModel( std::string fileName ) = 0;
		virtual void loadModel( std::string fileName ) = 0;
		virtual std::string label( std::vector<float> bowVector ) = 0;
};

#endif
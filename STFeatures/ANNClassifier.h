#ifndef __ANNCLASSIFIER_H__
#define __ANNCLASSIFIER_H__

#include <string>
#include "IClassifier.h"

class CANNClassifier : public IClassifier
{
	private:

	public:
		virtual ~CANNClassifier();
		CANNClassifier();

		virtual void createModel( CVideoPool& pool, CBoVW& bow );
		virtual void saveModel( std::string fileName );
		virtual void loadModel( std::string fileName );
		virtual std::string label( std::vector<float> bowVector ) = 0;
};

#endif
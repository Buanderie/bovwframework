#ifndef __ANNCLASSIFIER_H__
#define __ANNCLASSIFIER_H__

#include <vector>
#include <string>

#include "..\fann\include\fann.h"

#include "IClassifier.h"

class CANNClassifier : public IClassifier
{
	private:
		std::vector< std::string > _classNames;
		struct fann* ann;

		void init();
		void createFannInput( CVideoPool* pool, CBoVW* bow, char* fileName );

	public:
		virtual ~CANNClassifier();
		CANNClassifier();

		virtual void createModel( CVideoPool* pool, CBoVW* bow );
		virtual void saveModel( std::string fileName );
		virtual void loadModel( std::string fileName );
		virtual std::string label( std::vector<float> bowVector, CVideoPool* pool, CBoVW* bow, std::vector<float>& truthValues );
};

#endif
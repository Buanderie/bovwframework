#include "ANNClassifier.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

//FANN Include
#include "..\fann\include\fann.h"

#include "CBoVW.h"
#include "VideoPool.h"
#include "ModelArchiver.h"

#define ANN CANNClassifier

using namespace std;

ANN::ANN()
{
	init();
}

ANN::~ANN()
{

}

void ANN::init()
{
	ann = 0;
}

void ANN::createFannInput( CVideoPool* pool, CBoVW* bow, char* fileName )
{
	vector< string > classList = pool->getClassList();
	int nClasses	= classList.size();
	int nEntries	= pool->getVideoEntries().size();
	int vocabSize	= bow->getVocabSize();

	ofstream fout( fileName );
	fout	<< nEntries
			<< " "
			<< vocabSize
			<< " "
			<< nClasses
			<< endl;

	for( int i = 0; i < nEntries; ++i )
	{
		vector< float > curBow			= pool->getVideoEntries()[i].getBoWVector();
		string			curClassName	= pool->getVideoEntries()[i].getClassName(); 
		for( int j = 0; j < vocabSize; ++j )
		{
			fout << fixed << setprecision(16) << curBow[j] << " ";
		}
		fout << endl;
		for( int j = 0; j < nClasses; ++j )
		{
			if( classList[j] == curClassName )
				fout << "1 ";
			else
				fout << "0 ";
		}
		fout << endl;
	}
}

void ANN::createModel( CVideoPool* pool, CBoVW* bow )
{
	//Record class names for further use
	_classNames.clear();
	_classNames = pool->getClassList();

	//Build the FANN input file from the BoW data
	createFannInput( pool, bow, "fanninput.dat" );

	//FANN Neural Network creation
	const unsigned int num_input = bow->getVocabSize();
	const unsigned int num_output = _classNames.size();
	const unsigned int num_layers = 3;
	const unsigned int num_neurons_hidden = 24;
	const float desired_error = (const float) 0.0001;
	const unsigned int max_epochs = 500000;
	const unsigned int epochs_between_reports = 1;

	ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);
	fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
	fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
	fann_train_on_file(ann, "fanninput.dat", max_epochs, epochs_between_reports, desired_error);
}

void ANN::saveModel( std::string fileName )
{
	string netFileName = string("net_")+fileName;
	fann_save(ann, netFileName.c_str() );

	//Model Archiver
	CModelArchiver archiver;
	archiver.write( _classNames, netFileName, fileName );
	//
}

void ANN::loadModel( std::string fileName )
{
	string netFileName;

	//Model Archiver
	CModelArchiver archiver;
	archiver.read( fileName, netFileName );
	//

	if( ann == 0 )
		ann = fann_create_from_file( netFileName.c_str() );

	//retrieve class list
	this->_classNames = archiver.getClassList();
}

std::string ANN::label( std::vector<float> bowVector, CVideoPool* pool, CBoVW* bow, std::vector<float>& truthValues )
{
	const int nClasses = _classNames.size();
	float* rawFeat = new float[ bowVector.size() ];
	std::copy( bowVector.begin(), bowVector.end(), rawFeat );
	//float* ann_response = new float[ pool->getClassList().size() ];
	float* ann_response;

	if( ann != 0 )
	{
		ann_response = fann_run( ann, rawFeat );	
	}
	else
		return "notclassified";

	//Policy : the class with the highest score
	float maxScore	= FLT_MIN;
	int   maxIdx	= -1;
	for( int i = 0; i < ann->num_output; ++i )
	{
		if( ann_response[i] > maxScore )
		{
			maxScore = ann_response[i];
			maxIdx = i;
		}
	}
	delete[] rawFeat;
	
	truthValues.resize( nClasses );
	copy( ann_response, ann_response + nClasses, truthValues.begin() );

	if( maxIdx >= 0 && _classNames.size() != 0 )
		return _classNames[maxIdx];
	else
		return "notclassified";
}

#undef ANN
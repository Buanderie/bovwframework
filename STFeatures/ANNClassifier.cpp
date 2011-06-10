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
	const unsigned int num_neurons_hidden = 4;
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
	fann_save(ann, fileName.c_str() );
}

void ANN::loadModel( std::string fileName )
{
	if( ann == 0 )
		ann = fann_create_from_file( fileName.c_str() );
}

std::string ANN::label( std::vector<float> bowVector )
{
	float* rawFeat = new float[ bowVector.size() ];
	std::copy( bowVector.begin(), bowVector.end(), rawFeat );
	float* ann_response = 0;

	if( ann != 0 )
	{
		float* ann_response = fann_run( ann, rawFeat );	
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

	return "polbak";
}

#undef ANN
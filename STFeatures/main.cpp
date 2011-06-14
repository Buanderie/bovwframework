#include <iostream>
#include <fstream>
#include <iomanip>

#include "CBoVW.h"
#include "VideoPool.h"
#include "SURFSTExtractor.h"
#include "IClassifier.h"
#include "ANNClassifier.h"
#include "GenUtils.h"
#include "CProfiler.h"

using namespace cv;
using namespace std;

void usage( char** argv )
{
	cout << argv[0] << " usage:" << endl;
}

int main( int argc, char** argv )
{
	//0 - Training
	//1 - Classification
	int mode = 0;

	if( argc < 5 || argc > 6 )
	{
		usage( argv );
		return -1;
	}

	//Load program arguments
	if( string(argv[1]) == string("-training") )
		mode = 0;
	else if( string(argv[1]) == string("-classification") )
		mode = 1;
	else
	{
		cout << "invalid argument" << endl;
		return -1;
	}

	string inputFile = argv[2];
	string vocFile = argv[3];
	string modelFile = argv[4];
	string classOutput;
	if( mode == 1 && argc == 6 )
		classOutput = argv[5];

	CProfiler prof;

	CVideoPool* pool = new CVideoPool();
	SURFSTExtractor* extractor = new SURFSTExtractor();
	CANNClassifier* classifier = new CANNClassifier();
	CBoVW* bow = new CBoVW();
	bow->setExtractor( extractor );
	bow->setClassifier( classifier );
	bow->setVideoPool( pool );

	//populate video pool from xml file
	pool->loadFromXml( inputFile );
	//

	//Training
	if( mode == 0 )
	{
		bow->setVocabSize( 50 );
		bow->computeFeatures();
		bow->buildVocabulary();
		bow->computeBoW();

		bow->createClassificationModel();
		bow->saveVocabulary(vocFile);
		bow->saveClassificationModel(modelFile);
	}

	//Classification
	if( mode == 1 )
	{
		bow->loadVocabulary(vocFile);
		bow->loadClassificationModel(modelFile);
		bow->computeFeatures();
		bow->computeBoW();

		//Last step: labeling and output
		//
		vector<string> classList = pool->getClassList();
		vector<float> truth;
		truth.resize( classList.size() );
		for( int i = 0; i < pool->getVideoEntries().size(); ++i )
		{
			vector<float> bowValues = pool->getVideoEntries()[i].getBoWVector();
			cout << "video_" << i;
			string classname = bow->label( bowValues,truth );
			cout << " class=\"" << classname << "\" - ";
			for( int j = 0; j < truth.size(); ++j )
			{
				cout << truth[j] << ";";
			}
			cout << endl;
		}
		//
	}
	
	delete classifier;
	delete pool;
	delete extractor;
	delete bow;

	return 0;
}
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
	/*
	if( argc <= 1 )
	{
		usage( argv );
		return -1;
	}
	*/
	CProfiler prof;

	CVideoPool* pool = new CVideoPool();
	pool->addVideoEntry( CVideoEntry("Z:\\HumanDetection\\videos_hbd\\6pc0m.avi", "class1") );
	pool->addVideoEntry(  CVideoEntry("Z:\\HumanDetection\\videos_hbd\\6pc0w.avi", "class2") );
	pool->addVideoEntry( CVideoEntry("Z:\\HumanDetection\\videos_hbd\\6pc0m.avi", "class1") );
	pool->addVideoEntry(  CVideoEntry("Z:\\HumanDetection\\videos_hbd\\6pc0w.avi", "class2") );
	
	SURFSTExtractor* extractor = new SURFSTExtractor();
	CANNClassifier* classifier = new CANNClassifier();

	CBoVW* bow = new CBoVW();
	bow->setExtractor( extractor );
	bow->setClassifier( classifier );
	bow->setVideoPool( pool );
	bow->setVocabSize( 25 );
	
	bow->computeFeatures();
	bow->buildVocabulary();
	bow->computeBoW();

	bow->createClassificationModel();

	//Classification test
	prof.start();
	vector<float> bowFeat = pool->getVideoEntries()[0].getBoWVector();
	vector<float> truth;
	std::string myClass = bow->label( bowFeat, truth );
	prof.stop();
	cout << "classification time: " << prof.getMilliSeconds() << "ms" << endl;
	//

	delete classifier;
	delete pool;
	delete extractor;
	delete bow;

	return 0;
}
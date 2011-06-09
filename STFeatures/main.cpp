#include <iostream>
#include <fstream>
#include <iomanip>

#include "CBoVW.h"
#include "SURFSTExtractor.h"
#include "GenUtils.h"

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

	CBoVW* bow = new CBoVW();
	SURFSTExtractor* extractor = new SURFSTExtractor();
	bow->setExtractor( extractor );

	bow->setVocabSize( 50 );
	bow->addClass( "class1" );
	bow->addVideo( "Z:\\HumanDetection\\videos_hbd\\6pc0m.avi", "class1" );
	bow->addVideo( "Z:\\HumanDetection\\videos_hbd\\6pc0w.avi", "class1" );
	
	bow->computeFeatures();
	bow->computeVocabulary();
	bow->computeBoW();

	vector< videntry_t > entryList = bow->getVideoEntries();

	float histoDist = computeBhattacharyya( entryList[0]._bag, entryList[1]._bag );
	cout << "Bhattacharyya coefficient = " << histoDist << endl;

	delete extractor;
	delete bow;

	return 0;
}
#include <iostream>
#include <fstream>
#include <iomanip>

#include "CBoVW.h"
#include "VideoPool.h"
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
	CVideoPool pool;
	pool.addVideoEntry( CVideoEntry("Z:\\HumanDetection\\videos_hbd\\6pc0m.avi", "class1") );
	pool.addVideoEntry( CVideoEntry("Z:\\HumanDetection\\videos_hbd\\accidentm.avi", "class1") );
	pool.addVideoEntry(  CVideoEntry("Z:\\HumanDetection\\videos_hbd\\6pc0w.avi", "class1") );

	CBoVW* bow = new CBoVW();
	SURFSTExtractor* extractor = new SURFSTExtractor();
	bow->setExtractor( extractor );
	bow->setVocabSize( 25 );
	
	bow->computeFeatures( pool );
	bow->buildVocabulary( pool );
	bow->computeBoW( pool );

	vector<CVideoEntry> entries = pool.getVideoEntries();
	float histoDistA = computeBhattacharyya( entries[0].getBoWVector(), entries[1].getBoWVector() );
	float histoDistB = computeBhattacharyya( entries[1].getBoWVector(), entries[2].getBoWVector() );
	float histoDistC = computeBhattacharyya( entries[0].getBoWVector(), entries[2].getBoWVector() );
	float histoDistD = computeBhattacharyya( entries[0].getBoWVector(), entries[0].getBoWVector() );
	float histoDistE = computeBhattacharyya( entries[1].getBoWVector(), entries[1].getBoWVector() );
	float histoDistF = computeBhattacharyya( entries[2].getBoWVector(), entries[2].getBoWVector() );

	//cout << "Bhattacharyya coefficient = " << histoDist << endl;

	delete extractor;
	delete bow;

	return 0;
}
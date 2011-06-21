//STL
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

//OpenCV 2.X
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

//Boost
#include <boost/timer.hpp>

//Internal
#include "CBoVW.h"
#include "VideoPool.h"
#include "SURFSTExtractor.h"
#include "IClassifier.h"
#include "ANNClassifier.h"
#include "GenUtils.h"
#include "CProfiler.h"
#include "ActionDetector.h"

using namespace cv;
using namespace std;

void usage( char** argv )
{
	cout << argv[0] << " usage:" << endl;
}

void actioncb( string className, vector<float> results )
{
	cout << className << " - ";
	for( int i = 0; i < results.size(); ++i )
	{
		cout << results[i] << ";";
	}
	cout << endl;
}

int main( int argc, char** argv )
{
	CActionDetector act;

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
	
	if( mode == 1 )
	{
		CActionDetector detector( vocFile, modelFile );
		detector.setResultCallback( actioncb );

		//Test Video
		//cv::VideoCapture cap("Z:\\HumanDetection\\videos_hbd\\6pc0.avi");
		cv::VideoCapture cap("Z:\\compil.avi");
		//cv::VideoCapture cap(0);

		double videoFrameRate = cap.get( CV_CAP_PROP_FPS );

		cv::Mat frame;

		boost::timer t;
		
		int framepos = 0;
		for(;;)
		{
			cap >> frame;
			if( frame.empty() )
				break;

			t.restart();
			detector.update( frame );
			imshow( "video", frame );
			double elapsed_t = 1000.0 * t.elapsed();

			//25 fps = 0.04s / frame
			int waitVal = 1;
			double msecPerFrame = (1.0 / videoFrameRate)*1000.0;

			if( elapsed_t < msecPerFrame )
			{
				waitVal = (int)( msecPerFrame - elapsed_t );
				if( waitVal < 1 )
					waitVal = 1;
				//cout << "waitVal=" << waitVal << endl;
			}
			char c = cv::waitKey(waitVal);
			
			if( c == 27 )
				break;
		
			framepos++;
		}
	}

	delete classifier;
	delete pool;
	delete extractor;
	delete bow;

	return 0;
}
#include "ActionDetector.h"

//STL
#include <string>
#include <algorithm>

//OpenCV 2.X
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>

//Boost
#include <boost\thread\thread.hpp>

//Internal
#include "CBoVW.h"
#include "SURFSTExtractor.h"
#include "ANNClassifier.h"

#define ACDC CActionDetector

using namespace std;
using namespace cv;
using namespace boost;

void ACDC::init()
{
	_callback = 0;

	_bow = new CBoVW();
	
	_extractor = new SURFSTExtractor();
	_classifier = new CANNClassifier();

	_bow->setExtractor( _extractor );
	_bow->setClassifier( _classifier );

	//Load vocabulary file
	if( _vocabFileName.size() > 0 )
	{
		_bow->loadVocabulary( _vocabFileName );
	}	

	//Load classification model file
	if( _modelFileName.size() > 0 )
	{
		_bow->loadClassificationModel( _modelFileName );
	}
}

ACDC::ACDC()
{
	init();
}

ACDC::ACDC( std::string vocabFileName, std::string modelFileName )
{
	_vocabFileName = vocabFileName;
	_modelFileName = modelFileName;
	init();
}

ACDC::~ACDC()
{
	delete _extractor;
	delete _classifier;
	delete _bow;
}

void ACDC::update( cv::Mat& frame )
{
	//Create a hardcopy of the submitted frame
	cv::Mat tmpFrame;
	cv::Mat grayFrame;
	frame.copyTo( tmpFrame );
	cv::cvtColor( tmpFrame, grayFrame, CV_RGB2GRAY );

	//Put it in the frame queue (Critical)
	_processingMutex.lock();
	_frames.push_back( grayFrame );
	_processingMutex.unlock();

	//If we reached a significant number of frames
	if( _frames.size() >= 20 )
	{
		//Launch processing thread
		_processingThread = boost::thread( procThreadStub, this );
	}
}

/*
*	Thread..
*/
void ACDC::procThreadStub( CActionDetector* owner )
{
	//Calling the real thread function
	owner->procThreadFunc();
}

void ACDC::procThreadFunc()
{

	//Copy the whole bunch of frames (Critical)
	_processingMutex.lock();
	vector<cv::Mat> frameQueue;
	frameQueue.resize( _frames.size() );
	copy( _frames.begin(), _frames.end(), frameQueue.begin() );
	_frames.clear();
	_processingMutex.unlock();
	//

	vector<float> truth;
	string classname = _bow->label( frameQueue, truth );

	callCB( classname, truth );
}

/*
*	Callback mgmt
*/
void ACDC::setResultCallback( ACDC::actionDetectionCallback cb )
{
	_callback = cb;
}

void ACDC::callCB(std::string className, vector<float> classificationResults )
{
	if( _callback != 0 )
	{
		_callbackMutex.lock();
		_callback( className, classificationResults );
		_callbackMutex.unlock();
	}
}

#undef ACDC
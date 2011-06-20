#ifndef __ACTIONDETECTOR_H__
#define __ACTIONDETECTOR_H__

//STL
#include <string>
#include <vector>

//OpenCV 2.X
#include <opencv2\core\core.hpp>

//Boost
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

//
#include "CBoVW.h"
#include "SURFSTExtractor.h"
#include "ANNClassifier.h"

class CActionDetector
{
	public:
		//( int n_truth_values, float* truth_values )
		typedef void(*actionDetectionCallback)(std::string className, std::vector<float>);

	private:
		boost::thread			_processingThread;
		boost::mutex			_processingMutex;
		boost::mutex			_callbackMutex;

		CBoVW*					_bow;
		SURFSTExtractor*		_extractor;
		CANNClassifier*			_classifier;
		std::string				_vocabFileName;
		std::string				_modelFileName;

		std::vector<cv::Mat>	_frames;

		actionDetectionCallback _callback;

		void init();
		void callCB( std::string className, std::vector<float> classificationResults );

	public:
		CActionDetector();
		CActionDetector( std::string vocabFileName, std::string modelFileName );
		virtual ~CActionDetector();

		//
		void update( cv::Mat& frame );

		static void procThreadStub( CActionDetector* owner );
		void procThreadFunc();

		//callback
		void setResultCallback( actionDetectionCallback cb );

};

#endif
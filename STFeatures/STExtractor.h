#ifndef __STEXTRACTOR_H__
#define __STEXTRACTOR_H__

#include "STFeature.h"

#include <vector>
#include <deque>
#include <opencv2\core\core.hpp>
#include <opencv2\features2d\features2d.hpp>

class STExtractor
{
	private:
		int _desiredFTRS;
		int _currentFrame;
		cv::GridAdaptedFeatureDetector* _featDetector;
		
		std::vector< cv::KeyPoint >		_movingKP;
		std::vector< cv::Point2f >		_movingVec;
		std::vector< float >			_movingDesc;
		std::vector< cv::KeyPoint >		_goodKP;

		void init();

		void extractMovingFeatures( std::deque< cv::Mat > frames );
		std::vector<STFeature> computeFeatures( std::deque< cv::Mat > frames, bool showOutput );

		double	_hessianThreshold;
		int		_procUnitLength;
		int		_intervalLength;

	public:
		STExtractor( int procUnitLength=20, int intervalLength=2 );
		virtual ~STExtractor();
		std::vector<STFeature> extract( std::deque< cv::Mat > frames, int currentFrame, bool showOutput );		
};

#endif
#ifndef __SURFSTEXTRACTOR_H__
#define __SURFSTEXTRACTOR_H__

#include "ISTExtractor.h"
#include "STFeature.h"

#include <vector>

#include <opencv2\core\core.hpp>
#include <opencv2\features2d\features2d.hpp>

class SURFSTExtractor : public ISTExtractor
{
	private:
		std::vector< STFeature >	_features;
		int							_currentFramePos;
		int _desiredFTRS;
		int _currentFrame;
		double	_hessianThreshold;
		int		_procUnitLength;
		int		_intervalLength;

		cv::GridAdaptedFeatureDetector* _featDetector;
		
		std::vector< cv::KeyPoint >		_movingKP;
		std::vector< cv::Point2f >		_movingVec;
		std::vector< float >			_movingDesc;
		std::vector< cv::KeyPoint >		_goodKP;

		//Internal SURF-ST Functions
		void init();
		void extractMovingFeatures( std::vector< cv::Mat > frames );
		void computeFeatures( std::vector< cv::Mat > frames );

	public:
		SURFSTExtractor();
		virtual ~SURFSTExtractor();

		virtual int getFeatureLength();
		virtual int	computeFeatures( std::vector< cv::Mat > frames, int framePos );
		virtual std::vector<STFeature>	getFeatures();
};

#endif
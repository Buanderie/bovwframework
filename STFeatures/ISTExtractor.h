#ifndef __ISTEXTRACTOR_H__
#define __ISTEXTRACTOR_H__

#include <vector>
#include <opencv2\core\core.hpp>

#include "STFeature.h"

class ISTExtractor
{
	
	public:
		virtual ~ISTExtractor(){}

		virtual int						getFeatureLength() = 0;
		virtual int						computeFeatures( std::vector< cv::Mat > frames, int framePos ) = 0;
		virtual std::vector<STFeature>	getFeatures() = 0;
		virtual void					clear() = 0;

	protected:
		std::vector<STFeature>	_features;
};

#endif
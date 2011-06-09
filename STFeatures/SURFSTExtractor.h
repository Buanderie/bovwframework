#ifndef __SURFSTEXTRACTOR_H__
#define __SURFSTEXTRACTOR_H__

#include "ISTExtractor.h"

class SURFSTExtractor : public ISTExtractor
{
	public:
		virtual ~SURFSTExtractor();

		virtual int getFeatureLength();
		virtual int	computeFeatures( std::vector< cv::Mat > frames, int framePos );
		virtual std::vector<STFeature>	getFeatures();
};

#endif
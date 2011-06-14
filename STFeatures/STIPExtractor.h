#ifndef __STIPEXTRACTOR_H__
#define __STIPEXTRACTOR_H__

#include "ISTExtractor.h"

class STIPExtractor : public ISTExtractor
{
	public:
		virtual ~STIPExtractor();

		virtual int getFeatureLength();
		virtual int	computeFeatures( std::vector< cv::Mat > frames, int framePos );
		virtual std::vector<STFeature>	getFeatures();
		virtual void clear();
};

#endif

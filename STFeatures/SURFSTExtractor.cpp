#include "SURFSTExtractor.h"

#include "STFeature.h"
#include <vector>

#define SURFST SURFSTExtractor

using namespace std;

SURFST::~SURFSTExtractor()
{

}

int SURFST::getFeatureLength()
{
	return 0;
}

int	SURFST::computeFeatures( std::vector< cv::Mat > frames, int framePos )
{
	return 0;
}

std::vector<STFeature> SURFST::getFeatures()
{
	vector<STFeature> feat;

	return feat;
}

#undef SURFST
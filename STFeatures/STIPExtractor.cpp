#include "STIPExtractor.h"

#include <vector>
#include <opencv2\core\core.hpp>

#include "STFeature.h"

#define STIP STIPExtractor

using namespace std;
using namespace cv;

STIP::~STIP()
{

}

int STIP::getFeatureLength()
{
	return 0;
}

int	STIP::computeFeatures( std::vector< cv::Mat > frames, int framePos )
{
	return 0;
}

std::vector<STFeature> STIP::getFeatures()
{
	vector<STFeature> feat;
	
	return feat;	
}

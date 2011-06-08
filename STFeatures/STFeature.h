#ifndef __STFEATURE_H__
#define __STFEATURE_H__

#include <vector>
#include <opencv2/core/core.hpp>

class STFeature
{
	private:
		std::vector< float > _appearanceFeat;
		std::vector< float > _motionFeat;

		std::vector< cv::Point2f > _motionTrack;
		std::vector< float > _timeStamps;

		int _n;
		int _m;

	public:
		STFeature();
		STFeature(const STFeature& copy);
		STFeature& operator =(const STFeature& Other);
		virtual ~STFeature();

		void setAppearanceFeature( std::vector< float > aFeat );
		void setMotionFeature( std::vector< float > mFeat );
		std::vector< float >& getMotionFeature();
		std::vector< float >& getAppearanceFeature();
		std::vector< float >  getAggregatedFeature();
		std::vector< cv::Point2f >& getMotionTrack();
		std::vector< float >& getTimeStamps();
		void normalizeMotionFeature();

		void setProcUnitLength( int N );
		void setIntervalLength( int M );
		int getProcUnitLength();
		int getIntervalLength();
		
		int getFeatureLength();
};

#endif
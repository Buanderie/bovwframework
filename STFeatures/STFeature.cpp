#include "STFeature.h"

#include <vector>
#include <algorithm>

#define ST STFeature

using namespace std;

ST::ST()
{
	_appearanceFeat.clear();
	_motionFeat.clear();
	_motionTrack.clear();
}

ST::ST(const ST& copy)
{
	(*this) = copy;
}

ST& ST::operator =(const ST& copy)
{
	if( this != &copy )
	{
		_appearanceFeat.resize( copy._appearanceFeat.size() );
		_motionFeat.resize( copy._motionFeat.size() );
		_motionTrack.resize( copy._motionTrack.size() );
		std::copy( copy._appearanceFeat.begin(), copy._appearanceFeat.end(), _appearanceFeat.begin() );
		std::copy( copy._motionFeat.begin(), copy._motionFeat.end(), _motionFeat.begin() );
		std::copy( copy._motionTrack.begin(), copy._motionTrack.end(), _motionTrack.begin() );
	}
	return *this;
}

ST::~ST()
{
	_appearanceFeat.clear();
	_motionFeat.clear();
	_motionTrack.clear();
}

void ST::setAppearanceFeature( std::vector< float > aFeat )
{
	_appearanceFeat.resize( aFeat.size() );
	copy( aFeat.begin(), aFeat.end(), _appearanceFeat.begin() );
}

void ST::setMotionFeature( std::vector< float > mFeat )
{
	_motionFeat.resize( mFeat.size() );
	copy( mFeat.begin(), mFeat.end(), _motionFeat.begin() );
}

std::vector< float >& ST::getMotionFeature()
{
	return this->_motionFeat;
}

std::vector< float >& ST::getAppearanceFeature()
{
	return this->_appearanceFeat;
}

std::vector< cv::Point2f >& ST::getMotionTrack()
{
	return this->_motionTrack;
}

std::vector< float >& ST::getTimeStamps()
{
	return this->_timeStamps;
}

void ST::setProcUnitLength( int N )
{
	_n = N;
}

void ST::setIntervalLength( int M )
{
	_m = M;
}

int ST::getProcUnitLength()
{
	return _n;
}

int ST::getIntervalLength()
{
	return _m;
}

void ST::normalizeMotionFeature()
{
	const int nIter = _n/_m;
	for( int k = 0; k < nIter; ++k )
	{
		float sqsum = 0.0f;
		for (int i = 0; i < 5; ++i )
		{
			sqsum += (_motionFeat[k*5 + i]*_motionFeat[k*5+i]);
		}
		float vecLen = sqrtf(sqsum);
		for( int i = 0; i < 5; ++i )
		{
			_motionFeat[k*5 + i] /= vecLen;
		}
	}
}

//
//Returns the (64 + (M-1)*5)-d Feature Vector
std::vector< float >  ST::getAggregatedFeature()
{
	vector< float > stFeat;
	stFeat.resize( _appearanceFeat.size() + _motionFeat.size() );

	//Concatenate SURF 64-d vector
	std::copy( _appearanceFeat.begin(), _appearanceFeat.end(), stFeat.begin() );

	//Concatenate Motion ((M-1)*5)-d vector
	std::copy( _motionFeat.begin(), _motionFeat.end(), stFeat.begin() + /*64*/ _appearanceFeat.size() );

	return stFeat;
}

#undef ST
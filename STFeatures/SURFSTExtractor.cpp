#include "SURFSTExtractor.h"

#include <iostream>
#include <vector>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\video\tracking.hpp>

#include "STFeature.h"

#define PI 3.141592653589793238463
#define SURFST SURFSTExtractor

using namespace std;
using namespace cv;

/*
*
  Extractor implementation
*
*/

SURFST::~SURFSTExtractor()
{

}

int SURFST::getFeatureLength()
{
	return 0;
}

int	SURFST::computeFeatures( std::vector< cv::Mat > frames, int framePos )
{
	vector< STFeature > features;
	_currentFramePos = framePos;
	extractMovingFeatures( frames );
	this->computeFeatures( frames );
	return 0;
}

std::vector<STFeature> SURFST::getFeatures()
{
	return _features;
}

/*
*
  SURF-ST Internal
*
*/
Point2f rotateOFVector( Point2f iVec, float theta )
{
	float x = iVec.x;
	float y = iVec.y;
	float x0 = x*cos( theta ) - y*sin( theta );
	float y0 = x*sinf( theta ) + y*cos( theta );
	return Point2f( x0, y0 );
}

SURFST::SURFST()
{
	init();
}

void SURFST::init()
{
	_hessianThreshold = 800;
	_procUnitLength = 20;
	_intervalLength = 5;
}

void SURFST::extractMovingFeatures( std::vector< cv::Mat > frames )
{
	_movingKP.clear();
	_movingDesc.clear();
	_movingVec.clear();
	_goodKP.clear();

	//..
	if( frames.size() != _procUnitLength )
		frames.resize( _procUnitLength );

	int N = frames.size();
	int N2 = N/2;

	//Detection des points d'interet à la frame 0
	Mat frame0 =	frames[0];

	cv::SURF surf(_hessianThreshold, 4, 2, false);
	surf.operator()( frame0, cv::Mat(), _movingKP );

	vector< Point2f > ofPointsPrev;
	vector< Point2f > ofPointsNext;
	vector< unsigned char > status;
	vector< float > err;
	for( int i = 0; i < _movingKP.size(); ++i )
		ofPointsPrev.push_back( _movingKP[i].pt );

	//Calcul du flot optique entre la frame 0 et la frame N/2
	Mat frameN2 =	frames[N2];
	Mat flowN2;
	Mat fbackout;
	cv::cvtColor( frame0, fbackout, CV_GRAY2RGB );
	calcOpticalFlowPyrLK( frame0, frameN2, ofPointsPrev, ofPointsNext, status, err );
	try{
		int cpt = 0;
		vector< KeyPoint > goodKP;
		vector< Point2f > goodVec;
		for( int i = 0; i < ofPointsNext.size(); ++i )
		{
			if( status[i] == 1 )
			{
				Point2f diff(ofPointsNext[i].x - ofPointsPrev[i].x, ofPointsNext[i].y - ofPointsPrev[i].y );
				float mov = sqrt( diff.x*diff.x + diff.y * diff.y );
				if( mov >= 10.0f && mov < 50.0f )
				{
					cpt ++;
					_goodKP.push_back( _movingKP[i] );
				}
			}
		}
		_movingVec.resize( cpt );
	}
	catch( cv::Exception& e )
	{
		cout << e.what() << endl;
	}
}

void SURFST::computeFeatures( std::vector< cv::Mat > frames )
{
	if( frames.size() != _procUnitLength )
		frames.resize( _procUnitLength );

	int N = frames.size();
	int M = _intervalLength;

	vector< unsigned char > status;
	vector< float > err;
	vector< Point2f > ofPointsNext;
	vector< Point2f > ofPointsPrev;

	_features.resize( _goodKP.size() );

	for( int i = 0; i < _goodKP.size(); ++i )
		ofPointsNext.push_back( _goodKP[i].pt );
	
	//Extracting appearance features
	cv::SURF surf(_hessianThreshold,4,2,false);
	surf.operator()( frames[0], cv::Mat(), _goodKP, _movingDesc, true );
	
	//_movingDesc.resize( _goodKP.size() * 64 );

	for( int i = 0; i < _goodKP.size(); ++i )
	{
		_features[i].getAppearanceFeature().resize( surf.descriptorSize() );
		int offset = surf.descriptorSize() * i;
		copy( _movingDesc.begin() + offset, _movingDesc.begin() + offset + surf.descriptorSize(), _features[i].getAppearanceFeature().begin() );
	}
	//

	//N n'est pas un multiple de M
	if( N % M != 0 )
		return;

	//sinon
	const int nInter = N/M;

	//Temp 5d motion feature
	vector<float> tempMotionFeat;
	tempMotionFeat.resize((nInter)*5);
	fill( tempMotionFeat.begin(), tempMotionFeat.end(), 0.0f );
	for( int k = 0; k < _features.size(); ++k )
	{
		_features[k].setMotionFeature(tempMotionFeat);
		_features[k].setProcUnitLength( N );
		_features[k].setIntervalLength( M );
	}

	//Extracting motion features
	//Pour chaque interval
	for( int i = 0; i < nInter; i+=1 )
	{
		ofPointsPrev.resize(ofPointsNext.size());
		copy( ofPointsNext.begin(), ofPointsNext.end(), ofPointsPrev.begin() );

		const int idxA = i*M;
		const int idxB = (i+1)*M - 1;

		Mat frameA = frames[ idxA ];
		Mat frameB = frames[ idxB ];

		//
		Mat fbackout;
		cv::cvtColor( frameB, fbackout, CV_GRAY2RGB );
		//

		calcOpticalFlowPyrLK( frameA, frameB, ofPointsPrev, ofPointsNext, status, err );

		for( int k = 0; k < ofPointsPrev.size(); ++k )
		{
			Point2f rot;
			tempMotionFeat = _features[k].getMotionFeature();

			Point2f diff( ofPointsNext[k].x - ofPointsPrev[k].x, ofPointsNext[k].y - ofPointsPrev[k].y );
			float mov = sqrt(diff.dot(diff));
			if( mov > 25.0f )
				status[k] = 0;

			if( status[k] == 1 )
			{
				_movingVec[k] = Point2f( ofPointsNext[k].x - ofPointsPrev[k].x, ofPointsNext[k].y - ofPointsPrev[k].y );

				//if status[k] == 0 -> X^0 = 0
				//tempMotionFeat[4] = 0.0f;

				float theta = _goodKP[k].angle/360.0f * PI;
				rot = rotateOFVector( _movingVec[k], theta );
				
				if( rot.x > 0.0f )
					tempMotionFeat[i*5 + 0] += abs(rot.x);
				else
					tempMotionFeat[i*5 + 1] += abs(rot.x);

				if( rot.y > 0.0f )
					tempMotionFeat[i*5 + 2] += abs(rot.y);
				else
					tempMotionFeat[i*5 + 3] += abs(rot.y);
			}
			else
			{
				tempMotionFeat[i*5 + 4] += 1.0f;
			}

			_features[k].getMotionTrack().push_back( ofPointsNext[k] );
			_features[k].getTimeStamps().push_back( (float)( _currentFrame - N + i ) );
			_features[k].setMotionFeature( tempMotionFeat );
		}
	}

	//Normalisation du vecteur 5d et aggregation
	for( int i = 0; i < _features.size(); ++i )
	{
		_features[i].normalizeMotionFeature();
	}
}

#undef SURFST
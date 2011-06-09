#include <opencv2\features2d\features2d.hpp>
#include <opencv2\video\tracking.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include <deque>
#include <iostream>
#include <cmath>

#include "STExtractor.h"

#define EXT STExtractor
#define PI 3.141592653589793238463
using namespace std;
using namespace cv;

EXT::EXT(int procUnitLength, int intervalLength):_procUnitLength(procUnitLength), _intervalLength(intervalLength)
{
	init();
}

EXT::~EXT()
{
	//delete _featDetector;
}

void EXT::init()
{
	//A definir...
	//Dépend de la densité des points
	_hessianThreshold = 800;
}

Point2f rotateOFVector( Point2f iVec, float theta )
{
	float x = iVec.x;
	float y = iVec.y;
	float x0 = x*cos( theta ) - y*sin( theta );
	float y0 = x*sinf( theta ) + y*cos( theta );
	return Point2f( x0, y0 );
}

void EXT::extractMovingFeatures( std::deque< cv::Mat > frames )
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

std::vector<STFeature> EXT::computeFeatures( std::deque< cv::Mat > frames, bool showOutput )
{
	if( frames.size() != _procUnitLength )
		frames.resize( _procUnitLength );

	int N = frames.size();
	int M = _intervalLength;

	vector< unsigned char > status;
	vector< float > err;
	vector< Point2f > ofPointsNext;
	vector< Point2f > ofPointsPrev;

	vector< STFeature > stFeat;
	stFeat.resize( _goodKP.size() );

	for( int i = 0; i < _goodKP.size(); ++i )
		ofPointsNext.push_back( _goodKP[i].pt );
	
	//Extracting appearance features
	cv::SURF surf(_hessianThreshold,4,2,false);
	surf.operator()( frames[0], cv::Mat(), _goodKP, _movingDesc, true );
	
	//_movingDesc.resize( _goodKP.size() * 64 );

	for( int i = 0; i < _goodKP.size(); ++i )
	{
		stFeat[i].getAppearanceFeature().resize( surf.descriptorSize() );
		int offset = surf.descriptorSize() * i;
		copy( _movingDesc.begin() + offset, _movingDesc.begin() + offset + surf.descriptorSize(), stFeat[i].getAppearanceFeature().begin() );
	}
	//

	//N n'est pas un multiple de M
	if( N % M != 0 )
		return stFeat;

	//sinon
	const int nInter = N/M;

	//Temp 5d motion feature
	vector<float> tempMotionFeat;
	tempMotionFeat.resize((nInter)*5);
	fill( tempMotionFeat.begin(), tempMotionFeat.end(), 0.0f );
	for( int k = 0; k < stFeat.size(); ++k )
	{
		stFeat[k].setMotionFeature(tempMotionFeat);
		stFeat[k].setProcUnitLength( N );
		stFeat[k].setIntervalLength( M );
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
			tempMotionFeat = stFeat[k].getMotionFeature();

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

				if( showOutput )
				{
					int radius = 10;
					cv::circle( fbackout, ofPointsPrev[k], radius, CV_RGB(0,0,255), 1 );
					float arclen = sqrt( rot.x*rot.x + rot.y*rot.y );
					cv::line( fbackout, ofPointsPrev[k], Point2f( ofPointsPrev[k].x + radius * rot.x/arclen, ofPointsPrev[k].y + radius * rot.y/arclen ), CV_RGB(0,0,255), 1 );
				}
			}
			else
			{
				tempMotionFeat[i*5 + 4] += 1.0f;
			}

			stFeat[k].getMotionTrack().push_back( ofPointsNext[k] );
			stFeat[k].getTimeStamps().push_back( (float)( _currentFrame - N + i ) );
			stFeat[k].setMotionFeature( tempMotionFeat );
		}

		if( showOutput )
		{
			imshow("fback", fbackout);
			cv::waitKey(5);
		}
	}

	//Normalisation du vecteur 5d et aggregation
	for( int i = 0; i < stFeat.size(); ++i )
	{
		//Operateurs de recopie ?
		stFeat[i].normalizeMotionFeature();
	}

	return stFeat;
}

std::vector<STFeature> EXT::extract( std::deque< cv::Mat > frameQueue, int currentFrame, bool showOutput )
{
	vector< STFeature > features;
	_currentFrame = currentFrame;
	this->extractMovingFeatures( frameQueue );
	features = this->computeFeatures( frameQueue, showOutput );
	return features;
}

#undef EXT
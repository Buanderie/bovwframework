#include "CBoVW.h"
#include "STFeature.h"
#include "STExtractor.h"
#include "..\kmpp\KMeans.h"
#include <stdlib.h>
#include <time.h>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#define BOW CBoVW

using namespace std;
using namespace cv;

void BOW::init()
{
	srand(time(NULL));
	//_maxClass = 10000;
	_vocabSize = 50;
}

BOW::BOW()
{
	init();
}

BOW::~BOW()
{

}

classtype_t BOW::addClass( std::string className )
{
	bool alreadyThere = false;
	int idx = 0;
	//Look for existing class
	for( int i = 0; i < _classes.size(); ++i )
	{
		if( _classes[i].className == className )
		{
			alreadyThere = true;
			idx = i;
			break;
		}
	}

	if( alreadyThere )
	{
		return _classes[idx];
	}
	else
	{
		classtype_t clt;
		clt.classId = rand()%_maxClass;
		clt.className = className;
		_classes.push_back( clt );
		return clt;
	}
}

void BOW::addVideo( std::string fileName, std::string className )
{
	videntry_t vident;
	vident.className = className;
	vident.videoFileName = fileName;
	_videoEntry.push_back( vident );
}

void BOW::computeVideoFeatures( std::string fileName, vector<STFeature>& featureVec )
{
	const int procUnitLen = 20;
	const int intervalLen = 5;
	const int width = 320;
	const int height = 240;

	STExtractor ext( procUnitLen, intervalLen );
	
	//Open video
	VideoCapture cap( fileName );
	Mat frame;
	Mat frameg;
	Mat frameg2;
	Mat hardCopy;
	vector<STFeature> tempFeat;
	vector<cv::Mat> frames;
	int frameCpt = 0;

	featureVec.clear();

	cout << "Processing " << fileName << "..." << endl;
	
	int lastPercent = 0;

	for(;;)
	{
		cap >> frame;
		if( frame.empty() )
			break;

		double pos = cap.get( CV_CAP_PROP_POS_AVI_RATIO );
		if( lastPercent != (int)(ceil(pos*100)) )
		{
			lastPercent = (int)(ceil(pos*100));
			cout << lastPercent << "%" << endl;
		}

		cv::cvtColor( frame, frameg, CV_RGB2GRAY );
		cv::resize( frameg, frameg2, cv::Size(width,height));
		hardCopy = frameg2.clone();
		frames.push_back( hardCopy );
		
		//we reached processing unit length
		if( frames.size() == procUnitLen )
		{
			tempFeat.clear();
			tempFeat = ext.extract( frames, frameCpt, false );

			//append extracted features
			featureVec.insert(featureVec.end(), tempFeat.begin(), tempFeat.end());

			frames.clear();
		}

		frameCpt++;
	}
	cout << "Done!" << endl;
}

void BOW::computeBoW( std::vector<STFeature>& features, std::vector< float >& output )
{
	//Do vector quantization over all video features
	vector< int > bow;
	bow.resize( _vocabSize );
	std::fill( bow.begin(), bow.end(), 0 );
	cout << "VQ Step..." << endl;
	for (int i = 0; i < features.size(); ++i )
	{
		cout << ceil( (double)i / (double)(features.size())*100 ) << endl;
		int word = featureVQ( features[i] );
		bow[ word ] += 1;
	}
	cout << "Done!" << endl;

	//Normalize bag of word
	output.resize( _vocabSize );
	float sqsum = 0.0f;
	for (int i = 0; i < bow.size(); ++i )
		sqsum += (bow[i]*bow[i]);
	float bowLen = sqrtf( sqsum );
	
	for( int i = 0; i < _vocabSize; ++i )
	{
		output[i] = (((float)(bow[i])) / bowLen);
	}
}

int BOW::featureVQ( STFeature& feat )
{
	float mindist = FLT_MAX;
	int min_idx = -1;
	float sqsum = 0;
	vector< float > featValues = feat.getAggregatedFeature();
	int wordLength = featValues.size();

	for( int i = 0; i < _vocabSize; ++i )
	{
		sqsum = 0.0f;
		for (int j = 0; j < wordLength; ++j )
		{
			float centerValue = _clusterCenters[ i * wordLength + j ];
			sqsum += (featValues[j]-centerValue)*(featValues[j]-centerValue);
		}
		float dist = sqrtf(sqsum);
		if( dist < mindist )
		{
			mindist = dist;
			min_idx = i;
		}
	}

	return min_idx;
}

void BOW::computeFeatures()
{
	//For each video entry...
	for( int i = 0; i < _videoEntry.size(); ++i )
	{
		//Compute features for one video
		computeVideoFeatures( _videoEntry[i].videoFileName, _videoEntry[i]._features );
	}
}

void BOW::computeVocabulary()
{
	if( _videoEntry.size() <= 0 )
		return;

	//Use kMeans clustering to build vocabulary
	//
	// 0 - Compute required memory space
	int szFeaturePool = 0;
	int szWord = _videoEntry[0]._features[0].getAggregatedFeature().size();
	int nFeatures = 0;
	for (int i = 0; i < _videoEntry.size(); ++i )
	{
		int nFeat = _videoEntry[i]._features.size();
		szFeaturePool += (nFeat * szWord);
		nFeatures += nFeat;
	}
	
	// 1 - Create feature space for clustering
	_featurePool = new double[ szFeaturePool ];
	//Copy feature data
	//For each video
	int arrayOffset = 0;
	for( int i = 0; i < _videoEntry.size(); ++i )
	{
		//for each computed feature
		for( int j = 0; j < _videoEntry[i]._features.size(); ++j )
		{
			vector<float> feat = _videoEntry[i]._features[j].getAggregatedFeature();
			//For each feature element
			for (int k = 0; k < feat.size(); ++k )
			{
				_featurePool[ arrayOffset++ ] = (double)(feat[k]); 
			}
		}
	}

	// 2 - Prepare kMeans clustering
	_clusterCenters = new double[ szWord * this->_vocabSize ];
	_kmeansResults = new int[ szFeaturePool ];

	// 3 - Start kMeans
	AddKMeansLogging( &cout, true );
	RunKMeans( nFeatures, _vocabSize, szWord, _featurePool, 1, _clusterCenters, _kmeansResults );
}

void BOW::computeBoW()
{
	for( int i = 0; i < _videoEntry.size(); ++i )
	{
		computeBoW( _videoEntry[i]._features, _videoEntry[i]._bag );
	}
}

std::vector< videntry_t >& BOW::getVideoEntries()
{
	return this->_videoEntry;
}

#undef BOW
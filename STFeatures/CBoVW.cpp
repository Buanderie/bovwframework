#include "CBoVW.h"
#include "STFeature.h"
#include "VideoPool.h"
#include "ISTExtractor.h"
#include "GenUtils.h"
#include "CProfiler.h"
#include "..\kmpp\KMeans.h"

#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#define BOW CBoVW

using namespace std;
using namespace cv;


void BOW::init()
{
	srand(time(NULL));
	_vocabSize = 50;
	_extractor = 0;
	_classifier = 0;
	_videoPool = 0;
	_featureCountThreshold = 10;
}

void BOW::setExtractor( ISTExtractor* extractor )
{
	_extractor = extractor;
}

void BOW::setClassifier( IClassifier* classifier )
{
	_classifier = classifier;
}

void BOW::setVideoPool( CVideoPool* videoPool )
{
	_videoPool = videoPool;
}

BOW::BOW()
{
	init();
}

BOW::~BOW()
{

}

void BOW::computeVideoFeatures( std::string fileName, vector<STFeature>& featureVec )
{
	const int procUnitLen = 20;
	const int intervalLen = 5;
	const int width = 320;
	const int height = 240;

	CProfiler prof;

	//Open video
	VideoCapture cap( fileName );
	Mat frame;
	Mat frameg;
	Mat frameg2;
	Mat hardCopy;

	vector<STFeature> tempFeat;
	vector<cv::Mat> frames;
	vector<cv::Mat> frameQueue;

	int frameCpt = 0;
	int nbFrameTotal = cap.get(CV_CAP_PROP_FRAME_COUNT);

	featureVec.clear();
	featureVec.reserve( 10000 );

	cout << "Processing " << fileName << "..." << endl;
	cout << "Total number of frames: " << nbFrameTotal << endl;

	int lastPercent = 0;
	double lastFrameRate = 0;
	double accFrameRate = 0;
	int accFrameRateCpt = 0;

	for(;;)
	{
		cap >> frame;
		if( frame.empty() )
			break;

		/*
		double pos = cap.get( CV_CAP_PROP_POS_AVI_RATIO );
		if( lastPercent != (int)(ceil(pos*100)) )
		{
			double meanFrameRate = accFrameRate/(double)accFrameRateCpt;
			accFrameRate = 0;
			accFrameRateCpt = 0;
			lastPercent = (int)(ceil(pos*100));
			cout << lastPercent << "%" << " - " << meanFrameRate << " fps" << endl;
		}
		else
		{
			accFrameRate += lastFrameRate;
			accFrameRateCpt++;
		}
		*/

		cv::cvtColor( frame, frameg, CV_RGB2GRAY );
		cv::resize( frameg, frameg2, cv::Size(width,height));
		hardCopy = frameg2.clone();

		//we reached processing unit length
		if( frameQueue.size() == procUnitLen )
		{
			prof.start();
			tempFeat.clear();

			/*
			* new arch
			*/
				if( _extractor != 0 )
				{
					_extractor->computeFeatures( frameQueue, frameCpt );
					tempFeat = _extractor->getFeatures();
				}
			/*
			*/

			featureVec.insert(featureVec.end(), tempFeat.begin(), tempFeat.end());
			prof.stop();
			frameQueue.clear();
			lastFrameRate = 1.0/prof.getSeconds();
		}
		frameQueue.push_back( hardCopy );
		//
		
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
	//cout << "VQ Step..." << endl;
	
	int lastPercent = 0;

	for (int i = 0; i < features.size(); ++i )
	{
		int progress = (int)(ceil( (double)i / (double)(features.size())*100 ));
		if( progress != lastPercent )
		{
			//cout << progress << "%" << endl;
			lastPercent = progress;
		}
		int word = featureVQ( features[i] );
		bow[ word ] += 1;
	}
	//cout << "Done!" << endl;

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
	/*
	* TODO: kD-Tree descent instead of naive O( n . d . |w| )
	*/
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
			float centerValue = _vocabulary[i][j];
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
	if( _videoPool == 0 )
		return;

	CProfiler prof;
	//For each video entry...
	std::cout << "####### BoVW - Computing Features #######" << endl;
	double progress = 0;
	const int corpusSize = _videoPool->getVideoEntries().size();
	
	double accProcTime = 0;
	double ETA = 0;
	int ETAs;
	int resteETAs;
	int ETAm;
	int resteETAm;
	int ETAh;

	for( int i = 0; i < corpusSize; ++i )
	{
		prof.start();
		//Compute features for one video
		vector<STFeature> entryFeatures;
		computeVideoFeatures( _videoPool->getVideoEntries()[i].getFileName(), entryFeatures );
		//Normalize motion features
		for( int i = 0; i < entryFeatures.size(); ++i )
		{
			entryFeatures[i].normalizeMotionFeature();
		}
		//
		_videoPool->getVideoEntries()[i].setFeatures( entryFeatures );
		std::cout << "extracted " << entryFeatures.size() << " ST Features" << std::endl;
		_extractor->clear();
		prof.stop();
		accProcTime += prof.getSeconds();
		ETA = (accProcTime/((double)(i+1)))*((double)corpusSize);
		ETAs = (int)ETA;
		resteETAs = ETAs%60;
		ETAm = ETAs / 60;
		resteETAm = ETAm%60;
		ETAh = ETAm/60;

		progress = ((double)i / (double)( corpusSize ))*100.0;
		std::cout	<< "## Feature Computation Progress - "
				<< i+1 << "/" << corpusSize << " - " 
				<< fixed << setprecision(2) << progress << "%" 
				<< " - " << "ETA=" << ETAh << "h" << resteETAm << "m" << resteETAs << "s" << endl;
	}
	std::cout << "####### Done #######" << endl;
}

/*
*	Build Video Vocabulary from Video Corpus
*/
void BOW::buildVocabulary()
{
	if( _videoPool == 0 )
		return;
	
	if( _videoPool->getVideoEntries().size() <= 0 )
		return;
	//

	//Use kMeans clustering to build vocabulary
	//
	// 0 - Compute required memory space
	int szFeaturePool = 0;
	int szWord = _extractor->getFeatureLength();
	int nFeatures = 0;
	for (int i = 0; i < _videoPool->getVideoEntries().size(); ++i )
	{
		int nFeat = _videoPool->getVideoEntries()[i].getFeatures().size();
		szFeaturePool += (nFeat * szWord);
		nFeatures += nFeat;
	}
	
	// 1 - Create feature space for clustering
	_featurePool = new double[ szFeaturePool ];
	//Copy feature data
	//For each video
	int arrayOffset = 0;
	for( int i = 0; i < _videoPool->getVideoEntries().size(); ++i )
	{
		//for each computed feature
		for( int j = 0; j < _videoPool->getVideoEntries()[i].getFeatures().size(); ++j )
		{
			vector<float> feat = _videoPool->getVideoEntries()[i].getFeatures()[j].getAggregatedFeature();
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

	// 4 - Tidy up eveything
	// Copy raw data from kMeans to more structured DS
	_vocabulary.resize( _vocabSize );
	for( int i = 0; i < _vocabSize; ++i )
	{
		_vocabulary[i].resize( szWord );
		for( int j = 0; j < szWord; ++j )
		{
			_vocabulary[i][j] = _clusterCenters[ szWord * i + j];
		}
	}
	//
	// Remove kMeans raw data from memory
	delete[] _clusterCenters;
	delete[] _kmeansResults;
	delete[] _featurePool;
	_clusterCenters = 0;
	_kmeansResults = 0;
	_featurePool = 0;
}

void BOW::computeBoW()
{
	if( _videoPool == 0 )
		return;

	for( int i = 0; i < _videoPool->getVideoEntries().size(); ++i )
	{
		computeBoW( _videoPool->getVideoEntries()[i].getFeatures(), _videoPool->getVideoEntries()[i].getBoWVector() );
	}
}

/*
*	Save the computed vocabulary for further use...
*/
void BOW::saveVocabulary( std::string fileName )
{
	ofstream ofs( fileName.c_str() );
	int szWord = _extractor->getFeatureLength();

	//Write Magic Number
	ofs << "26098677" << endl;

	//Write vocabulary size (# of words)
	ofs << this->_vocabSize << endl;

	//Write word length (# of floating point values)
	ofs << szWord << endl;

	//Write CSV codeword data
	for( int i = 0; i < _vocabulary.size(); ++i )
	{
		for( int j = 0; j < szWord; ++j )
		{
			ofs << _vocabulary[i][j] << ";";
		}
		ofs << endl;
	}

	//Write reversed Magic Number (EOF)
	ofs << "77689062" << endl;
}

/*
* Load the vocabulary file
*/
int BOW::loadVocabulary( std::string fileName )
{
	ifstream ifs( fileName.c_str() );
	string buffer;
	
	int vocabSize;
	int szWord;

	//try to retrieve magic number
	getline( ifs, buffer );
	if( buffer != "26098677" )
		return -1;
		
	//retrieve the number of words
	getline( ifs, buffer );
	if( buffer.size() != 0 )
	{
		vocabSize = atoi( buffer.c_str() );
		this->setVocabSize( vocabSize );
	}
	else
		return -1;

	//retrieve word length
	getline( ifs, buffer );
	if( buffer.size() != 0 )
		szWord = atoi( buffer.c_str() );

	//Loop through |vocabSize| lines and retrieve codewords
	_vocabulary.resize( vocabSize );
	for( int i = 0; i < vocabSize; ++i )
	{
		_vocabulary[i].resize( szWord );
		getline( ifs, buffer );
		std::vector< std::string > tokenList = split( buffer, ';' );
		if( tokenList.size() == szWord )
		{
			for( int j = 0; j < tokenList.size(); ++j )
			{
				_vocabulary[i][j] = atof( tokenList[j].c_str() );
			}
		}
		else
			return -1;
	}

	//Look for EOF marker
	getline( ifs, buffer );
	if( buffer != "77689062" )
		return -1;

	return 0;
}

/*
* Classification
*/
void BOW::createClassificationModel()
{
	if( _classifier != 0 )
		_classifier->createModel( this->_videoPool, this );
}

void BOW::saveClassificationModel( std::string fileName )
{
	if( _classifier != 0 )
		_classifier->saveModel( fileName );
}

void BOW::loadClassificationModel( std::string fileName )
{
	if( _classifier != 0 )
		_classifier->loadModel( fileName );
}

std::string BOW::label( std::vector< cv::Mat > frames, vector<float>& truthValues )
{
	//Compute features from frames
	_extractor->computeFeatures( frames, 0 );

	//Check if we have enough features
	if( _extractor->getFeatures().size() < _featureCountThreshold )
		return "not_enough_features";

	//Compute BoVW
	vector<float> bowValues;
	this->computeBoW( _extractor->getFeatures(), bowValues );
	 
	//Label
	return this->label( bowValues, truthValues );
}

std::string BOW::label( std::vector< float > bowValues, vector<float>& truthValues )
{
	string result;

	if( _videoPool == 0 )
		_videoPool = new CVideoPool();

	if( _classifier != 0 )
	{
		result = _classifier->label( bowValues, _videoPool, this, truthValues );
	}
	else
		result = "notclassified";

	return result;
}

std::string BOW::label( std::string videoName, vector<float>& truthValues )
{
	string result;
	//Compute features from video

	//Compute BoVW

	//Label

	return result;
}

#undef BOW
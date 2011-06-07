#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

#include "STExtractor.h"
#include "..\kmpp\KMeans.h"

using namespace cv;
using namespace std;

const int N = 20;

vector< vector< float > > rawVocabulary;

int main( int argc, char** argv )
{

	int width = 320;
	int height = 240;
	int frameCpt = 0;

	vector< Mat > frames;
	vector<STFeature> stFeatures;
	
	VideoCapture cap( "Z:\\HumanDetection\\videos_hbd\\6pc0.avi" );
	ofstream ofs("stfeatures_tracks.csv");


	//VideoCapture cap(0);

	STExtractor ste(N,5);

	cv::namedWindow("test");
	cv::namedWindow("fback");

	Mat cossu(cv::Size(width,height),CV_8UC3);
	Mat frame;

	cv::setNumThreads(10);

	for(;;)
	{
		
		cap >> frame;

		if( frame.empty() )
			break;

		//Conversion RGB -> GRAY
		Mat frameg;
		Mat frameg2;
		Mat hardCopy;
		cv::cvtColor( frame, frameg, CV_RGB2GRAY );
		cv::resize( frameg, frameg2, cv::Size(width,height));
		frameg2.copyTo(hardCopy);
		frames.push_back( hardCopy );

		/*
		double t = (double)getTickCount();
		// do something ...
		t = ((double)getTickCount() - t)/getTickFrequency();
		*/

		//on a assez de frames
		if( frames.size() == N )
		{
			stFeatures.clear();

			double t = (double)getTickCount();
			stFeatures = ste.extract( frames, frameCpt );
			t = ((double)getTickCount() - t)/getTickFrequency();
			cout << "STFeatures extraction t=" << t << " s." << endl;
			frames.clear();

			for( int k = 0; k < stFeatures.size(); ++k )
			{
				vector< cv::Point2f > track			= stFeatures[k].getMotionTrack();
				vector< float > timeStamps			= stFeatures[k].getTimeStamps();
				vector< float > motionFeatures		= stFeatures[k].getMotionFeature();
				vector< float > finalFeatures		= stFeatures[k].getAggregatedFeature();

				for( int l = 0; l < track.size(); ++l )
				{
					ofs << timeStamps[l] << ";" << (int)(track[l].x) << ";" << (int)(track[l].y) << endl;
				}

				rawVocabulary.push_back( finalFeatures );
			}
		}

		for( int k = 0; k < stFeatures.size(); ++k )
		{
			vector< cv::Point2f > track = stFeatures[k].getMotionTrack();

			for( int i = 0; i < track.size()-1; ++i )
			{
				int colinterp = (255/track.size())*i;
				cv::line( frame, track[i], track[i+1], CV_RGB(colinterp,0,255-colinterp), 1);
				cv::line( cossu, track[i], track[i+1], CV_RGB(colinterp,0,255-colinterp), 1);
			}
		}

		imshow("test", frame);
		char c = cv::waitKey(2);

		if( c == 27 )
			break;

		frameCpt++;
	}

	//Generate codebook by kMeans clustering ?
	const int wordSize = 84;
	const int vocabSize = 256;
	//Get the raw vocab data size in bytes
	const int rawVocabSize = rawVocabulary.size() * wordSize * sizeof(float);
	double* highdPoints =	new double[ rawVocabulary.size() * wordSize ];
	double* clusterCenters = new double[ wordSize * vocabSize ];
	int* results =			new int[ rawVocabulary.size() ];
	const int restarts = 1;
	//Need to copy highD points
	cout << "Preparing data for k-Means clustering..." << endl;
	for( int i = 0; i < rawVocabulary.size(); ++i )
	{
		for( int j = 0; j < wordSize; ++j )
		{
			highdPoints[ i * wordSize + j ] = (double)rawVocabulary[i][j];
		}
	}
	cout << "Starting k-Means clustering..." << endl;
	AddKMeansLogging( &cout, true );
	RunKMeans( rawVocabulary.size(), vocabSize, wordSize, highdPoints, restarts, clusterCenters, results );  
	cout << "Done!" << endl;
	//

	/*
	-- Bag of Video Word ? ****************************
	*/
	cout << "Building BoVW for the video..." << endl;
	vector< float > bovw;
	bovw.resize( vocabSize );
	for( int i = 0; i < rawVocabulary.size(); ++i )
	{
		//Vector quantization...
		bovw[ results[ i ] ] += 1.0f;
	}
	//Histogram normalization
	cout << "Histogram normalization..." << endl;
	float sqsum = 0.0f;
	for( int i = 0; i < vocabSize; ++i )
		sqsum += bovw[i]*bovw[i];
	float histoVecLen = sqrtf( sqsum );
	for( int i = 0; i < vocabSize; ++i )
		bovw[i] /= histoVecLen;

	//Saving video BoVW ?
	cout << "Saving BoVW file..." << endl;
	ofstream bovwfs("stfeature_bovw.csv");
	for( int i = 0; i < vocabSize; ++i )
	{
		bovwfs << bovw[ i ] << ";" << endl; 
	}
	cout << "Done!" << endl;
	/*
	***************************************************
	*/

	//Saving vocabulary ?
	cout << "Saving vocabulary file..." << endl;
	ofstream vocfs("stfeature_voc.csv");
	for( int i = 0; i < vocabSize; ++i )
	{
		for( int j = 0; j < wordSize; ++j )
		{
			vocfs << clusterCenters[ i*wordSize + j ] << ";";
		}
		vocfs << endl;
	}
	cout << "Done!" << endl;
	//

	ofs.close();
	vocfs.close();
	bovwfs.close();

	cv::imwrite("cossu.png", cossu);

	return 0;
}
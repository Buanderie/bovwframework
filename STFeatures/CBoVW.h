#ifndef __CBOVW_H__
#define __CBOVW_H__

#include <string>
#include <vector>

#include "STFeature.h"
#include "VideoPool.h"
#include "ISTExtractor.h"
#include "IClassifier.h"

class CBoVW
{
	private:
		void init();
	
		void computeVideoFeatures( std::string fileName, std::vector<STFeature>& featureVec );
		void computeBoW( std::vector<STFeature>& features, std::vector< float >& output ); 
		int featureVQ( STFeature& feat );

		int _vocabSize;
		std::vector< std::vector< float > > _vocabulary;
		ISTExtractor*	_extractor;
		IClassifier*	_classifier;
		CVideoPool*		_videoPool;

		//k-Means stuff
		double* _featurePool;
		double* _clusterCenters;
		int*	_kmeansResults; 
		//

	public:
		CBoVW();
		virtual ~CBoVW();

		int getVocabSize(){ return _vocabSize; }
		void setVocabSize( int val ){ _vocabSize = val; }

		void setExtractor( ISTExtractor* extractor );
		void setClassifier( IClassifier* classifier );
		void setVideoPool( CVideoPool* videoPool );

		/*
		* Classification
		*/
		void createClassificationModel();
		void label( std::vector< cv::Mat > frames );
		void label( std::vector< float > bowValues );
		void label( std::string videoName );

		/*
		* BoW Computation
		*/
		void computeFeatures();
		void buildVocabulary();
		void computeBoW();		

		/*
		* I/O Functions
		*/
		void saveClassificationModel( std::string fileName );
		void loadClassificationModel( std::string fileName );
		void saveVocabulary( std::string fileName );
		int loadVocabulary( std::string fileName );
};

#endif
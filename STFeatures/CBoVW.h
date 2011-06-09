#ifndef __CBOVW_H__
#define __CBOVW_H__

#include <string>
#include <vector>

#include "STFeature.h"
#include "ISTExtractor.h"

typedef struct classtype
{
	int			classId;
	std::string className;
} classtype_t;

typedef struct videntry
{
	std::string videoFileName;
	std::string className;
	std::vector< STFeature > _features;
	std::vector< float > _bag;
} videntry_t;

class CBoVW
{
	private:
		void init();
	
		void computeVideoFeatures( std::string fileName, std::vector<STFeature>& featureVec );
		void computeBoW( std::vector<STFeature>& features, std::vector< float >& output ); 
		int featureVQ( STFeature& feat );

		int _vocabSize;
		static const int _maxClass = 10000;
		std::vector< classtype_t >			_classes;
		std::vector< videntry_t >			_videoEntry;
		std::vector< std::vector< float > > _vocabulary;
		ISTExtractor*	_extractor;

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

		classtype_t addClass( std::string className );
		void addVideo( std::string fileName, std::string className );

		void computeFeatures();
		void computeVocabulary();
		void computeBoW();		

		std::vector< videntry_t >& getVideoEntries();

		/*
		* I/O Functions
		*/
		void saveVocabulary( std::string fileName );
		int loadVocabulary( std::string fileName );
};

#endif
#include <iostream>
#include <fstream>
#include <iomanip>

#include "CBoVW.h"

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
	CBoVW bow;

	bow.setVocabSize( 100 );

	bow.addClass( "class1" );
	bow.addVideo( "Z:\\HumanDetection\\videos_hbd\\6pc0m.avi", "class1" );
	bow.addVideo( "Z:\\HumanDetection\\videos_hbd\\6pc0.avi", "class1" );

	bow.computeFeatures();
	bow.computeVocabulary();
	bow.computeBoW();

	ofstream ofs("yodawg.csv");
	vector< videntry_t > entryList = bow.getVideoEntries();
	for( int i = 0; i < entryList.size(); ++i )
	{
		cout << "# Bag of Video Words #" << endl;
		for (int j = 0; j < entryList[i]._bag.size(); ++j )
		{
			ofs << entryList[i]._bag[j] << ";" << endl;
			cout << entryList[i]._bag[j] << ";";
		}
		cout << "\n######################" << endl;
	}

	//Computing Bhattacharyya coefficients
	float total1 = 0.0f;
	float total2 = 0.0f;
	int vocabSize = bow.getVocabSize();
	for( int i = 0; i < vocabSize; ++i )
	{
		total1 += entryList[0]._bag[i];
		total2 += entryList[1]._bag[i];
	}
	float div = sqrtf( total1*total2 );
	float sqsum = 0.0f;
	for( int i = 0; i < vocabSize; ++i )
	{
		sqsum += sqrtf( entryList[0]._bag[i] * entryList[1]._bag[i] ) / div;
	}
	float bhatt = sqrtf( max(1.0f - sqsum, 0.0f) );
	
	return 0;
}
#include "GenUtils.h"

#include <vector>
#include <string>
#include <sstream>

using namespace std;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

float computeBhattacharyya( vector<float>& histoA, vector<float>& histoB )
{
	if( histoA.size() != histoB.size() )
		return 1.0f;

	//Computing Bhattacharyya coefficients
	float total1 = 0.0f;
	float total2 = 0.0f;
	int vocabSize = histoA.size();
	for( int i = 0; i < vocabSize; ++i )
	{
		total1 += histoA[i];
		total2 += histoB[i];
	}
	float div = sqrtf( total1*total2 );
	float sqsum = 0.0f;
	for( int i = 0; i < vocabSize; ++i )
	{
		sqsum += sqrtf( histoA[i] * histoB[i] ) / div;
	}

	float bhatt = sqrtf( max(1.0f - sqsum, 0.0f) );

	return bhatt;
}
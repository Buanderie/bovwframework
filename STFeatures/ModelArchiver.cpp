#include "ModelArchiver.h"

//STL
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

//Boost
#include <boost/crc.hpp>

using namespace std;
using namespace boost;

#define MODARCH CModelArchiver

MODARCH::MODARCH()
{

}

MODARCH::~MODARCH()
{

}

/*
HEADER Format:
0-model_file_name	(string)
--File size in bytes
--CRC
1-number_of_classes	(int)
2-class_name_1
3-class_name_2
...
n-MODEL_FILE_BEGIN
Model data
x-MODEL_FILE_END
*/

void MODARCH::write( vector<string> classList,
			std::string modelFileName,
			std::string outputFileName
		)
{
	//Model file input
	ifstream ifsb(modelFileName.c_str(), std::ios_base::binary);
	
	//Archive file output
	ofstream ofs(outputFileName.c_str(), std::ios_base::binary);

	//CRC
	boost::crc_32_type crcResult;

	//Read model file data
	const int buffer_size = 1024;
	char * buffer = new char[ buffer_size ];
	int readByte = 0;
	do
	{
		ifsb.read( buffer, buffer_size );
		crcResult.process_bytes( buffer, ifsb.gcount() );
		readByte += ifsb.gcount();
	} while( ifsb );
	ifsb.close();

	//Write header
	ofs << modelFileName << endl;
	ofs << readByte << endl;
	ofs << hex << uppercase << crcResult.checksum() << endl;
	ofs << classList.size() << endl;
	for( int i = 0; i < classList.size(); ++i )
	{
		ofs << classList[i] << endl;
	}

	//MODEL_FILE_BEGIN
	ofs << "MODEL_FILE_BEGIN" << endl;

	//Write model file data
	//Reset file pointer to beginning
	ifsb.open(modelFileName.c_str(), std::ios_base::binary);
	do
	{
		ifsb.read( buffer, buffer_size );
		int readCount = ifsb.gcount();
		ofs.write( buffer, readCount );
	} while( ifsb );
	ifsb.close();

	//MODEL_FILE_END
	ofs << "MODEL_FILE_END" << endl;

	delete[] buffer;
}
				
void MODARCH::read(	std::string archiveFileName,
					std::string& outputModelFileName )
{
	ifstream ifs( archiveFileName.c_str(), std::ios_base::binary );
	
	//
	string modelFileName;
	int modelFileSize;
	string strCRC;
	int classCount;
	//

	string strBuffer;

	//Retrieve file name
	getline( ifs, strBuffer );
	modelFileName = string("out_")+strBuffer;
	outputModelFileName = modelFileName;

	//Retrieve byte count
	getline( ifs, strBuffer );
	modelFileSize = atoi( strBuffer.c_str() );

	//Retrieve CRC
	getline( ifs, strBuffer );
	strCRC = strBuffer;

	//retrieve class count
	getline( ifs, strBuffer );
	classCount = atoi( strBuffer.c_str() );

	//retrieve class names
	_classList.clear();
	for( int i = 0; i < classCount; ++i )
	{
		getline( ifs, strBuffer );
		_classList.push_back( strBuffer );
	}

	//Check file start
	getline( ifs, strBuffer );
	if( strBuffer != string("MODEL_FILE_BEGIN") )
		return;

	//Write model file
	char* buffer = new char[ modelFileSize ];
	ofstream ofs( modelFileName.c_str(), std::ios_base::binary );
	ifs.read( buffer, modelFileSize );
	ofs.write( buffer, modelFileSize );
	delete[] buffer;

	//Check for file end
	getline( ifs, strBuffer );
	if( strBuffer != string("MODEL_FILE_END") )
		return;

	ifs.close();
	ofs.close();

	//Check CRC
	boost::crc_32_type result;
	const int buffer_size = 1024;
	buffer = new char[ buffer_size ];
	ifs.open( modelFileName, std::ios_base::binary );
	do
    {
        ifs.read( buffer, buffer_size );
        result.process_bytes( buffer, ifs.gcount() );
	} while ( ifs );
	delete[] buffer;
	ifs.close();

	stringstream sstr;
	sstr << hex << uppercase << result.checksum();

	strBuffer = sstr.str();
	if( strBuffer != strCRC )
		return;
}

#undef MODARCH
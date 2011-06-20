#include "VideoPool.h"

//STL
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

//Boost
#include <boost/filesystem.hpp>

//RapidXML
#include "..\rapidxml\rapidxml.hpp"

//Internal
#include "VideoEntry.h"

#define POOL CVideoPool

using namespace std;
using namespace rapidxml;
using namespace boost::filesystem;

/*
UTILS
*/
string LoadXMLFile( string Filename )
{
	string ret = "";
	ifstream fin( Filename.c_str() );
	string buffer;
	while( getline( fin, buffer ) )
	{
		if( fin.eof() )
			break;
		ret += buffer;
	}
	return ret;
}

POOL::POOL()
{

}

POOL::~POOL()
{

}

std::vector< std::string > POOL::getClassList()
{
	vector<string> returnList;
	string sName;
	bool toAdd = true;

	// Naive O(n^2) algorithm... No big deal 
	for( int i = 0; i < _videoEntries.size(); ++i )
	{
		toAdd = true;
		sName = _videoEntries[i].getClassName();
		for( int j = 0; j < returnList.size(); ++j )
		{
			if( sName == returnList[j] )
			{
				toAdd = false;
				break;
			}
		}
		if( toAdd )
			returnList.push_back( sName );
	}

	return returnList;
}

std::vector<CVideoEntry>& POOL::getVideoEntries()
{
	return _videoEntries;
}

void POOL::addVideoEntry( CVideoEntry entry )
{
	_videoEntries.push_back( entry );
}

void POOL::loadFromXml( std::string xmlFile )
{
	ifstream ifs(xmlFile.c_str());
	if( !ifs.is_open() )
		return;

	//Recupere l'integralite du texte
	string tmpBuffer;
	string xmlText = "";
	while(1)
	{
		if( ifs.eof() )
			break;
		getline(ifs, tmpBuffer);
		if( tmpBuffer.size() > 0 )
			xmlText += tmpBuffer;
	}

	xml_document<> doc;
	doc.parse<0>( (char*)xmlText.c_str() );

	xml_node<>* nRoot = doc.first_node( "videopool" );
	
	for( xml_node<> *nEntry = nRoot->first_node("entry",5); nEntry != 0; nEntry = nEntry->next_sibling("entry",5))
	{
		xml_attribute<>* nClass = nEntry->first_attribute("class",5);
		string className = nClass->value();
		string fileName = nEntry->value();
		this->addVideoEntry( CVideoEntry( fileName, className ) );
	}

	for( xml_node<> *nEntry = nRoot->first_node("folderentry",11); nEntry != 0; nEntry = nEntry->next_sibling("folderentry",11))
	{
		xml_attribute<>* nClass = nEntry->first_attribute("class",5);
		string className = nClass->value();
		string folderName = nEntry->value();
		
		path p( folderName.c_str() );
		if( exists(p) && is_directory(p) )
		{
			vector<path> vec;
			copy( directory_iterator(p), directory_iterator(), back_inserter(vec));

			for (vector<path>::const_iterator it (vec.begin()); it != vec.end(); ++it)
			{
				stringstream sstr;
				sstr << (*it);
				string fileName;
				sstr >> fileName;
				fileName = (*it).string();
				this->addVideoEntry( CVideoEntry( fileName, className ) );
	        }
		}
	}
}

#undef POOL
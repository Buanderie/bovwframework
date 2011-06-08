#ifndef __GENUTILS_H__
#define __GENUTILS_H__

#include <vector>
#include <string>

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
float computeBhattacharyya( std::vector<float>& histoA, std::vector<float>& histoB );

#endif
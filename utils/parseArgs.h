#pragma once


#include <iostream>
#include <vector>
#include "rkcommon/math/vec.h"

using namespace rkcommon::math;

struct Args
{
    std::string extension;
    std::string filename;
    vec3i dims;
    std::string dtype;
};

std::string getFileExt(const std::string& s);

void parseArgs(int argc, const char **argv, Args &args);
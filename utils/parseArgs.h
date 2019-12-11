#pragma once


#include <iostream>
#include <vector>

struct Args
{
    std::string extension;
    std::string filename;
};

std::string getFileExt(const std::string& s);

void parseArgs(int argc, const char **argv, Args &args);
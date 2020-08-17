#pragma once

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>

#include "rkcommon/math/vec.h"

using namespace rkcommon::math;

struct Volume
{
    std::vector<float> voxels;
    vec2f range;
    vec3i dims;
    vec3f spacing = vec3f{1.f, 1.f, 1.f};
    Volume();
};

Volume::Volume(){}

bool loadRaw(const std::string fileDir, const vec3i dims, Volume &volume)
{
    volume.dims = dims;

    std::ifstream input(fileDir.c_str(), std::ios::binary);

    // find file size
    input.seekg(0, input.end); 
    int filesize = input.tellg();              
    input.seekg(0, input.beg);
    
    // std::cout << "size of float " << sizeof(float) << std::endl;
    // std::cout << "file size = " << filesize << std::endl;

    // Here is the size of data
    // volume.voxels.resize(16*16*16);
    volume.voxels.resize(filesize / sizeof(float));
    input.read(reinterpret_cast<char*>(volume.voxels.data()), volume.voxels.size()*sizeof(float));

    // find the data range
    float minimum = *(std::min_element(volume.voxels.begin(), volume.voxels.end()));
    float maximum = *(std::max_element(volume.voxels.begin(), volume.voxels.end()));
    volume.range = rkcommon::math::vec2f(minimum, maximum);

    std::cout << "volume info: " << std::endl;
    std::cout << "volume dims " << volume.dims << std::endl;
    std::cout << "volume range " << volume.range << std::endl;

    if(volume.voxels.size() != 0){
        return true;
    }

    return false;
}
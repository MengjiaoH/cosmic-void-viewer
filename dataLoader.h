#pragma once

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>

#include "ospcommon/math/vec.h"

struct Volume
{
    std::vector<float> voxels;
    ospcommon::math::vec2f range;
    ospcommon::math::vec3i dims;
    Volume(ospcommon::math::vec3i dimensions);
};

Volume::Volume(ospcommon::math::vec3i dimensions): dims(dimensions)
{}

bool loadRaw(const std::string fileDir, Volume &volume)
{
    std::ifstream input(fileDir.c_str(), std::ios::binary);

    // find file size
    input.seekg(0, input.end); 
    int filesize = input.tellg();              
    input.seekg(0, input.beg);

    // Here is the size of data
        // volume.voxels.resize(16*16*16);
    volume.voxels.resize(filesize / sizeof(float));
    // volume.voxels.resize(filesize / sizeof(float));
    input.read(reinterpret_cast<char*>(volume.voxels.data()), volume.voxels.size()*sizeof(float));

    // find the data range
    float minimum = *(std::min_element(volume.voxels.begin(), volume.voxels.end()));
    float maximum = *(std::max_element(volume.voxels.begin(), volume.voxels.end()));
    volume.range = ospcommon::math::vec2f(minimum, maximum);

    if(volume.voxels.size() != 0){
        return true;
    }

    return false;
}
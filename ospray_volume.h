#pragma pnce 

#include <iostream>
#include <vector>

#include "ospray/ospray_cpp.h"
#include "rkcommon/math/vec.h"
#include "rkcommon/math/box.h"

#include "dataLoader.h"


ospray::cpp::Volume make_ospray_volume(Volume v, rkcommon::math::box3f &worldBound){
    
    ospray::cpp::Volume volume("structured_volume");
    const rkcommon::math::vec3i dims(v.dims.x, v.dims.y, v.dims.z);

    rkcommon::math::vec3f spacing{.5f};
    auto numVoxels = dims.product();

    std::vector<float> data;

    for(int i = 0; i < v.voxels.size(); i++){
        // std::cout << v.voxels[i] << " ";
        data.push_back(v.voxels[i]);
    }
    std::cout << std::endl;

    volume.setParam("voxelType",int(OSP_FLOAT));
    volume.setParam("dimensions", dims);
    volume.setParam("voxelData", ospray::cpp::Data(data));
    // volume.setParam("gridSpacing", rkcommon::math::vec3i(2.5f));
    worldBound = rkcommon::math::box3f(rkcommon::math::vec3f(0.f), dims * spacing);

    volume.commit();
  
    return volume;
}

// ospray::cpp::Volume recommit_ospray_volume(Volume<float> v, rkcommon::math::vec2f &range){
//     ospray::cpp::Volume volume("structured_volume");
//     const rkcommon::math::vec3i dims(v.dim.x, v.dim.y, v.dim.z);
//     rkcommon::math::vec3f spacing{1.f};

//     auto numVoxels = dims.product();
//     volume.setParam("voxelType",int(OSP_FLOAT));
//     volume.setParam("dimensions", dims);
//     volume.setParam("voxelData", ospray::cpp::Data(numVoxels, OSP_FLOAT, v.voxels.data()));
//     range  = v.range;

//     volume.commit();

//     return volume;
// }


void update_transfer_fcn(ospray::cpp::TransferFunction &tfcn, const std::vector<uint8_t> &colormap, rkcommon::math::vec2f valueRange) {
    std::vector<rkcommon::math::vec3f> colors;
    std::vector<float> opacities;

    for (size_t i = 0; i < colormap.size() / 4; ++i) {
        rkcommon::math::vec3f c(colormap[i * 4] / 255.f, colormap[i * 4 + 1] / 255.f, colormap[i * 4 + 2] / 255.f);
        colors.push_back(c);
        if(colormap[i * 4 + 3] / 255.f < 0.1f){
            opacities.push_back(0.f);
        }else{
            opacities.push_back(colormap[i * 4 + 3] / 255.f);
        }
        // std::cout << colormap[i * 4 + 3] / 255.f << " ";
    }
    // std::cout << std::endl;
    tfcn.setParam("color", ospray::cpp::Data(colors));
    ospray::cpp::Data opacity = ospray::cpp::Data(opacities);
    tfcn.setParam("opacity", opacity);
    // tfcn.setParam("opacity", ospray::cpp::Data(opacities));
    tfcn.setParam("valueRange", valueRange);
    tfcn.commit();

}

void looping_transfer_fcn(ospray::cpp::TransferFunction &tfcn, rkcommon::math::vec2f valueRange, int temp){
    int total = (valueRange.y - valueRange.x) / 0.1f;
    std::vector<rkcommon::math::vec3f> colors;
    std::vector<float> opacities;

    for (size_t i = 0; i < total; ++i) {
        rkcommon::math::vec3f c(0.f / 255.f, 0.f/ 255.f, 255.f/ 255.f);
        colors.push_back(c);
        if(i < temp){
            opacities.push_back(1.f);
        }else{
            opacities.push_back(1.0f);
        }
        // std::cout << colormap[i * 4 + 3] / 255.f << " ";
    }
    // std::cout << std::endl;
    tfcn.setParam("color", ospray::cpp::Data(colors));
    ospray::cpp::Data opacity = ospray::cpp::Data(opacities);
    tfcn.setParam("opacity", opacity);
    // tfcn.setParam("opacity", ospray::cpp::Data(opacities));
    tfcn.setParam("valueRange", valueRange);
    tfcn.commit();
}
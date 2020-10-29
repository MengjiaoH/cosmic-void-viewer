#pragma once

#include <vector>

#include "ospray/ospray_cpp.h"
#include "ospray/ospray_util.h"
#include "rkcommon/math/vec.h"
#include "rkcommon/math/box.h"

#include "dataLoader.h"

using namespace rkcommon::math;

ospray::cpp::Volume createStructuredVolume(const Volume volume)
{
  ospray::cpp::Volume osp_volume("structuredRegular");

  auto voxels = *(volume.voxel_data);
// vec3f(-volume.dims.x/ 2.f, -volume.dims.y/2.f, -volume.dims.z/2.f)
  osp_volume.setParam("gridOrigin", vec3f(-volume.dims.x/ 2.f, -volume.dims.y/2.f, -volume.dims.z/2.f));
  osp_volume.setParam("gridSpacing", vec3f(2.f));
  osp_volume.setParam("data", ospray::cpp::CopiedData(voxels.data(), volume.dims));
  osp_volume.commit();
  return osp_volume;
}
 

std::vector<float> getAllIsoValues(const Volume volume, float iso_value)
{
  std::vector<float> iso_values;

  auto voxels = *(volume.voxel_data);
  // std::cout << voxels.size() << std::endl;

  for( int i = 0; i < voxels.size(); i++){
    if(voxels[i] < iso_value){
      iso_values.push_back(voxels[i]);
    }
  }
  // std::cout << iso_values.size() << std::endl;
  
  return iso_values;
}

// void update_transfer_fcn(ospray::cpp::TransferFunction &tfcn, const std::vector<uint8_t> &colormap, rkcommon::math::vec2f valueRange) {
//     std::vector<rkcommon::math::vec3f> colors;
//     std::vector<float> opacities;

//     for (size_t i = 0; i < colormap.size() / 4; ++i) {
//         rkcommon::math::vec3f c(colormap[i * 4] / 255.f, colormap[i * 4 + 1] / 255.f, colormap[i * 4 + 2] / 255.f);
//         colors.push_back(c);
//         if(colormap[i * 4 + 3] / 255.f < 0.1f){
//             opacities.push_back(0.f);
//         }else{
//             opacities.push_back(colormap[i * 4 + 3] / 255.f);
//         }
//         // std::cout << colormap[i * 4 + 3] / 255.f << " ";
//     }
//     // std::cout << std::endl;
//     tfcn.setParam("color", ospray::cpp::Data(colors));
//     ospray::cpp::Data opacity = ospray::cpp::Data(opacities);
//     tfcn.setParam("opacity", opacity);
//     // tfcn.setParam("opacity", ospray::cpp::Data(opacities));
//     tfcn.setParam("valueRange", valueRange);
//     tfcn.commit();

// }

// void looping_transfer_fcn(ospray::cpp::TransferFunction &tfcn, rkcommon::math::vec2f valueRange, int temp){
//     int total = (valueRange.y - valueRange.x) / 0.1f;
//     std::vector<rkcommon::math::vec3f> colors;
//     std::vector<float> opacities;

//     for (size_t i = 0; i < total; ++i) {
//         rkcommon::math::vec3f c(0.f / 255.f, 0.f/ 255.f, 255.f/ 255.f);
//         colors.push_back(c);
//         if(i < temp){
//             opacities.push_back(1.f);
//         }else{
//             opacities.push_back(1.0f);
//         }
//         // std::cout << colormap[i * 4 + 3] / 255.f << " ";
//     }
//     // std::cout << std::endl;
//     tfcn.setParam("color", ospray::cpp::Data(colors));
//     ospray::cpp::Data opacity = ospray::cpp::Data(opacities);
//     tfcn.setParam("opacity", opacity);
//     // tfcn.setParam("opacity", ospray::cpp::Data(opacities));
//     tfcn.setParam("valueRange", valueRange);
//     tfcn.commit();
// }

ospray::cpp::TransferFunction makeTransferFunction(const std::vector<uint8_t> colormap, const vec2f &valueRange)
{
    ospray::cpp::TransferFunction transferFunction("piecewiseLinear");

    // std::vector<vec3f> colors;
    // std::vector<float> opacities;

    // if (tfColorMap == "jet") {
    //     colors.emplace_back(0, 0, 0.562493);
    //     colors.emplace_back(0, 0, 1);
    //     colors.emplace_back(0, 1, 1);
    //     colors.emplace_back(0.500008, 1, 0.500008);
    //     colors.emplace_back(1, 1, 0);
    //     colors.emplace_back(1, 0, 0);
    //     colors.emplace_back(0.500008, 0, 0);
    // } else if (tfColorMap == "rgb") {
    //     colors.emplace_back(0, 0, 1);
    //     colors.emplace_back(0, 1, 0);
    //     colors.emplace_back(1, 0, 0);
    // } else {
    //     colors.emplace_back(0.f, 0.f, 0.f);
    //     colors.emplace_back(1.f, 1.f, 1.f);
    // }

    // std::string tfOpacityMap = "linear";

    // if (tfOpacityMap == "linear") {
    //     opacities.emplace_back(0.f);
    //     opacities.emplace_back(1.f);
    // }

    std::vector<vec3f> colors;
    std::vector<float> opacities;

    for (size_t i = 0; i < colormap.size() / 4; ++i) {
        vec3f c(colormap[i * 4] / 255.f, colormap[i * 4 + 1] / 255.f, colormap[i * 4 + 2] / 255.f);
        colors.push_back(c);
        if(colormap[i * 4 + 3] / 255.f < 0.1f){
            opacities.push_back(0.f);
        }else{
            opacities.push_back(colormap[i * 4 + 3] / 255.f);
        }
    }

    transferFunction.setParam("color", ospray::cpp::CopiedData(colors));
    transferFunction.setParam("opacity", ospray::cpp::CopiedData(opacities));
    transferFunction.setParam("valueRange", valueRange);
    transferFunction.commit();

    return transferFunction;
}
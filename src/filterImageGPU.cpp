/*
 * filterImg.cpp
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

/*
 * tarpFind.cpp
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#include <chrono>
#include <future>
#include <iostream>            //Input/output library
#include <opencv2/opencv.hpp>  //OpenCV library
#include <string>
#include <vector>
#include "Tarp.h"
#include "colors.h"
#include "timing.h"

#include "Image.h"
#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;

Image filterImageGPU(Image& img, double scale)
{
  // Start timer
  TickMeter stepTime;
  TickMeter totalTime;
  stepTime.start();
  totalTime.start();

  Mat matBGR = img.img;

  // Run multiple times to get accurate timing info. First iteration
  // Is always slower than normal

  /*----- RESIZE/FILTER IMAGE -----*/

  // Resize with CPU. Faster than resizing using GPU due to memory latency
  cuda::GpuMat gpuMatBGR(matBGR);
  cuda::GpuMat gpuMatBGRSmall;

  if (scale != 1.0)
  {
    cuda::resize(gpuMatBGR, gpuMatBGRSmall, Size(), scale, scale, INTER_LINEAR);
  }
  else
  {
    gpuMatBGRSmall = gpuMatBGR;
  }

  printTime("     Resize GPU", stepTime);

  // Convert color space to HSV using GPU
  cuda::GpuMat gpuMatHSV;
  cuda::cvtColor(gpuMatBGRSmall, gpuMatHSV, CV_BGR2HSV, 0);

  // Split HSV image into 3 channels
  vector<cuda::GpuMat> gpuSplitMatHSV(3);
  cuda::split(gpuMatHSV, gpuSplitMatHSV);

  vector<Mat> splitMatHSV(3);
  gpuSplitMatHSV[0].download(splitMatHSV[0]);
  gpuSplitMatHSV[1].download(splitMatHSV[1]);
  gpuSplitMatHSV[2].download(splitMatHSV[2]);

  // GPU Split faster than CPU
  Mat matHSV(gpuMatHSV);
  printTime("     Convert & Split", stepTime);

  // Blur image (Must use CPU for a 3-channel image)
  boxFilter(matHSV, matHSV, -1, Size(7, 7));

  printTime("     Blur", stepTime);

  // Return
  Image filteredImage;
  filteredImage.img = matHSV;
  filteredImage.imgColors = img.imgColors;
  filteredImage.imgPath = img.imgPath;
  filteredImage.valid = img.valid;

  return filteredImage;
}

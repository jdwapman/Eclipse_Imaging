/*
 * gpuCustom.h
 *
 *  Created on: Jan 2, 2018
 *      Author: jwapman
 */

#ifndef SRC_GPUCUSTOM_H_
#define SRC_GPUCUSTOM_H_

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void gpuInRange(cuda::GpuMat& src, cuda::GpuMat& dest, int* low, int* high);
void gpuBoxFilter(vector<cuda::GpuMat>& gpuSplitImgHSV, cuda::GpuMat& gpuImgHSV);

#endif /* SRC_GPUCUSTOM_H_ */

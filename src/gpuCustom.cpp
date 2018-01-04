/*
 * gpuCustom.cpp
 *
 *  Created on: Jan 2, 2018
 *      Author: jwapman
 */

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//Applies in-range thresholding to an image using the GPU
void gpuInRange(cuda::GpuMat& src, cuda::GpuMat& dest, int* low, int* high)
{

	//Perform range thresholding using GPU
	cuda::GpuMat gpuSplitImg[3];
	cuda::split(src,gpuSplitImg);

	//Create matrices for upper & lower values
	cuda::GpuMat gpuTarps_Thresh_Above[3];
	cuda::GpuMat gpuTarps_Thresh_Below[3];

	//Apply lower thresholding
	cuda::threshold(gpuSplitImg[0],gpuTarps_Thresh_Above[0],low[0],255,THRESH_BINARY);
	cuda::threshold(gpuSplitImg[1],gpuTarps_Thresh_Above[1],low[1],255,THRESH_BINARY);
	cuda::threshold(gpuSplitImg[2],gpuTarps_Thresh_Above[2],low[2],255,THRESH_BINARY);

	//Apply upper bound thresholding using inverse threshold operation
	cuda::threshold(gpuSplitImg[0],gpuTarps_Thresh_Below[0],high[0],255,THRESH_BINARY_INV);
	cuda::threshold(gpuSplitImg[1],gpuTarps_Thresh_Below[1],high[1],255,THRESH_BINARY_INV);
	cuda::threshold(gpuSplitImg[2],gpuTarps_Thresh_Below[2],high[2],255,THRESH_BINARY_INV);

	//Combine channel thresholds
	cuda::bitwise_and(gpuTarps_Thresh_Above[0], gpuTarps_Thresh_Above[1], dest);
	cuda::bitwise_and(dest, gpuTarps_Thresh_Above[2], dest);
	cuda::bitwise_and(dest, gpuTarps_Thresh_Below[0], dest);
	cuda::bitwise_and(dest, gpuTarps_Thresh_Below[1], dest);
	cuda::bitwise_and(dest, gpuTarps_Thresh_Below[2], dest);

	//Use this to display thresholding
//	Mat cpuDest(dest);
//	imshow("Thresholded Image", cpuDest);
//	waitKey(0); //Wait for any key press before closing window


}

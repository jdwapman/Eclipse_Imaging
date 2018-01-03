/*
 * Tarp.h
 *
 *  Created on: Jan 2, 2018
 *      Author: jwapman
 */

#ifndef SRC_TARP_H_
#define SRC_TARP_H_

#include <vector>
#include <opencv2/opencv.hpp>
#include "gpuCustom.h"
#include <string>

using namespace std;
using namespace cv;

//Holds possible tarps of a given color
class Tarp {

	string color;

	//Stores the HSV values of the tarp
	int hsv_ideal[3];
	int hsv_low[3];
	int hsv_high[3];

	//Target identification
	double last_position = 0;
	vector<double> tarpArea;
	vector<Scalar> tarpMean;
	vector<vector<Point> > tarpContours;
	vector<bool> validContour;


public:
	Tarp(string color, int* ideal, int* low, int* high);
	virtual ~Tarp();

	//Custom functions
	void findTarpMeanSTD(Mat* splitImgHSV);
	void findTarpContours(cuda::GpuMat gpuImgHSV);
	void findTarpArea();
};


#endif /* SRC_TARP_H_ */

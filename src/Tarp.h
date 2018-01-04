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


	//Private functions
	vector< tuple<Scalar, Scalar, unsigned int> > findTarpMeans(vector<vector<Point> > tarpContours, Mat* splitImgHSV);
	vector<vector<Point> > findTarpContours(cuda::GpuMat gpuImgHSV);
	vector< tuple<double, unsigned int> > findTarpAreas(vector<vector<Point> > tarpContours);
	vector< tuple<unsigned int, unsigned int> > findTarpVertices(vector<vector<Point> > tarpContours);

	//Sorting functions
	bool compare(tuple<Scalar, Scalar, unsigned int>);

public:
	Tarp(string color, int* ideal, int* low, int* high);
	virtual ~Tarp();

	//Find most likely tarp
	vector<Point> findBestTarp(cuda::GpuMat gpuImgHSV, Mat* splitImgHSV);
};


#endif /* SRC_TARP_H_ */

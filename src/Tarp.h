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

	//Tarp values



	//Private functions
	vector< tuple<Scalar, Scalar, unsigned int> > findTarpMeans(vector<vector<Point> > tarpContours, vector<Mat> splitImgHSV, vector<bool>& tarpValid);
	vector<vector<Point> > findTarpContours(const Mat& imgHSV);
	vector< tuple<double, unsigned int> > findTarpAreas(vector<vector<Point> > tarpContours, vector<bool>& tarpValid);
	vector< tuple<unsigned int, unsigned int> > findTarpVertices(vector<vector<Point> > tarpContours, vector<bool>& tarpValid);
	vector< tuple<double, unsigned int> > findTarpHist(vector<vector<Point> > tarpContours, vector<Mat> splitImgHSV, vector<bool>& tarpValid);


public:
	Tarp(string color, int* ideal, int* low, int* high);
	virtual ~Tarp();
	double dominantColor;
	//Find most likely tarp
	//vector<Point> findBestTarp(cuda::GpuMat& gpuImgHSV, vector<Mat>& splitImgHSV);
	void findBestTarp(Mat& gpuImgHSV, vector<Mat>& splitImgHSV, vector<Point>& bestTarp);
	vector<Point> findBestTarpORB(Mat& imgHSV);
};


#endif /* SRC_TARP_H_ */

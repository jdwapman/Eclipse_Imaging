/*
 * Tarp.cpp
 *
 *  Created on: Jan 2, 2018
 *      Author: jwapman
 */

#include "Tarp.h"

#include <vector>
#include <opencv2/opencv.hpp>
#include "gpuCustom.h"
#include <string>

using namespace std;
using namespace cv;

Tarp::Tarp(string color, int* ideal, int* low, int* high)
{
	// HSV Values of tarps [H, S, V]
	// H: 0-360 degrees
	// S: 0-100%
	// V: 0-100%

	this->color = color;

}

Tarp::~Tarp()
{

}

//Identify possible tarp contours for a given HSV image
void Tarp::findTarpContours(cuda::GpuMat gpuImgHSV)
{

	//Apply thresholding.
	//This isolates the desired color and makes it easier for the following
	//edge detection algorithm to identify the tarps

	cuda::GpuMat gpuThresh;

	gpuInRange(gpuImgHSV,gpuThresh,this->hsv_low,this->hsv_high);

	Mat cpuThresh(gpuThresh); //Save to CPU

	vector<vector<Point> > contours, contours_approx;
	vector<Vec4i> hierarchy;

	findContours( cpuThresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0) );
	contours_approx.resize(contours.size());

	//Appromiximate contours. Typically 1% - 5% error is typical.
	//Additionally, this creates closed loops of all contours. (Needed to find center/average color)
	for( size_t k = 0; k < contours.size(); k++ )
	{
		approxPolyDP(Mat(contours[k]), contours_approx[k], 0.01*arcLength(contours[k],true), true);
	}

	this->tarpContours = contours_approx; //Save possible contours

	this->validContour.resize(contours_approx.size(), true); //Initialize validity vector

	return;
}

//Get average color & std deviation of each contour location
void Tarp::findTarpMeanSTD(Mat* splitImgHSV)
{
	for(unsigned int i = 0; i < this->tarpContours.size(); i++)
	    {
	    	if(this->validContour[i])
	    	{
				Mat mask(splitImgHSV[0].rows,splitImgHSV[0].cols,CV_8UC1, Scalar(0)); //Initialize

				drawContours(mask, this->tarpContours, i, Scalar(255), -1, 8); //Draw filled in mask
				this->tarpMean[i] = mean(splitImgHSV[0],mask); //Gets average value of the points inside the mask
	    	}
	    }

	return;
}

//Find the area of each tarp contour
void Tarp::findTarpArea()
{

	this->tarpArea.resize(this->tarpContours.size(), 0);

	for(unsigned int i = 0; i < tarpArea.size(); i++)
	    {
	    	if(this->validContour[i]){
	    		this->tarpArea[i] = contourArea(this->tarpContours[i]);
	    	}
	    }

	return;
}

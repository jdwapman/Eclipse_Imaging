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
	// H: 0-360 degrees -> 0-180 degrees
	// S: 0-100% -> 0-255
	// V: 0-100% -> 0-255

	this->color = color;

	this->hsv_ideal[0] = (int)(low[0] / 2.0);
	this->hsv_ideal[1] = (int)((low[1] / 100.0) * 255.0);
	this->hsv_ideal[2] = (int)((low[2] / 100.0) * 255.0);

	this->hsv_low[0] = (int)(low[0] / 2.0);
	this->hsv_low[1] = (int)((low[1] / 100.0) * 255.0);
	this->hsv_low[2] = (int)((low[2] / 100.0) * 255.0);

	this->hsv_high[0] = (int)(high[0] / 2.0);
	this->hsv_high[1] = (int)((high[1] / 100.0) * 255.0);
	this->hsv_high[2] = (int)((high[2] / 100.0) * 255.0);
}

Tarp::~Tarp()
{

}

//Identify possible tarp contours for a given HSV image
vector<vector<Point> > Tarp::findTarpContours(cuda::GpuMat gpuImgHSV)
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

	return contours_approx; //Save possible contours

}

//Get average color of each contour location
vector<Scalar> Tarp::findTarpMeans(vector<vector<Point> > tarpContours, Mat* splitImgHSV)
{
	vector<Scalar> tarpMeans(tarpContours.size(),0);

	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{

		Mat mask(splitImgHSV[0].rows,splitImgHSV[0].cols,CV_8UC1, Scalar(0)); //Initialize

		drawContours(mask, tarpContours, i, Scalar(255), -1, 8); //Draw filled in mask
		tarpMeans[i] = mean(splitImgHSV[0],mask); //Gets average value of the points inside the mask

	}

	return tarpMeans;
}

//Get std deviation of each contour location
vector<Scalar> Tarp::findTarpSTDevs(vector<vector<Point> > tarpContours, Mat* splitImgHSV)
{
	vector<Scalar> tarpSTDevs(tarpContours.size(),0);

	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{

		Mat mask(splitImgHSV[0].rows,splitImgHSV[0].cols,CV_8UC1, Scalar(0)); //Initialize

		drawContours(mask, tarpContours, i, Scalar(255), -1, 8); //Draw filled in mask
		tarpSTDevs[i] = mean(splitImgHSV[0],mask); //Gets STD of the points inside the mask

	}

	return tarpSTDevs;
}

//Find the area of each tarp contour
vector<double> Tarp::findTarpAreas(vector<vector<Point> > tarpContours)
{
	vector<double> tarpAreas(tarpContours.size(),0);

	for(unsigned int i = 0; i < tarpAreas.size(); i++)
	{
		tarpAreas[i] = contourArea(tarpContours[i]);
	}

	return tarpAreas;
}

vector<unsigned int> Tarp::findTarpVertices(vector<vector<Point> > tarpContours)
{

	vector<unsigned int> tarpVertices(tarpContours.size(), 0);

	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{
		tarpVertices[i] = tarpContours[i].size();
	}

	return tarpVertices;
}

vector<Point> Tarp::findBestTarp(cuda::GpuMat gpuImgHSV, Mat* splitImgHSV)
{

	//Get tarp contours
	vector<vector<Point> > tarpContours = findTarpContours(gpuImgHSV);

	//Get number of tarp vertices
	vector<unsigned int> tarpVertices = findTarpVertices(tarpContours);

	//Get tarp areas
	vector<double> tarpAreas = findTarpAreas(tarpContours);

	//Get tarp means
	vector<Scalar> tarpMeans = findTarpMeans(tarpContours, splitImgHSV);

	//Get tarp standard deviations
	vector<Scalar> tarpSTDevs = findTarpSTDevs(tarpContours, splitImgHSV);

	//Store whether a tarp is valid
	vector<bool> tarpValid(tarpContours.size(), true);

	// If > 10 vertices, not valid
//	for(unsigned int i = 0; i < tarpContours.size(); i++)
//	{
//		if(tarpVertices[i] > 10)
//		{
//			tarpValid[i] = false;
//		}
//	}

	//Sort by distance from

	vector<Point> bestTarp(0);

	return bestTarp;
}


/*
 * Tarp.cpp
 *
 *  Created on: Jan 2, 2018
 *      Author: jwapman
 */

#include "Tarp.h"
#include "tarpSort.h"

#include <vector>
#include <opencv2/opencv.hpp>
#include "gpuCustom.h"
#include <string>
#include <tuple>
#include <algorithm>
#include <iostream>

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
vector< tuple<Scalar, Scalar, unsigned int> > Tarp::findTarpMeans(vector<vector<Point> > tarpContours, Mat* splitImgHSV)
{
	vector< tuple<Scalar, Scalar, unsigned int> > tarpMeans(tarpContours.size());

	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{

		Mat mask(splitImgHSV[0].rows,splitImgHSV[0].cols,CV_8UC1, Scalar(0)); //Initialize

		drawContours(mask, tarpContours, i, Scalar(255), -1, 8); //Draw filled in mask

		Scalar mean;
		Scalar stddev;
		meanStdDev(splitImgHSV[0], mean, stddev, mask);

		tarpMeans[i] = make_tuple(mean[0], stddev[0], i); //Gets average value of the points inside the mask

	}

	return tarpMeans;
}

//Find the area of each tarp contour. Return <area, index>
vector< tuple<double, unsigned int> > Tarp::findTarpAreas(vector<vector<Point> > tarpContours)
{
	vector< tuple<double, unsigned int> > tarpAreas(tarpContours.size());

	for(unsigned int i = 0; i < tarpAreas.size(); i++)
	{
		tarpAreas[i] = make_tuple(contourArea(tarpContours[i]), i);
	}

	return tarpAreas;
}

vector< tuple<unsigned int, unsigned int> > Tarp::findTarpVertices(vector<vector<Point> > tarpContours)
{

	vector< tuple<unsigned int, unsigned int> > tarpVertices(tarpContours.size());

	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{
		tarpVertices[i] = make_tuple(tarpContours[i].size(), i);
	}

	return tarpVertices;
}


vector<Point> Tarp::findBestTarp(cuda::GpuMat gpuImgHSV, Mat* splitImgHSV)
{

	//Get tarp contours
	vector<vector<Point> > tarpContours = findTarpContours(gpuImgHSV);

	//Store whether a tarp is valid
	vector<bool> tarpValid(tarpContours.size(), true);

	//Get number of tarp vertices
	vector< tuple<unsigned int, unsigned int> > tarpVertices = findTarpVertices(tarpContours);

	//Get tarp areas
	vector< tuple<double, unsigned int> > tarpAreas = findTarpAreas(tarpContours);

	//Get tarp means & stddevs //TODO: Change from mean to delta from ideal
	vector< tuple<Scalar, Scalar, unsigned int> > tarpMeanSTDs = findTarpMeans(tarpContours, splitImgHSV);

	/*----- Sort &  make decision -----*/

	vector<Point> bestTarp(0); //Default empty contour

	sort(tarpAreas.begin(), tarpAreas.end(), sortAreas);
	sort(tarpVertices.begin(), tarpVertices.end(), sortVertices);

	//Base Case 0
	if (tarpContours.size() == 0)
		return bestTarp; //Return empty vector of points


	//Reject tarps with > 10 vertices
//	for(unsigned int i = 0; i < tarpVertices.size(); i++)
//		if(get<0>(tarpVertices[i]) > 10)
//			tarpValid[ get<1>(tarpVertices[i]) ] = false;

	//Reject tarps of 0 area
	for(unsigned int i = 0; i < tarpAreas.size(); i++)
		if(get<0>(tarpAreas[i]) == 0)
			tarpValid[ get<1>(tarpAreas[i]) ] = false;


//	for(unsigned int i = 0; i < tarpAreas.size(); i++)
//	{
//		cout << get<0>(tarpAreas[i]) << "," << get<1>(tarpAreas[i]) << endl;
//		cout << tarpValid[i] << endl;
//	}


	//Find largest valid area. tarpAreas sorted largest to smallest, with indexes
	//Corresponding to tarpValid, tarpContours
	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{
		if(tarpValid[ get<1>(tarpAreas[i]) ] == true)
		{
			bestTarp = tarpContours[ get<1>(tarpAreas[i]) ];
			break;
		}
	}

	//draw contours
//	Mat drawmat = Mat::zeros(splitImgHSV[0].rows,splitImgHSV[0].cols, CV_8UC1);

	//Draw contours on image.
//		for(unsigned int i = 0; i< tarpContours.size(); i++ )
//		{
//			Scalar color = Scalar(255,255,255);
//			if(tarpContours[i].size() > 0){
//				drawContours( drawmat, tarpContours, i, color, 1, 8);
//			}
//			else
//			{
//				cout << "No valid tarp" << endl;
//			}
//		}

		//imshow("Final Image", drawmat);
		//waitKey(0); //Wait for any key press before closing window

	//TODO: Scale bestTarp to fit large output image (if desired)

	return bestTarp;
}


/*
 * Tarp.cpp
 *
 *  Created on: Jan 2, 2018
 *      Author: jwapman
 */

#include "Tarp.h"
#include "tarpSort.h"
#include "timing.h"
#include "colors.h"
#include <unistd.h>
#include <vector>
#include <opencv2/opencv.hpp>
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

	dominantColor = 0.0;

}

Tarp::~Tarp()
{

}

//Identify possible tarp contours for a given HSV image
vector<vector<Point> > Tarp::findTarpContours(const Mat& imgHSV)
{

	//Apply thresholding.
	//This isolates the desired color and makes it easier for the following
	//edge detection algorithm to identify the tarps

	Mat cpuThresh;


	Scalar low = {(double)this->hsv_low[0], (double)this->hsv_low[1], (double)this->hsv_low[2]};
	Scalar high = {(double)this->hsv_high[0], (double)this->hsv_high[1], (double)this->hsv_high[2]};
	TickMeter tm;
	tm.start();
	inRange(imgHSV, low, high, cpuThresh);
	tm.stop();
	//cout << "CPU Thresh" << ": "  << tm.getTimeMilli() << " ms" << endl;


//	imshow("Thresh Image", cpuThresh);
//	waitKey(0); //Wait for any key press before closing window

	//TODO: try replacing with cpu inrange

	//Mat cpuThresh(gpuThresh); //Save to CPU

	vector<vector<Point> > contours, contours_approx;
	vector<Vec4i> hierarchy;

	findContours( cpuThresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0) );
	contours_approx.resize(contours.size());

	//Appromiximate contours. Typically 1% - 5% error is typical.
	//Additionally, this creates closed loops of all contours. (Needed to find center/average color)
	for( size_t k = 0; k < contours.size(); k++ )
	{
		approxPolyDP(Mat(contours[k]), contours_approx[k], 0.05*arcLength(contours[k],true), true);
	}

	return contours_approx; //Save possible contours

}

//Get average color of each contour location
vector< tuple<Scalar, Scalar, unsigned int> > Tarp::findTarpMeans(vector<vector<Point> > tarpContours, vector<Mat> splitImgHSV, vector<bool>& tarpValid)
{
	vector< tuple<Scalar, Scalar, unsigned int> > tarpMeans(tarpContours.size());

	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{
		if(tarpValid[i] == true)
		{
			Mat mask(splitImgHSV[0].rows,splitImgHSV[0].cols,CV_8UC1, Scalar(0)); //Initialize

			drawContours(mask, tarpContours, i, Scalar(255), -1, 8); //Draw filled in mask

			Scalar mean;
			Scalar stddev;
			meanStdDev(splitImgHSV[0], mean, stddev, mask);

			tarpMeans[i] = make_tuple(mean[0], stddev[0], i); //Gets average value of the points inside the mask
		}
	}

	return tarpMeans;
}

//Get histogram of each contour location to find dominant color
vector< tuple<double, unsigned int> > Tarp::findTarpHist(vector<vector<Point> > tarpContours, vector<Mat> splitImgHSV, vector<bool>& tarpValid)
{
	vector< tuple<double, unsigned int> > tarpDominantColor(tarpContours.size());

	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{
		if(tarpValid[i] == true)
		{
			//Get mask
			Mat mask(splitImgHSV[0].rows,splitImgHSV[0].cols,CV_8UC1, Scalar(0)); //Initialize

			drawContours(mask, tarpContours, i, Scalar(255), -1, 8); //Draw filled in mask

			//Histogram variables
			int histSize = 90;
			float range[] = {0, 180};
			const float* histRange = { range };
			bool uniform = true;
			bool accumulate = false;
			Mat hist;

			calcHist(&splitImgHSV[0], 1, 0, mask, hist, 1, &histSize, &histRange, uniform, accumulate);



			// Draw the histograms for B, G and R
			int hist_w = 800; int hist_h = 800;
			int bin_w = cvRound( (double) hist_w/histSize );

			Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

			/// Normalize the result to [ 0, histImage.rows ]
			normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

			int numPeaks = 0;

			/// Draw for each channel
			for( int i = 1; i < histSize; i++ )
			{
			  line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
							   Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
							   Scalar( 180, 0, 0), 1, 8, 0  );
			}

			//Get max location
			double min, max;
			Point minLoc, maxLoc;
			minMaxLoc(hist, &min, &max, &minLoc, &maxLoc);

			//cout << "maxLocY: " << maxLoc.y  << endl; //Dominant frequency of the contour

			for(int i = 0; i < histSize; i++)
			{
			  //Find peaks
			  //cout << hist.at<float>(-1) << endl;
			  if( (hist.at<float>(i-1) < hist.at<float>(i))
					  && ((hist.at<float>(i) > hist.at<float>(i+1)))
					  && (hist.at<float>(i) > 0.025*max))
			  {
				  numPeaks++;
			  }
			}

			if(numPeaks > 1)
			{
				tarpValid[i] = false;
			}

			//cout << "Peaks: " << i << ": " << numPeaks << endl;

			/// Display
//			namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE );
//
//			imshow("calcHist Demo", histImage );
//
//			waitKey(0);



			tarpDominantColor[i] = make_tuple(maxLoc.y, i); //Gets average value of the points inside the mask
		}
	}

	return tarpDominantColor;
}

//Find the area of each tarp contour. Return <area, index>
vector< tuple<double, unsigned int> > Tarp::findTarpAreas(vector<vector<Point> > tarpContours, vector<bool>& tarpValid)
{
	vector< tuple<double, unsigned int> > tarpAreas(tarpContours.size());

	double area = 0;;

	for(unsigned int i = 0; i < tarpAreas.size(); i++)
	{

		area = contourArea(tarpContours[i]);

		tarpAreas[i] = make_tuple(area, i);

		if(area < 200) //Modify based on height
		{
			//tarpValid[i] = false;
		}

	}

	return tarpAreas;
}

vector< tuple<unsigned int, unsigned int> > Tarp::findTarpVertices(vector<vector<Point> > tarpContours, vector<bool>& tarpValid)
{

	vector< tuple<unsigned int, unsigned int> > tarpVertices(tarpContours.size());

	double size = 0;

	for(unsigned int i = 0; i < tarpContours.size(); i++)
	{

		size = tarpContours[i].size();

		tarpVertices[i] = make_tuple(size, i);

		if((size > 6) || (size <= 3))
		{
			tarpValid[i] = false;
		}

	}

	return tarpVertices;
}

vector<Point> Tarp::findBestTarpORB(Mat& imgHSV)
{
	vector<Point> o;

	//Create ORB detector

	TickMeter tm;
	Mat img = imread("/home/jwapman/Eclipse_Workspace/Target_Detection/src/img.jpg");
	Mat pinkTemp = imread("/home/jwapman/Eclipse_Workspace/Target_Detection/src/Pink.jpg");

	Ptr<ORB> orbPtr = ORB::create();

	vector<KeyPoint> keypointsTemp, keypointsImg;
	Mat descriptorsTemp, descriptorsImg;

	//Threshold template
	Mat templateThresh;
	Mat pinkHSV;
	cvtColor(pinkTemp, pinkHSV, CV_BGR2HSV,0);
	Scalar low = {(double)this->hsv_low[0], (double)this->hsv_low[1], (double)this->hsv_low[2]};
	Scalar high = {(double)this->hsv_high[0], (double)this->hsv_high[1], (double)this->hsv_high[2]};
	inRange(pinkHSV, low, high, templateThresh);
 	orbPtr->detectAndCompute(templateThresh, Mat(), keypointsTemp, descriptorsTemp);




 	//Threshold input images
 	Mat cpuThresh;
 	Mat i_HSV;
 	cvtColor(img, i_HSV, CV_BGR2HSV,0);

	inRange(i_HSV, low, high, cpuThresh);



 	orbPtr->detectAndCompute(cpuThresh, Mat(), keypointsImg, descriptorsImg);


 	Ptr<DescriptorMatcher> d = DescriptorMatcher::create(4);
 	vector<DMatch> matches;
 	d->match(descriptorsTemp, descriptorsImg, matches);

 	//Draw
 	Mat imgMatches;
 	drawMatches(pinkTemp, keypointsTemp, img, keypointsImg, matches, imgMatches);


 	namedWindow("Matches",WINDOW_NORMAL);
 			resizeWindow("Matches",600,600);
 	imshow("Matches", imgMatches);
 	waitKey(0);

	return o;

}

void Tarp::findBestTarp(Mat& imgHSV, vector<Mat>& splitImgHSV, vector<Point>& bestTarp)
{


	//Get tarp contours
	vector<vector<Point> > tarpContours = findTarpContours(imgHSV);

	unsigned int numContours = tarpContours.size();
	/*----- Get data -----*/

	//Store whether a tarp is valid
	vector<bool> tarpValid(numContours, true);

	//Get number of tarp vertices
	vector< tuple<unsigned int, unsigned int> > tarpVertices = findTarpVertices(tarpContours, tarpValid);

	//Get tarp areas
	vector< tuple<double, unsigned int> > tarpAreas = findTarpAreas(tarpContours, tarpValid);

	//Get tarp histogram
	vector< tuple<double, unsigned int> > tarpDominantColor = findTarpHist(tarpContours, splitImgHSV, tarpValid);

	//Get tarp mean, stddev
	//vector< tuple<Scalar, Scalar, unsigned int> > tarpMeanStddev = findTarpMeans(tarpContours, splitImgHSV, tarpValid);

	//Get tarp convexity
	vector<bool> tarpConvexity(numContours, false);
	for (unsigned int i = 0; i < numContours; i++)
	{
		if(tarpValid[i])
			tarpValid[i] = isContourConvex(tarpContours[i]);


		tarpConvexity[i] = isContourConvex(tarpContours[i]);
		//cout << isContourConvex(tarpContours[i]) << endl;
	}


	/*----- Sort &  make decision -----*/

	vector<Point> tarp(0); //Default empty contour

	sort(tarpAreas.begin(), tarpAreas.end(), sortAreas);
//	sort(tarpVertices.begin(), tarpVertices.end(), sortVertices);
//	sort(ratios.begin(), ratios.end(), sortAreas); //TODO: change function call

	//Base Case 0
	if (numContours == 0)
		bestTarp = tarp; //Return empty vector of points

	//Default
	bestTarp = tarp;

	//Find largest valid area. tarpAreas sorted largest to smallest, with indexes
	//Corresponding to tarpValid, tarpContours
	for(unsigned int i = 0; i < numContours; i++)
	{
		if(tarpValid[ get<1>(tarpAreas[i]) ] == true)
		{
			bestTarp = tarpContours[ get<1>(tarpAreas[i]) ];

			this->dominantColor = get<0>(tarpDominantColor[get<1>(tarpAreas[i])]);
//
//			//Write information
//			cout << "Area: " << get<0>(tarpAreas[get<1>(tarpAreas[i])]) << endl;
//			cout << "Vertices: " << get<0>(tarpVertices[get<1>(tarpAreas[i])]) << endl;
//			cout << "Convex: " << tarpConvexity[get<1>(tarpAreas[i])] << endl;
//			cout << "Dominant Color: " << (get<0>(tarpDominantColor[get<1>(tarpAreas[i])])) << endl;
//			cout << "Index: " << get<1>(tarpAreas[i]);
//			cout << endl;


			break;
		}
	}


	//draw contours
//	Mat drawmat = Mat::zeros(splitImgHSV[0].rows,splitImgHSV[0].cols, CV_8UC1);
//
//	//Draw contours on image.
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
//
//		imshow("Final Image", drawmat);
//		resizeWindow("Final Image", 800, 800);
//		waitKey(0); //Wait for any key press before closing window

	//: Scale bestTarp to fit large output image (if desired)

	return;
}

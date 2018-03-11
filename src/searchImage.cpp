/*
 * searchImage.cpp
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#include <iostream> //Input/output library
#include <vector>
#include <string>
#include <algorithm> //For sorting
#include <opencv2/opencv.hpp> //OpenCV library
#include "Tarp.h"
#include "colors.h"
#include "timing.h"
#include <future>
#include <chrono>

#include "boost/filesystem.hpp"
#include "Image.h"

using namespace std;
using namespace cv;


vector<vector<Point> > searchImage(Image imgHSV)
{
	//Split Mat
	Mat matHSV = imgHSV.img;
	vector<Mat> splitMatHSV(3);

	split(imgHSV.img, splitMatHSV);

	//Find contours

	color_data colors = imgHSV.imgColors;
	Tarp blue("Blue", colors.blue_ideal, colors.blue_low, colors.blue_high);
	Tarp pink("Pink", colors.pink_ideal, colors.pink_low, colors.pink_high);
	Tarp yellow("Yellow", colors.yellow_ideal, colors.yellow_low, colors.yellow_high);


	/*----- PER-TARP OPERATIONS -----*/

	vector<vector<Point> > bestContours(3);

	//Threading option
//	thread findBlue(&Tarp::findBestTarp,&blue, ref(matHSV), ref(splitMatHSV),ref(bestContours[0]));
//	thread findPink(&Tarp::findBestTarp,&pink, ref(matHSV), ref(splitMatHSV),ref(bestContours[1]));
//	thread findYellow(&Tarp::findBestTarp,&yellow, ref(matHSV), ref(splitMatHSV),ref(bestContours[2]));
//	findBlue.join();
//	findPink.join();
//	findYellow.join();

	//Sequential option

	blue.findBestTarp(matHSV, splitMatHSV, bestContours[0]);
	pink.findBestTarp(matHSV, splitMatHSV, bestContours[1]);
	yellow.findBestTarp(matHSV, splitMatHSV, bestContours[2]);

	return bestContours;
}

/*
 * tarpFind.cpp
 *
 *  Created on: Feb 1, 2018
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
#include "Image.h"
#include <future>
#include <chrono>

#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;

void Image::processImage()
{

	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();

	color_data colors = this->colors.front(); //Get
	this->colors.pop();	//Remove

	//Create Tarp Objects
	Tarp blue("Blue", colors.blue_ideal, colors.blue_low, colors.blue_high);
	Tarp pink("Pink", colors.pink_ideal, colors.pink_low, colors.pink_high);
	Tarp yellow("Yellow", colors.yellow_ideal, colors.yellow_low, colors.yellow_high);

	//Run multiple times to get accurate timing info. First iteration
	//Is always slower than normal


	/*----- RESIZE/FILTER IMAGE -----*/

	//Resize with CPU. Faster than resizing using GPU due to memory latency
	cuda::GpuMat gpuCameraImgBGR(this->cameraImgBGR);
	cuda::GpuMat gpuCameraImgBGRSmall;

	if(this->scale != 1)
	{
		cuda::resize(gpuCameraImgBGR,gpuCameraImgBGRSmall,Size(),scale,scale,INTER_LINEAR);
	}
	else
	{
		gpuCameraImgBGRSmall = gpuCameraImgBGR;
	}

	printTime("     Resize GPU", stepTime);

	//Convert color space to HSV using GPU
	cuda::GpuMat gpuImgHSV;
	cuda::cvtColor(gpuCameraImgBGRSmall, gpuImgHSV, CV_BGR2HSV,0);

	//Split HSV image into 3 channels
	vector<cuda::GpuMat> gpuSplitImgHSV(3);
	cuda::split(gpuImgHSV,gpuSplitImgHSV);

	vector<Mat> splitImgHSV(3);
	gpuSplitImgHSV[0].download(splitImgHSV[0]);
	gpuSplitImgHSV[1].download(splitImgHSV[1]);
	gpuSplitImgHSV[2].download(splitImgHSV[2]);


	//GPU Split faster than CPU
	Mat imgHSV(gpuImgHSV);
	printTime("     Convert & Split", stepTime);

	//Blur image (Must use CPU for a 3-channel image)
	boxFilter(imgHSV,imgHSV,-1,Size(12,12));

	printTime("     Blur", stepTime);


	/*----- PER-TARP OPERATIONS -----*/

	vector<vector<Point> > bestContours(3);

	//Threading option
	thread findBlue(&Tarp::findBestTarp,&blue, ref(imgHSV), ref(splitImgHSV),ref(bestContours[0]));
	thread findPink(&Tarp::findBestTarp,&pink, ref(imgHSV), ref(splitImgHSV),ref(bestContours[1]));
	thread findYellow(&Tarp::findBestTarp,&yellow, ref(imgHSV), ref(splitImgHSV),ref(bestContours[2]));
	findBlue.join();
	findPink.join();
	findYellow.join();

//	//Save data to filesystem
//	ofstream blueFile, pinkFile, yellowFile;
//	blueFile.open("/home/jwapman/Eclipse_Workspace/Target_Detection/blue_color_vals.txt", fstream::out);
//	pinkFile.open("/home/jwapman/Eclipse_Workspace/Target_Detection/pink_color_vals.txt", fstream::out);
//	yellowFile.open("/home/jwapman/Eclipse_Workspace/Target_Detection/yellow_color_vals.txt", fstream::out);



//	blueFile.close();
//	pinkFile.close();
//	yellowFile.close();

	//blueFile << blue.dominantColor * 2 << endl;
	//pinkFile << pink.dominantColor * 2 << endl;
	//yellowFile << yellow.dominantColor * 2 << endl;

	//Sequential option

//	blue.findBestTarp(imgHSV, splitImgHSV, bestContours[0]);
//	pink.findBestTarp(imgHSV, splitImgHSV, bestContours[1]);
//	yellow.findBestTarp(imgHSV, splitImgHSV, bestContours[2]);




	this->finalContours = bestContours;

	printTime("     Decision", stepTime);


	return;
}


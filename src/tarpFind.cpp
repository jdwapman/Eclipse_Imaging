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
#include <future>
#include <chrono>

#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;



vector<vector<Point> > processImage(Mat cameraImgBGR, color_data colors)
{

	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();

	//Create Tarp Objects
	Tarp blue("Blue", colors.blue_ideal, colors.blue_low, colors.blue_high);
	Tarp pink("Pink", colors.pink_ideal, colors.pink_low, colors.pink_high);
	Tarp yellow("Yellow", colors.yellow_ideal, colors.yellow_low, colors.yellow_high);




	//Get image dimensions for preallocation. Can eventually replace with constants
	int rows = cameraImgBGR.rows;
	int cols = cameraImgBGR.cols;
	int imgType = cameraImgBGR.type();

	//Reduced image dimensions
	double scale = (1.0/4.0);
	int rrows = rows * scale;
	int rcols = cols * scale;

	//Check image exists
	if(cameraImgBGR.empty() == true)
	{
		cout << "No image detected" << endl;
		Mat empty;
		return empty;
		//continue; //Error code that no data was gathered
	}

	Mat cameraImgBGRSmall(rrows,rcols,imgType);

	printTime("Check Image", stepTime);

	//Run multiple times to get accurate timing info. First iteration
	//Is always slower than normal


	/*----- RESIZE/FILTER IMAGE -----*/

	//Resize with CPU. Faster than resizing using GPU due to memory latency
//	cameraImgBGRSmall = cameraImgBGR;
	resize(cameraImgBGR,cameraImgBGRSmall,Size(),scale,scale,INTER_LINEAR);

	printTime("Resize CPU", stepTime);


	cuda::GpuMat gpuCameraImgBGRSmall(cameraImgBGRSmall);
//		//Declare GPU matrices to hold converted color space
//		cuda::GpuMat gpuImgHSV(rrows,rcols,imgType);
//
//		//Convert color space to HSV using GPU
	cuda::GpuMat gpuImgHSV;
	cuda::cvtColor(gpuCameraImgBGRSmall, gpuImgHSV, CV_BGR2HSV,0);




	//Mat imgHSV(gpuImgHSV);

	printTime("Convert Color 0", stepTime);

//
//
//		//Split HSV image into 3 channels
	vector<cuda::GpuMat> gpuSplitImgHSV(3);
	cuda::split(gpuImgHSV,gpuSplitImgHSV);

	vector<Mat> splitImgHSV(3);
	gpuSplitImgHSV[0].download(splitImgHSV[0]);
	gpuSplitImgHSV[1].download(splitImgHSV[1]);
	gpuSplitImgHSV[2].download(splitImgHSV[2]);

//		Mat imgHSV;
//		cvtColor(cameraImgBGRSmall, imgHSV, CV_BGR2HSV,0);
//		vector<Mat> splitImgHSV(3);
//		split(imgHSV, splitImgHSV);

	//GPU Split faster than CPU
	Mat imgHSV(gpuImgHSV);
	printTime("Convert Color", stepTime);

	//Blur image (Must use CPU for a 3-channel image)

	printTime("Start Blur", stepTime);

	boxFilter(imgHSV,imgHSV,-1,Size(12,12));




//	namedWindow("Final Image",WINDOW_NORMAL);
//	resizeWindow("Final Image",800,800);
//	imshow("Final Image", imgHSV);
//	waitKey(0); //Wait for any key press before closing window

	printTime("Blur", stepTime);


	/*----- PER-TARP OPERATIONS -----*/

	vector<vector<Point> > finalContours(3);

	//Threading option
//	thread findBlue(&Tarp::findBestTarp,&blue, ref(imgHSV), ref(splitImgHSV),ref(finalContours[0]));
//	thread findPink(&Tarp::findBestTarp,&pink, ref(imgHSV), ref(splitImgHSV),ref(finalContours[1]));
//	thread findYellow(&Tarp::findBestTarp,&yellow, ref(imgHSV), ref(splitImgHSV),ref(finalContours[2]));
//	findBlue.join();
//	findPink.join();
//	findYellow.join();

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
	blue.findBestTarp(imgHSV, splitImgHSV, finalContours[0]);
	pink.findBestTarp(imgHSV, splitImgHSV, finalContours[1]);
	yellow.findBestTarp(imgHSV, splitImgHSV, finalContours[2]);

	printTime("Decision", stepTime);


	return finalContours;
}


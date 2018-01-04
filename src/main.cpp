/*
 * Jonathan Wapman
 * Created 12/23/2017
 * UC Davis Eclipse Rocketry Senior Design Project
 * Target Detection
 */

#include <iostream> //Input/output library
#include <vector>
#include <string>
#include <algorithm>
#include <opencv2/opencv.hpp> //OpenCV library
#include "Tarp.h"

using namespace std;
using namespace cv;

//Forware-declare functions
Mat getImage();
void printTime(String operation, TickMeter& tm);

int main(int argc, char** argv )
{

	// Ranges of colors to look for in HSV color space
	int blue_ideal[3] = {0,0,0};
	int blue_low[3] = {100,150,0};
	int blue_high[3] = {130,255,255};

	int pink_ideal[3] = {0,0,0};
	int pink_low[3] = {300,30,70};
	int pink_high[3] = {340,60,100};

	int yellow_ideal[3] = {0,0,0};
	int yellow_low[3] = {150,0,0};
	int yellow_high[3] = {180,255,255};

	//Create Tarp Objects
	Tarp blue("Blue", blue_ideal, blue_low, blue_high);
	Tarp pink("Pink", pink_ideal, pink_low, pink_high);
	Tarp yellow("Yellow", yellow_ideal, yellow_low, yellow_high);

	/*-----Initial setup and image capture-----*/

	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();

	//Import images. imread imports in BGR format.


	Mat cameraImgBGR = imread("/home/jwapman/Eclipse_Workspace/Target_Detection/Images/chaos.jpg", CV_LOAD_IMAGE_COLOR);

	//Get image dimensions for preallocation. Can eventually replace with constants
	int rows = cameraImgBGR.rows;
	int cols = cameraImgBGR.cols;
	int imgType = cameraImgBGR.type();

	//Reduced image dimensions
	double scale = (1.0/8.0);
	int rrows = rows * scale;
	int rcols = cols * scale;

	//Check image exists
	if(cameraImgBGR.empty() == true)
	{
		cerr << "No image detected" << endl;
		return 1; //Error code that no data was gathered
	}

	//Check if CUDA-enabled GPU can be accessed
	if(cuda::getCudaEnabledDeviceCount() < 1)
	{
		cerr << "No CUDA-enabled GPU detected" << endl;
		return 2;
	}

	//Initialize GPU
	cuda::setDevice(0);
	cuda::resetDevice();

	Mat cameraImgBGRSmall(rrows,rcols,imgType);

	resize(cameraImgBGR,cameraImgBGRSmall,Size(),0.125,0.125,INTER_LINEAR);

	cuda::GpuMat gpuCameraImgBGRSmall(rrows,rcols,imgType);

	gpuCameraImgBGRSmall.upload(cameraImgBGRSmall);

	//Declare GPU matrices to hold converted color space
	cuda::GpuMat gpuImgHSV(rrows,rcols,imgType);

	//Convert color space to HSV using GPU
	cuda::cvtColor(gpuCameraImgBGRSmall, gpuImgHSV, CV_BGR2HSV,0);
	Mat imgHSV(gpuImgHSV);

	printTime("Start", stepTime);
	//Split HSV image into 3 channels
	cuda::GpuMat gpuSplitImgHSV[3];
	cuda::split(gpuImgHSV,gpuSplitImgHSV);

	Mat splitImgHSV[3];
	gpuSplitImgHSV[0].download(splitImgHSV[0]);
	gpuSplitImgHSV[1].download(splitImgHSV[1]);
	gpuSplitImgHSV[2].download(splitImgHSV[2]);

	//Blur image (Must use CPU for a 3-channel image)
	boxFilter(imgHSV,imgHSV,-1,Size(5,5));
	gpuImgHSV.upload(imgHSV);
	printTime("Filter", stepTime);

	/*----- PER-TARP OPERATIONS -----*/
	vector<vector<Point> > finalContours(3);
	finalContours[0] = pink.findBestTarp(gpuImgHSV, splitImgHSV);
	printTime("Decision", stepTime);


	//Draw contours on image.
	for(unsigned int i = 0; i< finalContours.size(); i++ )
	{
		Scalar color = Scalar(255,255,255);
		if(finalContours[i].size() > 0){
			drawContours( cameraImgBGRSmall, finalContours, i, color, -1, 8);
		}
		else
		{
			cout << "No valid tarp" << endl;
		}
	}
	printTime("Draw Contour", stepTime);

	//Display window containing thresholded tarp
    imshow("Final Image", cameraImgBGRSmall);
    waitKey(0); //Wait for any key press before closing window

    //NOTE: Failing to close the display window before running a new iteration of the code
    //can result in GPU memory errors

    //Save image

    imwrite("/home/jwapman/Eclipse_Workspace/Target_Detection/Images/Output_Image.jpg",cameraImgBGRSmall);
    printTime("Stop Save", stepTime);

    //Free GPU Resources
	cuda::resetDevice();

    return 0;

}

/*---------- CUSTOM FUNCTIONS ----------*/

//Function to import an image. Currently only reads files from filesystem.
//In the future, expand to include code for accessing the camera
Mat getImage()
{
	return imread("/home/jwapman/Eclipse_Workspace/Target_Detection/Images/tarps.jpg", CV_LOAD_IMAGE_COLOR);
}


//Output elapsed time since last printTime() operation. Useful for determining runtime of given step.
void printTime(String operation, TickMeter& tm)
{
	tm.stop();
	cout << operation << ": "  << tm.getTimeMilli() << " ms" << endl;
	tm.reset();
	tm.start();
	return;
}

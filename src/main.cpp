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
#include <thread>

using namespace std;
using namespace cv;

//Forware-declare functions
Mat getImage();
void gpuInRange(cuda::GpuMat& src, cuda::GpuMat& dest, int* low, int* high);
void printTime(String operation, TickMeter& tm);
vector<vector<Point> > findContours(cuda::GpuMat gpuImgHSV, int* low_thresh, int* high_thresh);

int main(int argc, char** argv )
{

	// Ranges of colors to look for in HSV color space
	int blue_low[3] = {100,150,0};
	int blue_high[3] = {130,255,255};

	int pink_low[3] = {150,70,178};
	int pink_high[3] = {180,155,255};

	int yellow_low[3] = {150,0,0};
	int yellow_high[3] = {180,255,255};

	//Create Tarp Objects
	Tarp blue("Blue", blue_low, blue_high, blue_high);
	Tarp pink("Pink", pink_low, pink_high, pink_high);
	Tarp yellow("Yellow", yellow_low, yellow_high, yellow_high);

	/*-----Initial setup and image capture-----*/

	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();

	//Import images. imread imports in BGR format.

	Mat cameraImgBGR = imread("/home/jwapman/Eclipse_Workspace/Target_Detection/Images/tarps.jpg", CV_LOAD_IMAGE_COLOR);

	//printTime("Read Image",stepTime);


	//Get image dimensions
	int rows = cameraImgBGR.rows;
	int cols = cameraImgBGR.cols;
	int imgType = cameraImgBGR.type();

	//Reduced image dimensions
	double scale = (1.0/8.0);
	int rrows = rows * scale;
	int rcols = cols * scale;

	//printTime("Get Dimensions",stepTime);

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

	/*----- PER-TARP OPERATIONS -----*/
	
	/*----- Contour detection -----*/

	printTime("Start", stepTime);
	blue.findTarpContours(gpuImgHSV);
	pink.findTarpContours(gpuImgHSV);
	yellow.findTarpContours(gpuImgHSV);
	printTime("Contours", stepTime);

	/*----- Areas -----*/
	blue.findTarpArea();
	pink.findTarpArea();
	yellow.findTarpArea();
	printTime("Areas", stepTime);



	vector<vector<Point> > contours_approx = findContours(gpuImgHSV, pink_low, pink_high);

    

	//Determine which contour matches the tarp. Initially, assume all contours are valid.
    
    vector<bool> validContour(contours_approx.size(), true);

	//First, remove contours with > 10 vertices
    for(unsigned int i = 0; i < contours_approx.size(); i++)
    {
    	//Filter by number of vertices
    	if(validContour[i] && (contours_approx[i].size() > 10))
    	{
    		validContour[i] = false;
    	}
    }


    //Get average color & std deviation of each contour location
    for(unsigned int i = 0; i < contours_approx.size(); i++)
    {
    	if(validContour[i])
    	{
			Mat mask(rrows,rcols,CV_8UC1, Scalar(0)); //Initialize

			drawContours(mask, contours_approx, i, Scalar(255), -1, 8); //Draw filled in mask
			Scalar contourMean = mean(splitImgHSV[0],mask); //Gets average value of the points inside the mask
    	}
    }

    vector<double> area(contours_approx.size(), 0);

    //Get area of each contour
    for(unsigned int i = 0; i < area.size(); i++)
    {
    	if(validContour[i]){
    		area[i] = contourArea(contours_approx[i]);
    	}
    }


    sort(area.begin(), area.end(), greater<int>());


	//Draw contours on image. Eventually, only 1 contour per tarp will need to be drawn
	for(unsigned int i = 0; i< contours_approx.size(); i++ )
	{
		Scalar color = Scalar(255,255,255);
		if(validContour[i]){
			drawContours( cameraImgBGRSmall, contours_approx, 0, color, -1, 8);
		}
	}



	//Display window containing thresholded tarp
    imshow("Final Image", cameraImgBGRSmall);
    waitKey(0); //Wait for any key press before closing window

    //NOTE: Failing to close the display window before running a new iteration of the code
    //can result in GPU memory errors

    //Save image
    imwrite("/home/jwapman/Eclipse_Workspace/Target_Detection/Images/Output_Image.jpg",cameraImgBGRSmall);

    //Free GPU Resources
	cuda::resetDevice();

    return 0;

}

vector<vector<Point> > findContours(cuda::GpuMat gpuImgHSV, int* low_thresh, int* high_thresh)
{
	//Apply thresholding.
	//This isolates the desired color and makes it easier for the following
	//edge detection algorithm to identify the tarps
	cuda::GpuMat gpuThresh;

	gpuInRange(gpuImgHSV,gpuThresh,low_thresh,high_thresh);

	Mat cpuThresh(gpuThresh); //Save to CPU

	vector<vector<Point> > contours, contours_approx;
	vector<Vec4i> hierarchy;

	findContours( cpuThresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0) );
	contours_approx.resize(contours.size());
	//Appromiximate/smooth contours. Typically 1% - 5% error is typical.
	//Additionally, this creates closed loops of all contours. (Needed to find center/average color)
    for( size_t k = 0; k < contours.size(); k++ )
    {
        approxPolyDP(Mat(contours[k]), contours_approx[k], 0.01*arcLength(contours[k],true), true);
    }

    return contours_approx;

}


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

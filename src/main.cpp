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
#include <future>
#include <chrono>

#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;
using namespace boost::filesystem;

//Forware-declare functions
Mat getImage();
void printTime(String operation, TickMeter& tm);
void saveImage(Mat& img, string path);


int main(int argc, char** argv )
{

	/*----- COLOR VARIABLES -----*/

	// Ranges of colors to look for in HSV color space
	//TODO Get more precise values
	int blue_ideal[3] = {0,0,0};
	int blue_low[3] = {200,65,40};
	int blue_high[3] = {240,100,100};

	int pink_ideal[3] = {0,0,0};
	int pink_low[3] = {300,20,50};
	int pink_high[3] = {340,60,100};

	int yellow_ideal[3] = {0,0,0};
	int yellow_low[3] = {45,20,50};
	int yellow_high[3] = {60,100,100};



	/*----- INITIALIZATION -----*/

	//Check if CUDA-enabled GPU can be accessed
	if(cuda::getCudaEnabledDeviceCount() < 1)
	{
		cerr << "No CUDA-enabled GPU detected" << endl;
		return 1;
	}

	//Initialize GPU
	cuda::setDevice(0);
	cuda::resetDevice();



	//Create Tarp Objects
	Tarp blue("Blue", blue_ideal, blue_low, blue_high);
	Tarp pink("Pink", pink_ideal, pink_low, pink_high);
	Tarp yellow("Yellow", yellow_ideal, yellow_low, yellow_high);



	/*----- SET UP FOLDER -----*/
	//path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images"));
	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder
	recursive_directory_iterator end_itr;

	vector<thread> images;

    /*----- PROCESS ALL IMAGES IN FOLDER -----*/
    for (recursive_directory_iterator itr(p); itr != end_itr; ++itr)
    {
    	//Path strings
		string currentFilePath = itr->path().string();
		string currentFileName = itr->path().filename().string();
		cout << "Reading " << currentFileName << endl;

		//If directory, make folder and continue
		if (is_directory(itr->path()))
		{
			size_t index = 0;
			string outputDirectoryPath = currentFilePath;
			index = outputDirectoryPath.find("Input", index);
			outputDirectoryPath.replace(index,5,"Output"); //Replace "Input" with "Output

			create_directory(outputDirectoryPath);

			continue;

		}

		/*----- READ & CHECK -----*/

		//Import image. imread imports in BGR format.
		Mat cameraImgBGR = imread(currentFilePath, CV_LOAD_IMAGE_COLOR);

		//Start timer
		TickMeter stepTime;
		TickMeter totalTime;
		stepTime.start();
		totalTime.start();

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
			cout << "No image detected" << endl;
			continue; //Error code that no data was gathered
		}

		Mat cameraImgBGRSmall(rrows,rcols,imgType);

		printTime("Check Image", stepTime);

		//Run multiple times to get accurate timing info. First iteration
		//Is always slower than normal


		/*----- RESIZE/FILTER IMAGE -----*/

		//Resize with CPU. Faster than resizing using GPU due to memory latency

		resize(cameraImgBGR,cameraImgBGRSmall,Size(),0.25,0.25,INTER_LINEAR);
		cuda::GpuMat gpuCameraImgBGRSmall(cameraImgBGRSmall);
		printTime("Resize CPU", stepTime);

//		//Declare GPU matrices to hold converted color space
//		cuda::GpuMat gpuImgHSV(rrows,rcols,imgType);
//
//		//Convert color space to HSV using GPU
		cuda::GpuMat gpuImgHSV;
		cuda::cvtColor(gpuCameraImgBGRSmall, gpuImgHSV, CV_BGR2HSV,0);
		Mat imgHSV(gpuImgHSV);
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

		printTime("Convert Color", stepTime);

		//Blur image (Must use CPU for a 3-channel image)
		boxFilter(imgHSV,imgHSV,-1,Size(5,5));
		gpuImgHSV.upload(imgHSV);


		printTime("Blur", stepTime);


		/*----- PER-TARP OPERATIONS -----*/


		vector<vector<Point> > finalContours(3);

		//Threading option
//		thread findBlue(&Tarp::findBestTarp,&blue, ref(imgHSV), ref(splitImgHSV),ref(finalContours[0]));
//		thread findPink(&Tarp::findBestTarp,&pink, ref(imgHSV), ref(splitImgHSV),ref(finalContours[1]));
//		thread findYellow(&Tarp::findBestTarp,&yellow, ref(imgHSV), ref(splitImgHSV),ref(finalContours[2]));
//		findBlue.join();
//		findPink.join();
//		findYellow.join();

		//Sequential option
		blue.findBestTarp(imgHSV, splitImgHSV, finalContours[0]);
		pink.findBestTarp(imgHSV, splitImgHSV, finalContours[1]);
		yellow.findBestTarp(imgHSV, splitImgHSV, finalContours[2]);

		pink.findBestTarpORB(imgHSV);

		printTime("Decision", stepTime);

		/*----- DISPLAY RESULTS -----*/

		vector<vector<Point> > contours_poly( finalContours.size() );
		vector<Rect> boundRect( finalContours.size() );
		vector<Point2f>center( finalContours.size() );
		vector<float>radius( finalContours.size() );

		for(unsigned int i = 0; i < finalContours.size(); i++ )
		 {
			if(finalContours[i].size() > 0){
				boundRect[i] = boundingRect( Mat(finalContours[i]) );
			}
		 }


		//Draw contours on image.
		const Scalar color[3] = {Scalar(0,0,255),Scalar(255,0,0),Scalar(0,255,0)};
		for(unsigned int i = 0; i< finalContours.size(); i++ )
		{
			if(finalContours[i].size() > 0){
				drawContours( cameraImgBGRSmall, finalContours, i, color[i], 3, 8);
				//rectangle( cameraImgBGRSmall, boundRect[i].tl(), boundRect[i].br(), color[i], 2, 8, 0 );
			}
			else
			{
				cout << "No valid tarp" << endl;
			}
		}

		printTime("Draw Contours", stepTime);

		//Display window containing thresholded tarp
//		namedWindow("Final Image",WINDOW_NORMAL);
//		resizeWindow("Final Image",600,600);
//		imshow("Final Image", cameraImgBGRSmall);
//		waitKey(0); //Wait for any key press before closing window

		//NOTE: Failing to close the display window before running a new iteration of the code
		//can result in GPU memory errors

		//Save image file
		saveImage(cameraImgBGRSmall, currentFilePath);

		printTime("Save Image", stepTime);

		printTime("Total Time", totalTime);

		cout << endl << endl;

    }

    /*----- EXIT PROGRAM -----*/

    for(unsigned int i = 0; i < images.size(); i++)
    {
    	images[i].join();
    	//cout << i << " Joined" << endl;
    }

	cuda::resetDevice();

    return 0;

}

/*---------- CUSTOM FUNCTIONS ----------*/

//Function to import an image. Currently only reads files from filesystem.
//In the future, expand to include code for accessing the camera
void saveImage(Mat& img, string path)
{
	size_t index = 0;
	string writePath = path;
	index = writePath.find("Input", index);
	writePath.replace(index,5,"Output"); //Replace "Input" with "Output
	cout << writePath << endl;
	imwrite(writePath,img);

}


Mat getImage()
{
	return imread("~/Eclipse_Workspace/Target_Detection/Images/tarps.jpg", CV_LOAD_IMAGE_COLOR);
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

/*
 * Jonathan Wapman
 * Created 12/23/2017
 * UC Davis Eclipse Rocketry Senior Design Project
 * Target Detection
 */

//System Libraries
#include <iostream> //Input/output library
#include <vector>
#include <string>
#include <algorithm> //For sorting
#include <future>
#include <chrono>
#include <queue>

//Boost
#include "boost/filesystem.hpp"

//OpenCV
#include <opencv2/opencv.hpp> //OpenCV library

//Source Files
#include "timing.h"
#include "Tarp.h"
#include "colors.h"
#include "tarpFind.h"
#include "getImages.h"
#include "saveImages.h"


//Namespaces
using namespace std;
using namespace cv;
using namespace boost::filesystem;


//Global configuration variables. False = read from filesystem;
const bool readFromCamera = false;

int main(int argc, char** argv )
{

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

	//Queues to store image data
	queue<Mat> images;
	queue<string> filePaths;
	queue<color_data> colors;



	/*----- IMAGE CAPTURE AND PROCESSING -----*/

	//Store whether all files have been read from filesystem
	bool allFilesRead = false;

	//IMPORTANT: Camera capture & image processing will continue as long as "run" is true.
	//Exit using telemetry sensors
	bool run = true;


	while(run)
	{

		/*----- GET IMAGE -----*/
		if(readFromCamera == true) //Set up and read from camera. Captures 1 frame per iteration
		{

		}
		else //Read from filesystem. Captures entire queue
		{
			if(allFilesRead == false)
			{
	//			path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images"));
				path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder

				images = getImages(p, ref(filePaths), ref(colors));
			}

			allFilesRead = true; //Only read once
		}

		/*----- PROCESS ALL IMAGES IN QUEUE -----*/

		//Get image from queue
		Mat cameraImgBGR = images.front(); //Get
		color_data imgColors = colors.front(); //Get
		images.pop();	//Remove
		colors.pop();	//Remove


		/*----- FIND TARP LOCATIONS -----*/
		vector<vector<Point> > finalContours = processImage(cameraImgBGR, imgColors);


		/*----- DISPLAY RESULTS -----*/
		Mat cameraImgBGRSmall = drawContours(cameraImgBGR, finalContours);


		/*----- SAVE IMAGES -----*/
		string currentFilePath = filePaths.front();
		filePaths.pop();
		saveImage(cameraImgBGRSmall, currentFilePath);

		cout << endl << endl;

		//Exit if all filesystem images have been processed
		if(filePaths.empty())
		{
			run = false;
		}

	}

    /*----- EXIT PROGRAM -----*/

	cuda::resetDevice();

    return 0;

}

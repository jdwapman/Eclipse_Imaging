/*
 * Jonathan Wapman
 * Created 12/23/2017
 * UC Davis Eclipse Rocketry Senior Design Project
 * Target Detection
 */

//System Libraries
#include <iostream>
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
const bool readFromCamera = true;

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

	/*----- INITIALIZE CAMERA -----*/

	VideoCapture cam1;
	int numImages = 0;

	if(readFromCamera == true)
	{
		cam1.open(0);

		if( !cam1.isOpened() )
		{
			cout << "***Could not initialize capturing...***\n";
			return -1;
		}

		//Camera Settings
		cam1.set(CAP_PROP_AUTO_EXPOSURE, 0.25); //Why 0.25? No idea, but it works

		cam1.set(CAP_PROP_FRAME_WIDTH, 960);
		cam1.set(CAP_PROP_FRAME_HEIGHT, 540);


		double exp = 4; //Shutter speed? Use increments of 2x or 0.5x for full stops
		cam1.set(CAP_PROP_EXPOSURE, exp/100.0);

	}

	/*----- IMAGE CAPTURE AND PROCESSING -----*/

	/*----- VARIABLES -----*/
	//Store whether all files have been read from filesystem
	bool allFilesRead = false;

	//IMPORTANT: Camera capture & image processing will continue as long as "run" is true.
	//Exit using telemetry sensors
	bool run = true;


	while(run)
	{

		/*----- GET IMAGE(S) -----*/
		if(readFromCamera == true) //Set up and read from camera. Captures 1 frame per iteration
		{
			//Capture Image
			Mat cameraFrame;
			cam1 >> cameraFrame;
			images.push(cameraFrame);
			//imshow("Camera Preview", cameraFrame);
			//waitKey(0);
			numImages++;
			color_data c;
			colors.push(c);
		}
		else //Read from filesystem. Captures entire queue
		{
			if(allFilesRead == false)
			{
//				path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images"));
				path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder

				images = getImages(p, ref(filePaths), ref(colors));
			}

			allFilesRead = true; //Only read once
		}

		/*----- PROCESS ALL IMAGES IN QUEUE -----*/
		Mat cameraImgBGR = images.front(); //Get
		images.pop();	//Remove
		color_data imgColors = colors.front(); //Get

		if(!readFromCamera)
			colors.pop();	//Remove


		/*----- FIND TARP LOCATIONS -----*/
		vector<vector<Point> > finalContours = processImage(cameraImgBGR, imgColors);


		/*----- DRAW RESULTS -----*/
		Mat cameraImgBGRSmall = drawContours(cameraImgBGR, finalContours);


		/*----- SAVE IMAGES -----*/

		string savePath;
		if(readFromCamera)
		{
			path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Output_Images/Camera_Images/img_") + to_string(numImages) + ".jpg");
			savePath = p.string();
			cout << savePath;
		}
		else //Create save path if reading from filesystem
		{
			string savePath = filePaths.front();
			filePaths.pop();

			size_t index = 0;
			index = savePath.find("Input", index);
			savePath.replace(index,5,"Output"); //Replace "Input" with "Output
		}

		saveImage(cameraImgBGRSmall, savePath);

		cout << endl << endl;


		/*----- CHECK EXIT CONDITIONS -----*/
		//Exit if all filesystem images have been processed
		if(readFromCamera == false && filePaths.empty())
		{
			run = false;
		}
		if(numImages == 100) //Exit after 100 images. //TODO: REMOVE FOR FLIGHT
		{
			run = false;
		}



	}

    /*----- EXIT PROGRAM -----*/

	cuda::resetDevice();
	cam1.release();

    return 0;

}

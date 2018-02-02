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

		cam1.set(CAP_PROP_FRAME_WIDTH, 1920);
		cam1.set(CAP_PROP_FRAME_HEIGHT, 1080);


		double exp = 4; //Shutter speed? Use increments of 2x or 0.5x for full stops
		cam1.set(CAP_PROP_EXPOSURE, exp/100.0);

	}

	/*----- IMAGE CAPTURE AND PROCESSING -----*/

	/*----- VARIABLES -----*/

	//IMPORTANT: Camera capture & image processing will continue as long as "run" is true.
	//Exit using telemetry sensors
	bool run = true;

	if(!readFromCamera)
	{
//		path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images"));
		path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder

		getImages(p, ref(filePaths), ref(colors));
	}

	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();


	while(run)
	{

		/*----- GET IMAGE(S) -----*/
		Mat cameraFrame;
		if(readFromCamera == true) //Set up and read from camera. Captures 1 frame per iteration
		{
			//Capture Image

			cam1 >> cameraFrame;
			//imshow("Camera Preview", cameraFrame);
			//waitKey(0);
			numImages++;
			color_data c;
			colors.push(c);
		}
		else //Read from filesystem. Captures entire queue
		{

		}

		/*----- PROCESS ALL IMAGES IN QUEUE -----*/
		Mat cameraImgBGR;
		if(readFromCamera)
		{
			cameraImgBGR = cameraFrame;
		}
		else //Read from filesystem
		{
			cameraImgBGR = imread(filePaths.front(), CV_LOAD_IMAGE_COLOR); //Import image. imread imports in BGR format.
			//Don't pop filepath until after save

			if(cameraImgBGR.empty())
				continue;
		}

		printTime("Read Image", stepTime);


		color_data imgColors = colors.front(); //Get
		colors.pop();	//Remove


		/*----- FIND TARP LOCATIONS -----*/
		vector<vector<Point> > finalContours = processImage(cameraImgBGR, imgColors);

		printTime("Process Image", stepTime);


		/*----- DRAW RESULTS -----*/
		Mat cameraImgBGRSmall = drawContours(cameraImgBGR, finalContours);

		printTime("Draw Contours", stepTime);

		/*----- SAVE IMAGES -----*/

		string savePath;
		if(readFromCamera)
		{
			path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Output_Images/Camera_Images/img_") + to_string(numImages) + ".jpg");
			savePath = p.string();
			//cout << savePath;
		}
		else //Create save path if reading from filesystem
		{
			savePath = filePaths.front();
			filePaths.pop();

			size_t index = 0;
			index = savePath.find("Input", index);
			savePath.replace(index,5,"Output"); //Replace "Input" with "Output
			//cout << savePath;
		}

		saveImage(cameraImgBGRSmall, savePath);

		printTime("Save Image", stepTime);

		cout << endl << endl;


		/*----- CHECK EXIT CONDITIONS -----*/
		//Exit if all filesystem images have been processed
		if(readFromCamera == false && filePaths.empty())
		{
			run = false;
		}
		if(numImages == 10) //Exit after 100 images. //TODO: REMOVE FOR FLIGHT
		{
			run = false;
		}



	}

    /*----- EXIT PROGRAM -----*/

	cuda::resetDevice();
	cam1.release();

    return 0;

}

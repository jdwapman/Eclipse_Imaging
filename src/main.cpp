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
#include "Image.h"


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

	/*----- INITIALIZE CAMERA -----*/

	VideoCapture cam1;

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




//	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images"));
	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder


	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();


	//IMPORTANT: Camera capture & image processing will continue as long as "run" is true.
	//Exit using telemetry sensors
	bool run = true;

	//Set up image filesystem class

	double scale = 1.0/4.0;

	Image im(p, scale);

	im.getImage();
	im.processImage();
	im.drawImageContours();
	im.saveImage();


    /*----- EXIT PROGRAM -----*/

	cuda::resetDevice();
	cam1.release();

    return 0;

}

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

#include "filterImgGPU.h"
#include "ImgSource.h"
#include "Image.h"
//Source Files
#include "timing.h"
#include "searchImage.h"
#include "saveImage.h"

//Namespaces
using namespace std;
using namespace cv;
using namespace boost::filesystem;


//Global configuration variables. False = read from filesystem;
const bool readCamera = false;

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

	if(readCamera)
	{
		cam1.open(0);

		if( !cam1.isOpened() )
		{
			cout << "Could not initialize capturing\n";
			return -1;
		}

		//Camera Settings
		cam1.set(CAP_PROP_AUTO_EXPOSURE, 0.25); //Why 0.25? No idea, but it works

		cam1.set(CAP_PROP_FRAME_WIDTH, 1920);
		cam1.set(CAP_PROP_FRAME_HEIGHT, 1080);


		double exp = 4; //Shutter speed? Use increments of 2x or 0.5x for full stops
		cam1.set(CAP_PROP_EXPOSURE, exp/100.0);

	}

//	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images"));
	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder

	ImgSource *src1;

	if(readCamera)
	{
		src1 = new ImgSource(cam1);
	}
	else
	{
		src1 = new ImgSource(p);
	}


	double scale = 1.0/4.0;

	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();



	/*------CAPTURE, PROCESS, AND SAVE IMAGES-----*/
	bool run = true;
	int numImages = 0;
	while(run)
	{
		Image img1 = src1->getImage();
		printTime("Get Image", stepTime);

		if(!img1.valid)
			run = false;


		if(run)
		{
			numImages++;
			Image img2 = filterImgGPU(img1, scale);
			printTime("Filter Image", stepTime);

			vector<vector<Point> > contours = searchImage(img2);
			printTime("Search Image", stepTime);

			Image img3 = drawImageContours(img1, contours, scale);
			saveImage(img3, numImages);
			printTime("Save Image", stepTime);
		}

		cout << endl << endl;

	}

    /*----- EXIT PROGRAM -----*/
	delete src1;
	cuda::resetDevice();
	cam1.release();

    return 0;

}

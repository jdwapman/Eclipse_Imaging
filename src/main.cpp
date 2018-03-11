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
#include <future>
#include <chrono>

//Boost
#include "boost/filesystem.hpp"

//OpenCV
#include <opencv2/opencv.hpp> //OpenCV library
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

//Source Files
#include "ImgSource.h"
#include "Image.h"
#include "filterImageGPU.h"
#include "saveImage.h"
#include "searchImage.h"

//Misc Source Files
#include "timing.h"


//Namespaces
using namespace std;
using namespace cv;
using namespace boost::filesystem;


/*======== IMAGE SOURCE LOCATION =======*/
const string SOURCE = "VIDEO"; //FILE, VIDEO, CAMERA

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


	//Filesystem sources
//	path p((getenv("HOME")) + string("/Eclipse/Target_Detection/Input_Images"));
//	path p((getenv("HOME")) + string("/Eclipse/Target_Detection/Input_Images/1-6 [Labeled]"));
	path p((getenv("HOME")) + string("/Eclipse/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder
//	path p((getenv("HOME")) + string("/Eclipse/Target_Detection/Input_Launch_Videos")); //Can select smaller folder
//	path p((getenv("HOME")) + string("/Eclipse/Target_Detection/Input_Launch_Videos/Nic_2")); //Can select smaller folder
//	path p((getenv("HOME")) + string("/Eclipse/Target_Detection/Input_Images/See3Cam/2-16/Manual")); //Can select smaller folder


	/*----- INITIALIZE IMAGE SOURCE -----*/

	VideoCapture cam1;
	string saveFolder, savePath;

	if(SOURCE == "CAMERA")
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

		/*----- CREATE CAMERA OUTPUT FOLDER -----*/
		//Get Date
		time_t t = time(0);
		struct tm * now = localtime(&t);

		int year = now->tm_year + 1900;
		int mon = now->tm_mon + 1;
		int day = now->tm_mday;

		int hour = now->tm_hour;
		int min = now->tm_min;
		int sec = now->tm_sec;

		saveFolder = "Cam1_" + to_string(year) + "-" + to_string(mon) + "-" + to_string(day) + "_" + to_string(hour) + ":" + to_string(min) + ":" + to_string(sec);
		savePath = ((getenv("HOME")) + string("/Eclipse/Target_Detection/Output_Images/Camera_Images/") + saveFolder);
		//savePath = (string("/media/nvidia/SD_TX1/Output_Images/Camera_Images/test/"));
		//create_directory(savePath);

	}


	/*----- SET UP IMAGE SOURCE -----*/
	ImgSource *src1;

	if(SOURCE == "CAMERA")
	{
		src1 = new ImgSource(cam1);
	}
	else if(SOURCE == "FILE")
	{
		src1 = new ImgSource(p); //Save path determined by file location
	}
	else if(SOURCE == "VIDEO")
	{
		path vp = (getenv("HOME")) + string("/Eclipse/Target_Detection/Input_Launch_Videos/flight_3/flight_3.mp4");
		savePath = vp.parent_path().string();

		size_t index = 0;
		index = savePath.find("Input", index); //TODO: Separate imagePath, saveImagePath
		savePath.replace(index,5,"Output"); //Replace "Input" with "Output

		VideoCapture vid(vp.string());
		src1 = new ImgSource(vid, vp);
	}
	else
	{
		cout << "Invalid Source" << endl;
		return -1;
	}


	double scale = 1.0/1.0; //Change this to increase speed, but decrease accuracy at high elevations

	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();


	/*------CAPTURE, PROCESS, AND SAVE IMAGES-----*/
	bool run = true;

	//End/count conditions
	int numImages = 0; //Number of images to take with camera. Set to -1 for continuous
	int i = 0;

	while(run)
	{
		Image cameraImage1 = src1->getImage();


		if(!cameraImage1.valid)
		{
			run = false;
			cout << "No Valid Image" << endl;
		}

		if(run)
		{



			numImages++;



			if(i == 8)
			{
				cout << "Iteration: " << numImages << endl;
				printTime("Get Image", stepTime);
				Image filteredImage1 = filterImageGPU(cameraImage1, scale);
				printTime("Filter Image", stepTime);

				vector<vector<Point> > contours1 = searchImage(filteredImage1);
				printTime("Search Image", stepTime);


				Image contourImage1 = drawImageContours(cameraImage1, contours1, scale);


				saveImage(contourImage1, numImages, savePath);
				printTime("Save Image", stepTime);
				i = 0;
				//continue;
				cout << endl << endl;
			}

			i += 1;



		}



	}

    /*----- EXIT PROGRAM -----*/
	delete src1;
	cuda::resetDevice();
	cam1.release();

    return 0;

}

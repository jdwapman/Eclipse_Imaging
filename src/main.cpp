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


	//Filesystem sources
	//	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images"));
//	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder
//	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Launch_Videos")); //Can select smaller folder
	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Launch_Videos/Nic_2")); //Can select smaller folder


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

	string videoSaveFolder = to_string(year) + "-" + to_string(mon) + "-" + to_string(day) + "_" + to_string(hour) + ":" + to_string(min) + ":" + to_string(sec);
	string videoSavePath((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Output_Images/Camera_Images/") + videoSaveFolder);
	create_directory(videoSavePath);

	/*----- SET UP IMAGE SOURCE -----*/
	ImgSource *src1;

	if(readCamera)
	{
		src1 = new ImgSource(cam1);
	}
	else
	{
		//src1 = new ImgSource(p);
		path vp = (getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Launch_Videos/Nic_2/YDXJ0439.mp4");
		videoSavePath = vp.parent_path().string();

		size_t index = 0;
		index = videoSavePath.find("Input", index); //TODO: Separate imagePath, saveImagePath
		videoSavePath.replace(index,5,"Output"); //Replace "Input" with "Output

		VideoCapture vid(vp.string());
		src1 = new ImgSource(vid, vp);
	}


	double scale = 1.0/1.0;

	//Start timer
	TickMeter stepTime;
	TickMeter totalTime;
	stepTime.start();
	totalTime.start();


	/*------CAPTURE, PROCESS, AND SAVE IMAGES-----*/
	bool run = true;
	int numImages = 0;


	//Create Tracker
	Ptr<Tracker> tracker;
	tracker = TrackerMedianFlow::create();


	bool init = true;

	while(run)
	{
		Image cameraImage1 = src1->getImage();


		if(!cameraImage1.valid)
			run = false;


		if(run)
		{

			printTime("Get Image", stepTime);

			numImages++;
			Image filteredImage1 = filterImageGPU(cameraImage1, scale);
			printTime("Filter Image", stepTime);

			vector<vector<Point> > contours1 = searchImage(filteredImage1);
			printTime("Search Image", stepTime);

			//Track Image
			vector<Rect> boundRect( contours1.size() );

			for(unsigned int i = 0; i < contours1.size(); i++ )
			 {
				if(contours1[i].size() > 0){
					boundRect[i] = boundingRect( Mat(contours1[i]) );
				}
			 }

			//Tarp bounding boxes using contours
//			Rect2d bblue(boundRect[0].tl, boundRect[0].br());
//			Rect2d bpink(boundRect[1].tl, boundRect[1].br());
//			Rect2d byellow(boundRect[2].tl, boundRect[2].br());


//			if(init) //If first iteration, initialize to contours's rectangle
//			{
//				tracker->init(filteredImage1.img, bblue);
//				init = false;
//			}




			Image contourImage1 = drawImageContours(cameraImage1, contours1, scale);





			saveImage(contourImage1, numImages, videoSavePath);
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

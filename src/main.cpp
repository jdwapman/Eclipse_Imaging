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
const bool tracking = true;

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
	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Images/Selected_Images")); //Can select smaller folder
//	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Launch_Videos")); //Can select smaller folder
//	path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Launch_Videos/Nic_2")); //Can select smaller folder


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

		//Uncomment to read images from source folder
//		src1 = new ImgSource(p);


		//Uncomment to read images from video
//		path vp = (getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Launch_Videos/Nic_2/YDXJ0439.mp4");
		path vp = (getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Input_Launch_Videos/Backyard/out.mp4");
		videoSavePath = vp.parent_path().string();

		size_t index = 0;
		index = videoSavePath.find("Input", index); //TODO: Separate imagePath, saveImagePath
		videoSavePath.replace(index,5,"Output"); //Replace "Input" with "Output

		VideoCapture vid(vp.string());
		src1 = new ImgSource(vid, vp);
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


	//Create Tracker
	Ptr<Tracker> tracker;
	tracker = TrackerMedianFlow::create(); //boosting

	Mat lastTarp1;
	vector<vector<Point> > lastContours1;
	Rect2d bblue;
	bool first = true;
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
			lastTarp1 = filteredImage1.img; //Save previous image to use as seed for tracking algorithm
			printTime("Filter Image", stepTime);

			vector<vector<Point> > contours1 = searchImage(filteredImage1);
			lastContours1 = contours1; //Save contours to use as seed for tracking algorithm
			printTime("Search Image", stepTime);



			Image contourImage1 = cameraImage1;

			//Initialize the tracker if there is a valid tarp
//			if(1) //Disable tracking detection
			if((contours1[0].size() != 0) && first == true) //If there is a blue tarp
			{

				cout << contours1[0].size() << endl;
				cout << first << endl;
				//Track Image
				vector<Rect> boundRect( contours1.size() ); //Vector of bounding rectangles for each tarp

				for(unsigned int i = 0; i < contours1.size(); i++ ) //Set the tracking bounding rect to the detection rectangles
				 {
					if(contours1[i].size() > 0){
						boundRect[i] = boundingRect( Mat(contours1[i]) );
					}
				 }

				//Define initial bounding boxes using contours
				bblue.x = boundRect[0].x / scale;
				bblue.y = boundRect[0].y / scale;
				bblue.width = boundRect[0].width / scale;
				bblue.height = boundRect[0].height / scale;



				tracker->init(cameraImage1.img, bblue); //Initialize tracker to blue bounding box
				contourImage1 = drawImageContours(cameraImage1, contours1, scale);

				first = false;

			}
			else //If no blue tarp is found
			{
				bool ok = tracker->update(cameraImage1.img, bblue);
				cout << ok << endl;
				contourImage1 = cameraImage1;
				rectangle(contourImage1.img, bblue, Scalar(50,0,0), 6, 1);

				cout << "Tracking" << endl;

			}








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

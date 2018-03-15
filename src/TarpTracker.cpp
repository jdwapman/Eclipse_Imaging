/*
 * TarpTracker.cpp
 *
 *  Created on: Mar 14, 2018
 *      Author: jwapman
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


#include "TarpTracker.h"



TarpTracker::TarpTracker() {

	this->tracker = TrackerMedianFlow::create();
	this->initialized = false;

}

TarpTracker::~TarpTracker() {

}

Rect2d TarpTracker::track(const Image& cameraImage, const Rect2d& bbox)
{

	Rect2d trackBox;

	if(bbox.area() != 0) //If bounding box exists, update seed
	{
		this->tracker->init(cameraImage.img, bbox);
		this->initialized = true;
	}
	else //No tarp detected
	{
		if(this->initialized) //If there is a previous bounding box
		{
			bool ok = tracker->update(cameraImage.img, trackBox);
		}
	}

	return trackBox;

}

//				//Track Image
//				vector<Rect> boundRect( contours1.size() ); //Vector of bounding rectangles for each tarp
//
//				for(unsigned int i = 0; i < contours1.size(); i++ ) //Set the tracking bounding rect to the detection rectangles
//				 {
//					if(contours1[i].size() > 0){
//						boundRect[i] = boundingRect( Mat(contours1[i]) );
//					}
//				 }
//
//				//Define initial bounding boxes using contours
//				bblue.x = boundRect[0].x / scale;
//				bblue.y = boundRect[0].y / scale;
//				bblue.width = boundRect[0].width / scale;
//				bblue.height = boundRect[0].height / scale;
//
//
//
//				rectangle(contourImage1.img, bblue, Scalar(50,0,0), 6, 1);



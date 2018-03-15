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

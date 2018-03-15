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

	Rect2d trackBox = bbox;

	Rect2d expBox;
	double scale = 4.0;
	expBox.height = bbox.height * scale;
	expBox.width = bbox.width * scale;
	expBox.x = bbox.x + bbox.height/2 - expBox.height/2;
	expBox.y = bbox.y + bbox.width/2 - expBox.width/2;


	if(bbox.area() > 200) //If bounding box exists, update seed
	{
		cout << "Init" << endl;

		this->tracker->init(cameraImage.img, expBox);
		this->initialized = true;
	}
	else //No tarp detected
	{
		if(this->initialized) //If there is a previous bounding box
		{
			cout << "Updating: ";
			Rect2d trackBoxScaled;
			bool ok = tracker->update(cameraImage.img, trackBoxScaled);

			trackBox.height = trackBoxScaled.height / scale;
			trackBox.width = trackBoxScaled.width / scale;

			trackBox.x = trackBoxScaled.x + trackBoxScaled.height/2 - trackBox.height/2;
			trackBox.y = trackBoxScaled.y + trackBoxScaled.width/2 - trackBox.width/2;

			cout << ok << endl;
		}
	}



	return trackBox;

}

/*
 * TarpTracker.h
 *
 *  Created on: Mar 14, 2018
 *      Author: jwapman
 */

#ifndef SRC_TARPTRACKER_H_
#define SRC_TARPTRACKER_H_


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



class TarpTracker {


	Ptr<Tracker> tracker;

	bool initialized;


public:
	Rect2d track(const Image& cameraImage, const Rect2d& bbox);
	TarpTracker();
	virtual ~TarpTracker();
};


#endif /* SRC_TARPTRACKER_H_ */

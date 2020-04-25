/*
 * TarpTracker.h
 *
 *  Created on: Mar 14, 2018
 *      Author: jwapman
 */

#ifndef SRC_TARPTRACKER_H_
#define SRC_TARPTRACKER_H_

// System Libraries
#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <vector>

// Boost
#include "boost/filesystem.hpp"

// OpenCV
#include <opencv2/core/ocl.hpp>
#include <opencv2/opencv.hpp>  //OpenCV library
#include <opencv2/tracking.hpp>

// Source Files
#include "Image.h"
#include "ImgSource.h"
#include "filterImageGPU.h"
#include "saveImage.h"
#include "searchImage.h"

// Misc Source Files
#include "timing.h"

// Namespaces
using namespace std;
using namespace cv;
using namespace boost::filesystem;

class TarpTracker
{
  Ptr<Tracker> tracker;

  bool initialized;

 public:
  Rect2d track(const Image& cameraImage, const Rect2d& bbox);
  TarpTracker();
  virtual ~TarpTracker();
};

#endif /* SRC_TARPTRACKER_H_ */

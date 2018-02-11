/*
 * saveImage.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_SAVEIMAGE_H_
#define SRC_SAVEIMAGE_H_

#include <iostream> //Input/output library
#include <vector>
#include <string>
#include <algorithm> //For sorting
#include <opencv2/opencv.hpp> //OpenCV library
#include "Tarp.h"
#include "colors.h"
#include "timing.h"
#include <future>
#include <chrono>

#include "boost/filesystem.hpp"
#include "Image.h"

void saveImage(Image img, int numImages);
Image drawImageContours(Image img, vector<vector<Point> > contours, double scale);


#endif /* SRC_SAVEIMAGE_H_ */

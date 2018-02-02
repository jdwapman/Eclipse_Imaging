/*
 * getImages.h
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#ifndef SRC_GETIMAGES_H_
#define SRC_GETIMAGES_H_

#include <queue>

#include <opencv2/opencv.hpp> //OpenCV library

//Boost
#include "boost/filesystem.hpp"

#include "colors.h"

//Namespaces
using namespace std;
using namespace cv;
using namespace boost::filesystem;


queue<Mat> getImages(path p, queue<string>& filePaths, queue<color_data>& colors); //Get images from filesystem

queue<Mat> getImages(VideoCapture cam1); //Get images from cameras. Eventually add one more camera


#endif /* SRC_GETIMAGES_H_ */

/*
 * tarpFind.h
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#ifndef SRC_TARPFIND_H_
#define SRC_TARPFIND_H_

#include <opencv2/opencv.hpp> //OpenCV library
#include "colors.h"

using namespace cv;

vector<vector<Point> > processImage(Mat cameraImgBGR, color_data colors);

#endif /* SRC_TARPFIND_H_ */

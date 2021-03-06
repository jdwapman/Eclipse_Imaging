/*
 * searchImage.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_SEARCHIMAGE_H_
#define SRC_SEARCHIMAGE_H_

#include <opencv2/opencv.hpp>  //OpenCV library
#include <vector>
#include "Image.h"
#include "colors.h"

using namespace std;
using namespace cv;

vector<Rect2d> searchImage(Image filteredImage, double scale);

#endif /* SRC_SEARCHIMAGE_H_ */

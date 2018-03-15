/*
 * saveImage.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_SAVEIMAGE_H_
#define SRC_SAVEIMAGE_H_

#include <vector>

#include <opencv2/opencv.hpp> //OpenCV library

#include "Image.h"

void saveImage(Image img, int numImages, string cameraSavePath);
Image drawImageContours(Image img, vector<vector<Point> > contours);


#endif /* SRC_SAVEIMAGE_H_ */

/*
 * saveImage.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_SAVEIMAGE_H_
#define SRC_SAVEIMAGE_H_

#include <vector>

#include <opencv2/opencv.hpp>  //OpenCV library

#include "Image.h"

void saveImage(Image img, int numImages, string cameraSavePath);
Rect2d contour2rect(vector<Point> contour);
Image drawImageContours(const Image& img,
                        const vector<vector<Point> >& contours);
Image drawImageBBoxes(const Image& img, const vector<Rect2d>& bboxes);

#endif /* SRC_SAVEIMAGE_H_ */

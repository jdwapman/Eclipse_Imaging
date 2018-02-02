/*
 * saveImages.h
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#ifndef SRC_SAVEIMAGES_H_
#define SRC_SAVEIMAGES_H_

//OpenCV
#include <opencv2/opencv.hpp> //OpenCV library
#include <string>

using namespace std;
using namespace cv;

void saveImage(const Mat& img, const string path);
Mat drawContours(Mat& image, vector<vector<Point> > finalContours);

#endif /* SRC_SAVEIMAGES_H_ */

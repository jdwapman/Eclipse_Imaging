/*
 * timing.h
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#ifndef SRC_TIMING_H_
#define SRC_TIMING_H_

#include <opencv2/opencv.hpp>  //OpenCV library

using namespace std;
using namespace cv;

// Output elapsed time since last printTime() operation. Useful for determining
// runtime of given step.
void printTime(String operation, TickMeter& tm);

#endif /* SRC_TIMING_H_ */

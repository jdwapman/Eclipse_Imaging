/*
 * filterImg.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_FILTERIMAGEGPU_H_
#define SRC_FILTERIMAGEGPU_H_

#include "opencv2/opencv.hpp"
#include "Image.h"

using namespace cv;

Image filterImageGPU(Image& imgBGR, double scale);


#endif /* SRC_FILTERIMAGEGPU_H_ */

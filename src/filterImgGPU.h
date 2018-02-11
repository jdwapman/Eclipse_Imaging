/*
 * filterImg.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_FILTERIMGGPU_H_
#define SRC_FILTERIMGGPU_H_

#include "opencv2/opencv.hpp"
#include "Image.h"

using namespace cv;

Image filterImgGPU(Image& imgBGR, double scale);


#endif /* SRC_FILTERIMGGPU_H_ */

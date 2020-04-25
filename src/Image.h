/*
 * Image.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_IMAGEOLD_H_
#define SRC_IMAGEOLD_H_

#include <opencv2/opencv.hpp>
#include <string>

#include "colors.h"

// Boost
#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;
using namespace boost::filesystem;

// Stores a Mat alongside its path and calibration color data.
struct Image
{
  Mat img;
  string imgPath;
  color_data imgColors;
  bool valid;
};

#endif /* SRC_IMAGE_H_ */

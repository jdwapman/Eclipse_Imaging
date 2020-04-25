/*
 * ImgSource.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_IMGSOURCE_H_
#define SRC_IMGSOURCE_H_

#include <opencv2/opencv.hpp>
#include <queue>
#include <string>

#include "Image.h"
#include "colors.h"

// Boost
#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;
using namespace boost::filesystem;

class ImgSource
{
  // Capture Source
  VideoCapture cam;

  // Read Path
  path folderPath;
  path videoPath;

  // Functions to read from filesystem
  void getFileImages();
  color_data getFileColors(path currentFilePath);

  // Queues to store image data when reading from filesystem
  queue<string> filePaths;
  queue<color_data> colors;

  bool readCamera;
  bool readVideo;

 public:
  ImgSource(VideoCapture cam);
  ImgSource(path folderPath);
  ImgSource(VideoCapture video, path videoPath);

  virtual ~ImgSource();

  Image getImage();
};

#endif /* SRC_IMGSOURCE_H_ */

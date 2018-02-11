/*
 * ImgSource.h
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#ifndef SRC_IMGSOURCE_H_
#define SRC_IMGSOURCE_H_

#include <opencv2/opencv.hpp>
#include <string>
#include <queue>

#include "colors.h"
#include "Image.h"

//Boost
#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;
using namespace boost::filesystem;


class ImgSource
{

	//Capture Source
	VideoCapture cam;

	//Read Path
	path folderPath;

	//Functions to read from filesystem
	void getFileImages();
	color_data getFileColors(path currentFilePath);

	//Queues to store image data when reading from filesystem
	queue<string> filePaths;
	queue<color_data> colors;

	bool readCamera;

public:

	ImgSource(VideoCapture cam);
	ImgSource(path folderPath);

	virtual ~ImgSource();

	Image getImage();

};



#endif /* SRC_IMGSOURCE_H_ */

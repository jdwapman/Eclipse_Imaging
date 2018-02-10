/*
 * Image.h
 *
 *  Created on: Feb 9, 2018
 *      Author: jwapman
 */

#ifndef SRC_IMAGE_H_
#define SRC_IMAGE_H_

#include <opencv2/opencv.hpp>
#include <string>
#include <queue>

#include "colors.h"

//Boost
#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;
using namespace boost::filesystem;

class Image
{

	//Image matrices
	Mat cameraImgBGR;
	Mat cameraImgBGRSmall;
	Mat cameraImgBGRContours;

	//Image processing options
	double scale;

	int numImages;

	//Image sources
	bool readCamera; //Stores whether the image source is the camera or the filesystem
	VideoCapture cam;

	path folderPath;
	string imagePath;

	//Queues to store image data when reading from filesystem
	queue<string> filePaths;
	queue<color_data> colors;

	//Processed image data
	vector<vector<Point> > finalContours;

	void getFileImages();
	color_data getFileColors(path currentFilePath);

public:

	Image(VideoCapture cam, double scale); //Image source = camera
	Image(path folderPath, double scale); //Image source = filesystem
	virtual ~Image();

	//Get Images
	bool getImage();

	void processImage();

	//Save Images without contour labels
	void saveImage();
	void drawImageContours();

	//Save contour output to a text file
	void saveContoursTextLarge(); //For full resolution image
	void saveContoursTextSmall(); //For reduced-resolution image
};



#endif /* SRC_IMAGE_H_ */

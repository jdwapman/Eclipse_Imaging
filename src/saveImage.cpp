/*
 * saveImage.cpp
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#include <iostream> //Input/output library
#include <vector>
#include <string>
#include <ctime>

#include <opencv2/opencv.hpp> //OpenCV library


#include "boost/filesystem.hpp"
#include "Image.h"

void saveImage(Image img, int numImages, string cameraSavePath)
{

	string savePath = img.imgPath;

	if(img.imgPath == "") //Save numerically
	{
		path p(cameraSavePath + string("/img_") + to_string(numImages) + ".jpg");
		savePath = p.string();
	}
	else //Save with original filename in output folder
	{
		size_t index = 0;
		index = savePath.find("Input", index); //TODO: Separate imagePath, saveImagePath
		savePath.replace(index,5,"Output"); //Replace "Input" with "Output
	}

	imwrite(savePath,img.img);
}

//Draws contours on top of cameraImgBGR.
Image drawImageContours(Image img, vector<vector<Point> > contours, double scale)
{

	Image drawImg = img;

	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect( contours.size() );
	vector<Point2f>center( contours.size() );
	vector<float>radius( contours.size() );

	for(unsigned int i = 0; i < contours.size(); i++ )
	 {
		if(contours[i].size() > 0){
			boundRect[i] = boundingRect( Mat(contours[i]) );
		}
	 }

	//Draw contours on image.
	const Scalar color[3] = {Scalar(50,0,0),Scalar(0,0,255),Scalar(24,130,0)};

	for(unsigned int i = 0; i < contours.size(); i++)
	{
		for(unsigned int j = 0; j < contours[i].size(); j++)
		{
			contours[i][j].x = contours[i][j].x / scale;
			contours[i][j].y = contours[i][j].y / scale;
		}
	}


	for(unsigned int i = 0; i< contours.size(); i++ )
	{
		if(contours[i].size() > 0){
			drawContours(drawImg.img, contours, i, color[i], 5, 8);
//			rectangle( drawImg, boundRect[i].tl(), boundRect[i].br(), color[i], 10, 8, 0 );
		}
	}

	//Display window containing thresholded tarp
//	namedWindow("Final Image",WINDOW_NORMAL);
//	resizeWindow("Final Image",600,600);
//	imshow("Final Image", cameraImgBGRSmall);
//	waitKey(0); //Wait for any key press before closing window

	return drawImg;
}

/*
 * saveImage.cpp
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#include <iostream>
#include <vector>
#include <string>
#include <ctime>

#include <opencv2/opencv.hpp>


#include "boost/filesystem.hpp"
#include "Image.h"

void saveImage(Image img, int numImages, string cameraSavePath)
{

	string savePath = img.imgPath;

	if(img.imgPath == "") //No path from filesystem. Save by number in folder
	{
		stringstream num;
		num << setw(7) << setfill('0') << numImages;
		path p(cameraSavePath + string("/img_") + num.str() + ".jpg");
		savePath = p.string();
	}
	else //Save with original filename in output folder
	{
		size_t index = 0;
		index = savePath.find("Input", index);
		savePath.replace(index,5,"Output"); //Replace "Input" with "Output
	}

	imwrite(savePath,img.img);
}

Rect2d contour2rect(vector<Point> contour, double scale)
{

	Rect2d rect;

	Rect2d boundRect;

	boundRect = boundingRect( Mat(contour) );

	//Define initial bounding boxes using contours
	rect.x = boundRect.x / scale;
	rect.y = boundRect.y / scale;
	rect.width = boundRect.width / scale;
	rect.height = boundRect.height / scale;

	return rect;

}

//Draws contours on top of image.
Image drawImageContours(Image img, vector<vector<Point> > contours)
{

	Image drawImg = img;

	//Draw contours on image.
	const Scalar color[3] = {Scalar(50,0,0),Scalar(0,0,255),Scalar(24,130,0)};

	for(unsigned int i = 0; i< contours.size(); i++ )
	{
		if(contours[i].size() > 0){
			drawContours(drawImg.img, contours, i, color[i], 5, 8);
		}
	}

	return drawImg;
}

//Displays bounding rectangles on top of image
Image drawImageBBoxes(Image img, vector<Rect2d> bboxes)
{

	Image drawImg = img;

	//Draw contours on image.
	const Scalar color[3] = {Scalar(50,0,0),Scalar(0,0,255),Scalar(24,130,0)};


	for(unsigned int i = 0; i< bboxes.size(); i++ )
	{
		if(bboxes[i].area() > 0){
			rectangle( drawImg.img, bboxes[i].tl(), bboxes[i].br(), color[i], 5, 8, 0 );
		}
	}

	return drawImg;
}

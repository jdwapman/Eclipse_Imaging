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

	Rect2d boundRect;
	vector<Rect> boundRect( contours1.size() ); //Vector of bounding rectangles for each tarp


		if(contours1[i].size() > 0){
			boundRect[i] = boundingRect( Mat(contours1[i]) );
		}
	 }

	//Define initial bounding boxes using contours
	bblue.x = boundRect[0].x / scale;
	bblue.y = boundRect[0].y / scale;
	bblue.width = boundRect[0].width / scale;
	bblue.height = boundRect[0].height / scale;
}

//Draws contours on top of image.
Image drawImageContours(Image img, vector<vector<Point> > contours, double scale)
{

	Image drawImg = img;

	vector<Rect> boundRect( contours.size() );

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
//			drawContours(drawImg.img, contours, i, color[i], 5, 8);
			rectangle( drawImg.img, boundRect[i].tl(), boundRect[i].br(), color[i], 5, 8, 0 );
		}
	}

	return drawImg;
}

/*
 * Image.cpp
 *
 *  Created on: Feb 9, 2018
 *      Author: jwapman
 */

#include "Image.h"
#include "colors.h"

#include <opencv2/opencv.hpp>
#include <string>
#include <queue>

//Boost
#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;
using namespace boost::filesystem;

//Camera source constructor
Image::Image(VideoCapture cam, double scale)
{
	this->readCamera = true;

	this->cam = cam; //OpenCV camera object

	this->scale = scale;

	numImages = 0;
}

//Filesystem source constructor
Image::Image(path folderPath, double scale)
{
	this->readCamera = false;

	this->folderPath = folderPath;

	this->scale = scale;

	numImages = 0;

	//Add all images and colors to queue
	this->getFileImages();
}

Image::~Image()
{

}

//Get image. Automatically picks source
bool Image::getImage()
{

	numImages++;

	if(readCamera)
	{
		//Read image from camera
		this->cam >> this->cameraImgBGR;

		color_data c;
		this->colors.push(c);

		if(this->cameraImgBGR.empty())
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		//Import image. imread imports in BGR format.
		this->cameraImgBGR = imread(this->filePaths.front(), CV_LOAD_IMAGE_COLOR); //Read
		this->imagePath = this->filePaths.front(); //Save path of image
		this->filePaths.pop(); //Remove

		if(this->cameraImgBGR.empty())
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

color_data Image::getFileColors(path currentFilePath)
{

	/*
	 * Usage:
	 * Image must be in the following format:
	 *
	 * #Blue
	 * lowh,lohs,lowv
	 * highh,highs,highv
	 * idealh,ideals,idealv
	 *
	 * #Pink
	 * lowh,lohs,lowv
	 * highh,highs,highv
	 * idealh,ideals,idealv
	 *
	 * #Yellow
	 * lowh,lohs,lowv
	 * highh,highs,highv
	 * idealh,ideals,idealv
	 *
	 *
	 */

	color_data colors;

	vector<int> v;

	string parentPath = currentFilePath.parent_path().string();
	string calibrationPath = parentPath.append("/colors.txt");

	if(exists(calibrationPath))
	{

		//cout << calibrationPath << endl;

		ifstream f;
		f.open(calibrationPath);

		while (f)
		  {
		    string s;
		    if (!getline( f, s )) break;

		    istringstream ss( s );

		    while (ss)
		    {
		      string s;
		      if (!getline( ss, s, ',' )) break;

		      if(s.at(0) == '#') break;

		      v.push_back( stoi(s) );
		    }
		  }
		//Get colors from file.
		//Save data to struct.
		colors.blue_low[0] = v[0];
		colors.blue_low[1] = v[1];
		colors.blue_low[2] = v[2];
		colors.blue_high[0] = v[3];
		colors.blue_high[1] = v[4];
		colors.blue_high[2] = v[5];
		colors.blue_ideal[0] = v[6];
		colors.blue_ideal[1] = v[7];
		colors.blue_ideal[2] = v[8];

		colors.pink_low[0] = v[9];
		colors.pink_low[1] = v[10];
		colors.pink_low[2] = v[11];
		colors.pink_high[0] = v[12];
		colors.pink_high[1] = v[13];
		colors.pink_high[2] = v[14];
		colors.pink_ideal[0] = v[15];
		colors.pink_ideal[1] = v[16];
		colors.pink_ideal[2] = v[17];

		colors.yellow_low[0] = v[18];
		colors.yellow_low[1] = v[19];
		colors.yellow_low[2] = v[20];
		colors.yellow_high[0] = v[21];
		colors.yellow_high[1] = v[22];
		colors.yellow_high[2] = v[23];
		colors.yellow_ideal[0] = v[24];
		colors.yellow_ideal[1] = v[25];
		colors.yellow_ideal[2] = v[26];



	}

	return colors;

}

void Image::getFileImages()
{
	recursive_directory_iterator end_itr;

	for (recursive_directory_iterator itr(this->folderPath); itr != end_itr; ++itr)
	{
		//Path strings
		string currentFilePath = itr->path().string();
		string currentFileName = itr->path().filename().string();

		//If directory, make corresponding output folder and continue
		if (is_directory(itr->path()))
		{
			size_t index = 0;
			string outputDirectoryPath = currentFilePath;
			index = outputDirectoryPath.find("Input", index);
			outputDirectoryPath.replace(index,5,"Output"); //Replace "Input" with "Output

			create_directory(outputDirectoryPath);

			continue;
		}

		/*----- CALIBRATE -----*/
		color_data imgColors = this->getFileColors(itr->path()); //Read colors from text file in folder. Inefficient, but easy.

		//If not a jpeg, skip to next file
		if(currentFileName.find(".jpg") == string::npos)
			continue;

		cout << "Reading " << currentFileName << endl;


		this->filePaths.push(currentFilePath);
		this->colors.push(imgColors);

	}

	cout << endl << endl;

	return;
}

void Image::saveImage()
{

	if(this->readCamera)
	{
		path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Output_Images/Camera_Images/img_") + to_string(this->numImages) + ".jpg");
		this->imagePath = p.string();
	}
	else
	{
		size_t index = 0;
		index = this->imagePath.find("Input", index);
		this->imagePath.replace(index,5,"Output"); //Replace "Input" with "Output
	}

	//cout << savePath << endl;
	imwrite(this->imagePath,this->cameraImgBGR);
}

void Image::drawImageContours()
{

	vector<vector<Point> > finalContours = this->finalContours;

	vector<vector<Point> > contours_poly( finalContours.size() );
	vector<Rect> boundRect( finalContours.size() );
	vector<Point2f>center( finalContours.size() );
	vector<float>radius( finalContours.size() );

	for(unsigned int i = 0; i < finalContours.size(); i++ )
	 {
		if(finalContours[i].size() > 0){
			boundRect[i] = boundingRect( Mat(finalContours[i]) );
		}
	 }

	//Draw contours on image.

	this->cameraImgBGRContours = this->cameraImgBGR; //Copy. Possibly remove later for speed

	const Scalar color[3] = {Scalar(50,0,0),Scalar(0,0,255),Scalar(24,130,0)};


	for(unsigned int i = 0; i < finalContours.size(); i++)
	{
		for(unsigned int j = 0; j < finalContours[i].size(); j++)
		{
			finalContours[i][j].x = finalContours[i][j].x / scale;
			finalContours[i][j].y = finalContours[i][j].y / scale;
		}
	}


	for(unsigned int i = 0; i< finalContours.size(); i++ )
	{
		if(finalContours[i].size() > 0){
			drawContours( this->cameraImgBGRContours, finalContours, i, color[i], 20, 8);
//			rectangle( this->cameraImgBGRContours, boundRect[i].tl(), boundRect[i].br(), color[i], 10, 8, 0 );
		}
		else
		{
			cout << "No valid tarp" << endl;
		}
	}

	//Display window containing thresholded tarp
//	namedWindow("Final Image",WINDOW_NORMAL);
//	resizeWindow("Final Image",600,600);
//	imshow("Final Image", cameraImgBGRSmall);
//	waitKey(0); //Wait for any key press before closing window

	return;
}


/*
 * Image.cpp
 *
 *  Created on: Feb 9, 2018
 *      Author: jwapman
 */

//System Libraries
#include <string>
#include <queue>

//Source Files
#include "Image.h"
#include "colors.h"

//OpenCV
#include <opencv2/opencv.hpp>

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

//Destructor. Unused.
Image::~Image()
{

}

//Get image. Automatically picks source
bool Image::getImage()
{

	numImages++;

	if(readCamera) //Read from camera
	{
		//Read image from camera
		this->cam >> this->cameraImgBGR;

		color_data c; //Create default color variable. Will need to implement pre-flight calibration
		this->colors.push(c);

		if(this->cameraImgBGR.empty())
		{
			cout << "No camera image available" << endl;
			return false;
		}
		else
		{
			return true;
		}
	}
	else //Read from filesystem
	{
		//Import image. imread imports in BGR format.

		if(filePaths.empty()) //Check to see if there are any more images to import
			return false;

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

//Get color calibration data from the .txt file in the folder with each image
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

//Loop through the parent folder in the filesystem to add all image paths to queue
//Makes directories
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
		color_data imgColors = this->getFileColors(itr->path()); //Read colors from text file in folder. Inefficient, but easy and only used during testing from filesystem.

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

	if(this->readCamera) //Save numerically
	{
		path p((getenv("HOME")) + string("/Eclipse_Workspace/Target_Detection/Output_Images/Camera_Images/img_") + to_string(this->numImages) + ".jpg");
		this->imagePath = p.string();
	}
	else //Save with original filename in output folder
	{
		size_t index = 0;
		index = this->imagePath.find("Input", index); //TODO: Separate imagePath, saveImagePath
		this->imagePath.replace(index,5,"Output"); //Replace "Input" with "Output
	}

	imwrite(this->imagePath,this->cameraImgBGR);
}

//Draws contours on top of cameraImgBGR.
void Image::drawImageContours()
{

	vector<vector<Point> > contours = this->finalContours;

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

	this->cameraImgBGRContours = this->cameraImgBGR; //Copy. Possibly remove later for speed

	const Scalar color[3] = {Scalar(50,0,0),Scalar(0,0,255),Scalar(24,130,0)};


	for(unsigned int i = 0; i < contours.size(); i++)
	{
		for(unsigned int j = 0; j < contours[i].size(); j++)
		{
			contours[i][j].x = contours[i][j].x / scale;
			contours[i][j].y = contours[i][j].y / scale;
		}
	}


	for(unsigned int i = 0; i< finalContours.size(); i++ )
	{
		if(contours[i].size() > 0){
			drawContours( this->cameraImgBGR, contours, i, color[i], 20, 8);
//			rectangle( this->cameraImgBGRContours, boundRect[i].tl(), boundRect[i].br(), color[i], 10, 8, 0 );
		}
	}

	//Display window containing thresholded tarp
//	namedWindow("Final Image",WINDOW_NORMAL);
//	resizeWindow("Final Image",600,600);
//	imshow("Final Image", cameraImgBGRSmall);
//	waitKey(0); //Wait for any key press before closing window

	return;
}

int Image::getNumImages()
{
	return this->numImages;
}

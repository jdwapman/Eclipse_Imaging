/*
 * ImgSource.cpp
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

//System Libraries
#include <string>
#include <queue>
#include <vector>

//Source Files
#include "ImgSource.h"
#include "colors.h"
#include "Image.h"
#include "timing.h"

//OpenCV
#include <opencv2/opencv.hpp>

//Boost
#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;
using namespace boost::filesystem;

//Camera source constructor
ImgSource::ImgSource(VideoCapture cam)
{
	this->readCamera = true;

	this->cam = cam; //OpenCV camera object

	this->readVideo = true;

}

//Filesystem source constructor
ImgSource::ImgSource(path folderPath)
{
	this->readCamera = false;

	this->folderPath = folderPath;

	this->readVideo = false;

	//Add all images and colors to queue
	this->getFileImages();
}

ImgSource::ImgSource(VideoCapture video, path videoPath)
{
	this->readCamera = false;

	this->readVideo = true;

	this->videoPath = videoPath;

	this->cam = video;

	color_data videoColors = this->getFileColors(videoPath);
	this->colors.push(videoColors);
}

//Destructor. Unused.
ImgSource::~ImgSource()
{

}

//Get image. Automatically picks source
Image ImgSource::getImage()
{

	Image img;

	if(readCamera) //Read from camera
	{
		Mat capture;

		//Read image from camera
		this->cam >> capture;

		color_data c; //Create default color variable. Will need to implement pre-flight calibration

		if(capture.empty())
		{
			cout << "No camera image available" << endl;
			img.valid = false;
		}
		else
		{
			img.img = capture;
			img.imgColors = c;
			img.imgPath = "";
			img.valid = true;
		}
	}
	else //Read from filesystem
	{


		if(readVideo)
		{
			Mat capture;

			//Read image from camera
			this->cam >> capture;


			if(capture.empty())
			{
				cout << "No video available" << endl;
				img.valid = false;
			}
			else
			{
				img.img = capture;
				img.imgColors = this->colors.front();
				img.imgPath = "";
				img.valid = true;
			}
		}
		else //Read series of images
		{
			//Import image. imread imports in BGR format.

			if(filePaths.empty()) //Check to see if there are any more images to import
			{
				img.valid = false;
				return img;
			}

			Mat filesystemImage;
			filesystemImage = imread(this->filePaths.front(), CV_LOAD_IMAGE_COLOR); //Read
			cout << this->filePaths.front() << endl;


			if(filesystemImage.empty())
			{
				cout << "No image file available" << endl;
				img.valid = false;
			}
			else
			{
				img.img = filesystemImage;
				img.imgColors = this->colors.front();
				img.imgPath = this->filePaths.front();
				img.valid = true;

				this->filePaths.pop(); //Remove path from queue
				this->colors.pop();
			}
		}
	}

	return img;
}

//Get color calibration data from the .txt file in the folder with each image
color_data ImgSource::getFileColors(path currentFilePath)
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
		std::ifstream f;
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
void ImgSource::getFileImages()
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

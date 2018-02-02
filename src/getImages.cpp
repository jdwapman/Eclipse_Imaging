/*
 * getImages.cpp
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#include <queue>

#include <opencv2/opencv.hpp> //OpenCV library

//Boost
#include "boost/filesystem.hpp"

#include "colors.h"

//Namespaces
using namespace std;
using namespace cv;
using namespace boost::filesystem;

color_data getColors(path currentFilePath)
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

void getImages(path p, queue<string>& filePaths, queue<color_data>& colors)
{
	recursive_directory_iterator end_itr;

	for (recursive_directory_iterator itr(p); itr != end_itr; ++itr)
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
		color_data imgColors = getColors(itr->path()); //Read colors from text file in folder. Inefficient, but easy.

		/*----- READ & CHECK -----*/

		//If not a jpeg, skip to next file
		if(currentFileName.find(".jpg") == string::npos)
			continue;

		cout << "Reading " << currentFileName << endl;

		/*----- ADD DATA TO QUEUES -----*/
		Mat image = imread(currentFilePath, CV_LOAD_IMAGE_COLOR); //Import image. imread imports in BGR format.

		//Make sure image actually exists
		if(image.empty() == false)
		{
			filePaths.push(currentFilePath);
			colors.push(imgColors);
		}
	}

	cout << endl << endl;

	return;
}


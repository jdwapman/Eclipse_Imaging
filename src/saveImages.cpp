/*
 * saveImages.cpp
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

//OpenCV
#include <opencv2/opencv.hpp> //OpenCV library
#include <string>

using namespace std;
using namespace cv;

Mat drawContours(Mat& image, vector<vector<Point> > finalContours)
{
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

	Mat cameraImgBGRSmall;
	double scale = (1.0/4.0);
	resize(image,cameraImgBGRSmall,Size(),scale,scale,INTER_LINEAR);

	//Draw contours on image.
	const Scalar color[3] = {Scalar(50,0,0),Scalar(0,0,255),Scalar(24,130,0)};
	for(unsigned int i = 0; i< finalContours.size(); i++ )
	{
		if(finalContours[i].size() > 0){
			drawContours( cameraImgBGRSmall, finalContours, i, color[i], 3, 8);
//			rectangle( cameraImgBGRSmall, boundRect[i].tl(), boundRect[i].br(), color[i], 10, 8, 0 );
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

	//NOTE: Failing to close the display window before running a new iteration of the code
	//can result in GPU memory errors

	return cameraImgBGRSmall;
}

//Save an image to the filesystem
void saveImage(const Mat& img, const string savePath)
{
	//cout << savePath << endl;
	imwrite(savePath,img);

}



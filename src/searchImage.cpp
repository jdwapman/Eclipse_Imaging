/*
 * searchImage.cpp
 *
 *  Created on: Feb 10, 2018
 *      Author: jwapman
 */

#include <algorithm>  //For sorting
#include <chrono>
#include <future>
#include <iostream>            //Input/output library
#include <opencv2/opencv.hpp>  //OpenCV library
#include <string>
#include <vector>
#include "Tarp.h"
#include "colors.h"
#include "timing.h"

#include "Image.h"
#include "boost/filesystem.hpp"

using namespace std;
using namespace cv;

vector<Rect2d> searchImage(Image imgHSV, double scale)
{
  // Split Mat
  Mat matHSV = imgHSV.img;
  vector<Mat> splitMatHSV(3);

  split(imgHSV.img, splitMatHSV);

  // Find contours

  color_data colors = imgHSV.imgColors;
  Tarp blue("Blue", colors.blue_ideal, colors.blue_low, colors.blue_high);
  Tarp pink("Pink", colors.pink_ideal, colors.pink_low, colors.pink_high);
  Tarp yellow("Yellow", colors.yellow_ideal, colors.yellow_low,
              colors.yellow_high);

  /*----- PER-TARP OPERATIONS -----*/

  vector<vector<Point> > bestContours(3);

  // Threading option
  thread findBlue(&Tarp::findBestTarp, &blue, ref(matHSV), ref(splitMatHSV),
                  ref(bestContours[0]));
  thread findPink(&Tarp::findBestTarp, &pink, ref(matHSV), ref(splitMatHSV),
                  ref(bestContours[1]));
  thread findYellow(&Tarp::findBestTarp, &yellow, ref(matHSV), ref(splitMatHSV),
                    ref(bestContours[2]));
  findBlue.join();
  findPink.join();
  findYellow.join();

  // Sequential option

  //	blue.findBestTarp(matHSV, splitMatHSV, bestContours[0]);
  //	pink.findBestTarp(matHSV, splitMatHSV, bestContours[1]);
  //	yellow.findBestTarp(matHSV, splitMatHSV, bestContours[2]);

  for (unsigned int i = 0; i < bestContours.size(); i++)
  {
    for (unsigned int j = 0; j < bestContours[i].size(); j++)
    {
      bestContours[i][j].x = bestContours[i][j].x / scale;
      bestContours[i][j].y = bestContours[i][j].y / scale;
    }
  }

  vector<Rect2d> bboxes(3);
  for (unsigned int i = 0; i < bestContours.size(); i++)
  {
    if (bestContours[i].size() > 0)
    {
      bboxes[i] = boundingRect(Mat(bestContours[i]));
    }
  }

  return bboxes;
}

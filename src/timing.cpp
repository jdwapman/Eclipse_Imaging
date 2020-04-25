/*
 * timing.cpp
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#include <opencv2/opencv.hpp>  //OpenCV library

using namespace std;
using namespace cv;

// Output elapsed time since last printTime() operation. Useful for determining
// runtime of given step.
void printTime(String operation, TickMeter& tm)
{
  tm.stop();
  cout << operation << ": " << tm.getTimeMilli() << " ms" << endl;
  tm.reset();
  tm.start();
  return;
}

/*
 * timing.cpp
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#include <opencv2/opencv.hpp> //OpenCV library
#include <mutex>
#include "coutMutex.h"

using namespace std;
using namespace cv;

//Output elapsed time since last printTime() operation. Useful for determining runtime of given step.
void printTime(String operation, TickMeter& tm)
{
	tm.stop();

	//Lock & unlock cout for multithreading
	coutMutex.lock();
	cout << operation << ": "  << tm.getTimeMilli() << " ms" << endl;
	coutMutex.unlock();

	tm.reset();
	tm.start();
	return;
}

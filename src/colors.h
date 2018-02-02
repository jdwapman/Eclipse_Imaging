/*
 * colors.h
 *
 *  Created on: Feb 1, 2018
 *      Author: jwapman
 */

#ifndef SRC_COLORS_H_
#define SRC_COLORS_H_

//Structure to store tarp color data
struct color_data{
	int blue_low[3] = {200,45,40};
	int blue_high[3] = {240,100,100};
	int blue_ideal[3] = {0,0,0};

	int pink_low[3] = {290,20,50};
	int pink_high[3] = {350,60,100};
	int pink_ideal[3] = {0,0,0};

	int yellow_low[3] = {45,20,40};
	int yellow_high[3] = {60,100,100};
	int yellow_ideal[3] = {0,0,0};
};



#endif /* SRC_COLORS_H_ */

#include <math.h>
#include "VRMath.h"
/*
Return radians of analog stick
Mult by 180/PI to get angles
*/
struct Point {
	double x;
	double y;
};
double VRtoPolar(double x, double y) {
	double radians = atan2(y, x);
	return radians;
}
/*
Transform angle to x and y coordinates
*/
Point VRtoRect(double angle) {
	Point p;
	angle *= 3.14 / 180;
	p.x = cos(angle);
	p.y = sin(angle);
	return p;
}
double VRnewAngle(double controllerAngle, double cameraAngle) {
	double offset = 90 - controllerAngle;
	return cameraAngle - offset;
}
 
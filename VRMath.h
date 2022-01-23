#pragma once
/*
Return radians of analog stick
Mult by 180/PI to get angles
*/
double VRtoPolar(double x, double y);
/*
Transform angle to x and y coordinates
*/
struct Point;
Point VRtoRect(double angle);
double VRnewAngle(double controllerAngle, double cameraAngle);

 

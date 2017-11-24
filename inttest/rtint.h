#pragma once

#define COLOR_FINDER_API __declspec(dllexport) 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"
#include <stdio.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/features2d.hpp"

using namespace cv;
using namespace std;

struct ObjectData {
	ObjectData(int x, int y, int type, int color) : X(x), Y(y), Type(type), Color(color) {}
	int X, Y, Type, Color;
};

extern "C" {
	int * loc = new int[2];
	cv::VideoCapture capture;
	COLOR_FINDER_API int init(int& outCameraWidth, int& outCameraHeight);
	COLOR_FINDER_API int stopcap();
	COLOR_FINDER_API void cap(ObjectData* ourMarkers, int maxOutMarkersCount, int& outDetectedMarkersCount);
}

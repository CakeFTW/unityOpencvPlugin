#pragma once

#define COLOR_FINDER_API __declspec(dllexport) 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"

#include <stdio.h>

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"


#include "opencv2/features2d.hpp";

using namespace cv;

extern "C" {
	int hello = 5;
	int * loc = new int[2];
	cv::VideoCapture capture;
	COLOR_FINDER_API int returnint();
	COLOR_FINDER_API int * cap();
	COLOR_FINDER_API int stopcap();

}
#pragma once

#define COLOR_FINDER_API __declspec(dllexport) 
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

struct ObjectData {
	ObjectData(int x, int y, int type, int rotationX, int rotationY) : X(x), Y(y), Type(type), RotationX(rotationX), RotationY(rotationY) {}
	int X, Y, Type, RotationX, RotationY;
};

struct cVector {
	int x;
	int y;
	cVector(int _x, int _y) {
		x = _x;
		y = _y;
	}
	cVector() {
		x = 0;
		y = 0;
	}
};

struct glyphObj {
	vector<cVector> list;
	int nrOfPixels;
	cVector bBoxStart;
	cVector bBowEnd;
	cVector center;
	cVector rotation;
	int nr;
	bool returnable = false;
};

extern "C" {
	int * loc = new int[2];
	int divLut[766][256];
	int theLut[256][256];
	cv::VideoCapture capture;
	COLOR_FINDER_API int init(int& outCameraWidth, int& outCameraHeight);
	COLOR_FINDER_API int stopcap();
	COLOR_FINDER_API void cap(ObjectData* ourMarkers, int maxOutMarkersCount, int& outDetectedMarkersCount);
	COLOR_FINDER_API void findBorder();
}

#include "rtint.h"

#include <iostream>

using namespace std;

extern "C" {
	


	int returnint() {
	
		capture.open(0);
		return hello;
		
	}


	int * cap() {
		
		Mat cameraFrame;
		capture.read(cameraFrame);

		Mat imageThresholded;
		inRange(cameraFrame, Scalar(0,0, 150), Scalar(94, 36, 255), imageThresholded);
		uchar * data = imageThresholded.data;


		imshow("cam", imageThresholded);

		
		return 0;
	}


}

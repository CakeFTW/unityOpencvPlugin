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

		Mat bgr[3];
		split(cameraFrame, bgr);
		bgr[2] = bgr[2] - ((bgr[0] + bgr[1]) * 0.63);




		//Mat imageThresholded;
		//inRange(cameraFrame, Scalar(0,0, 150), Scalar(94, 36, 255), imageThresholded);



		imshow("cam", bgr[2]);

		
		return 0;
	}

	 int stopcap()
	{
		 capture.~VideoCapture();
		 return 0;
	}


}

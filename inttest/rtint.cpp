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
		bgr[2] = bgr[2] - ((bgr[0]*0.55) + (bgr[1])*0.55);

	
		
		
		uchar highestVal = 0;
		uchar lowestVal = 255;
		uchar val;
		
		for (int x = 0; x < bgr[2].cols; x++) {
			for (int y = 0; y < bgr[2].rows; y++) {
				
				val = bgr[2].at<uchar>(Point(x, y));
				
				if (val < lowestVal) { lowestVal = val; }
				if (val > highestVal) { highestVal = val; }
				

			}
		}

		
		for (int x = 0; x <  bgr[2].cols; x++) {
			for (int y = 0; y <  bgr[2].rows; y++) {
				val = (bgr[2].at<uchar>(Point(x, y)) - lowestVal) * (((float)255) / highestVal);
				bgr[2].at<uchar>(Point(x, y)) = val;
			}

		}
		
		imshow("pre", bgr[2]);
		medianBlur(bgr[2], bgr[2], 5);
		threshold(bgr[2], bgr[2], 175, 255, THRESH_BINARY);
		imshow("the", bgr[2]);
		medianBlur(bgr[2], bgr[2], 5);
		medianBlur(bgr[2], bgr[2], 5);


		
		imshow("blr", bgr[2]);

		

		
		return 0;
	}

	 int stopcap()
	{
		 capture.~VideoCapture();
		 return 0;
	}


}

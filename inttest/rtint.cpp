#include "rtint.h"

extern "C" {

	bool showAllWindows = true;

	int returnint() {
	
		capture.open(0);
		return hello;
		
	}

	int * cap() {
		
		Mat cameraFrame;
		capture.read(cameraFrame);

		medianBlur(cameraFrame, cameraFrame, 5);

		Mat bgr[3];
		split(cameraFrame, bgr);
		bgr[2] = bgr[2] - ((bgr[0]*0.55) + (bgr[1])*0.55);

		uchar highestVal = 0;
		uchar lowestVal = 255;
		uchar val;
		if(showAllWindows)
			imshow("pre", bgr[2]);

		for (int x = 0; x < bgr[2].cols; x++) {
			for (int y = 0; y < bgr[2].rows; y++) {
				
				val = bgr[2].at<uchar>(Point(x, y));
				
				if (val < lowestVal) { lowestVal = val; }
				if (val > highestVal) { highestVal = val; }
				

			}
		}

		
		for (int x = 0; x <  bgr[2].cols; x++) {
			for (int y = 0; y <  bgr[2].rows; y++) {
				val = (bgr[2].at<uchar>(Point(x, y)) - lowestVal) * (((float)255 )/ highestVal);
				bgr[2].at<uchar>(Point(x, y)) = val;
			}

		}
		
		if (showAllWindows)
			imshow("histoed", bgr[2]);

		medianBlur(bgr[2], bgr[2], 5);
		threshold(bgr[2], bgr[2], 175, 255, THRESH_BINARY);
		if (showAllWindows)
			imshow("the", bgr[2]);

		medianBlur(bgr[2], bgr[2], 5);
		medianBlur(bgr[2], bgr[2], 5);


		if (showAllWindows)
			imshow("blr", bgr[2]);
		
		SimpleBlobDetector::Params params;

		params.filterByColor = 255;

		// Storage for blobs
		vector<KeyPoint> keypoints;

		// Set up detector with params
		Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

		// Detect blobs
		detector->detect(bgr[2], keypoints);


		// Draw detected blobs as red circles.
		// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
		// the size of the circle corresponds to the size of blob

		Mat im_with_keypoints;
		drawKeypoints(bgr[2], keypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

		// Show blobs
		imshow("keypoints", im_with_keypoints);
		
		return 0;
	}

	 int stopcap()
	{
		 capture.~VideoCapture();
		 return 0;
	}


}

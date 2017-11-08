#include "rtint.h"

extern "C" {

	vector<ObjectData> objectData;

	// Storage for blobs
	vector<KeyPoint> keypoints;

	bool showAllWindows = false;

	int init(int& outCameraWidth, int& outCameraHeight) {
	
		capture.open(0);
		if (!capture.isOpened())
			return -2;

		outCameraWidth = capture.get(CAP_PROP_FRAME_WIDTH);
		outCameraHeight = capture.get(CAP_PROP_FRAME_HEIGHT);

		return 0;
		
	}

	void cap(ObjectData* outMarkers, int maxOutMarkersCount, int& outDetectedMarkersCount) {
		
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
			imshow("the_blr", bgr[2]);
		
		SimpleBlobDetector::Params params;

		params.filterByColor = 255;

		// Set up detector with params
		Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

		// Detect blobs
		detector->detect(bgr[2], keypoints);

		// Collects the data into Objects readable by the Unity Script
		for (std::vector<KeyPoint>::size_type i = 0; i != keypoints.size(); i++) {
			outMarkers[i] = ObjectData(keypoints[i].pt.x, keypoints[i].pt.y, 1, 1);
			outDetectedMarkersCount++;
			if (outDetectedMarkersCount == maxOutMarkersCount)
				break;
		}

		// Draw detected blobs as red circles.
		// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
		// the size of the circle corresponds to the size of blob
		Mat im_with_keypoints;
		drawKeypoints(bgr[2], keypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

		// Show blobs
		imshow("keypoints", im_with_keypoints);
		
	}

	 int stopcap()
	{
		 capture.~VideoCapture();
		 return 0;
	}
}

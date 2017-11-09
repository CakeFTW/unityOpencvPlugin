#include "rtint.h"

extern "C" {

	vector<ObjectData> objectData;

	// Storage for blobs
	vector<KeyPoint> keypoints[6];

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
		Mat colors[6];


		split(cameraFrame, bgr);
		float sf = 0.55; // the spilt factor(how powerful the filtering of the colors are)
		colors[0] = bgr[0] - ((bgr[1] * sf) + (bgr[2])*sf); //blue
		colors[1] = bgr[1] - ((bgr[0] * sf) + (bgr[2])*sf); //green
		colors[2] = bgr[2] - ((bgr[0] * sf) + (bgr[1])*sf); //red
		colors[3] = ((bgr[0] + bgr[1])/2) - (bgr[2]*2*sf); //cyan
		colors[4] = ((bgr[0] + bgr[2])/2) - (bgr[1]*2*sf); //magenta
		colors[5] = ((bgr[2] + bgr[1])/2) - (bgr[0]*2*sf); //yellow

		Mat thresholder = bgr[0] * 0;


		SimpleBlobDetector::Params params;

		params.filterByColor = 255;

		// Set up detector with params
		Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);


		for (int i = 0; i <6; i++) {
			uchar highestVal = 0;
			uchar lowestVal = 255;
			uchar val;



			if (showAllWindows)
				imshow("pre", colors[i]);

			for (int x = 0; x < colors[i].cols; x++) {
				for (int y = 0; y < colors[i].rows; y++) {

					val = colors[i].at<uchar>(Point(x, y));

					if (val < lowestVal) { lowestVal = val; }
					if (val > highestVal) { highestVal = val; }


				}
			}


			for (int x = 0; x < colors[i].cols; x++) {
				for (int y = 0; y < colors[i].rows; y++) {
					val = (colors[i].at<uchar>(Point(x, y)) - lowestVal) * (((float)255) / highestVal);
					colors[i].at<uchar>(Point(x, y)) = val;
				}

			}

			if (showAllWindows)
				imshow("histoed", colors[i]);

			medianBlur(colors[i], colors[i], 5);
			threshold(colors[i], colors[i], 175, 255, THRESH_BINARY);

			if (showAllWindows)
				imshow("the_blr", thresholder);

			thresholder += colors[i];




			// Detect blobs
			detector->detect(colors[i], keypoints[i]);
			
			// Collects the data into Objects readable by the Unity Script
			for (std::vector<KeyPoint>::size_type j = 0; j != keypoints[i].size(); j++) {
				outMarkers[j] = ObjectData(keypoints[i][j].pt.x, keypoints[i][j].pt.y, i, 1);
				outDetectedMarkersCount++;
				if (outDetectedMarkersCount == maxOutMarkersCount)
					break;
			}
			
		}



		// Draw detected blobs as red circles.
		// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
		// the size of the circle corresponds to the size of blob

		vector<KeyPoint> totalKeypoints;

		for (int i = 0; i < 6; i++) {
			totalKeypoints.insert(totalKeypoints.end(), keypoints[i].begin(), keypoints[i].end());
		}


		Mat im_with_keypoints;
		drawKeypoints(cameraFrame, totalKeypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

		// Show blobs
		/*imshow("cyan", colors[3]);
		imshow("magenta", colors[4]);
		imshow("yellow", colors[5]);*/
		imshow("KEYPOINTS", im_with_keypoints);

		
	}

	 int stopcap()
	{
		 capture.~VideoCapture();
		 return 0;
	}
}

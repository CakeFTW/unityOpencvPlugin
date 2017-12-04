#include "rtint.h"

extern "C" {

	// parameters for blob detection


	const int GRIDSIZE = 1;

	int r = 180;
	int g = 40;

	bool timeKeeping = true;
	const float discrimHW = 0.2;
	const int rgConvThreshold = 150;


	void lookUpBgr2rg(Mat &in, Mat &out) {
		//convert to normalized rgb space

		//start by creating lookup table
		int divLUT[768][256]; //division lookuptavle;
		for (int i = rgConvThreshold; i < 768; i++) {
			for (int j = 0; j < 256; j++) {
				divLUT[i][j] = (j * 255) / i;
			}
		}
		//then convert using LUT
		int nRows = in.rows;
		int nCols = in.cols * 3;
		int sum = 0;
		uchar * p;
		uchar * cp;

		for (int i = 0; i < nRows; i++) {
			p = in.ptr<uchar>(i);
			cp = out.ptr<uchar>(i);
			for (int j = 0; j < nCols; j += 3) {
				sum = p[j] + p[j + 1] + p[j + 2];
				if (sum < rgConvThreshold) {
					cp[j] = 0;
					cp[j + 1] = 0;
					cp[j + 2] = 0;
					continue;
				}
				cp[j] = divLUT[sum][p[j]];
				cp[j + 1] = divLUT[sum][p[j + 1]];
				cp[j + 2] = divLUT[sum][p[j + 2]];
			}
		}
	}

	void thresholdSpeedy(Mat &in, Mat &out) {

		uchar * p;
		uchar * cp;
		int * ip;
		int nRows = in.rows;
		int nCols = in.cols;

		int lookup[255][255];
		int minPossibleValue = 0;


		for (int i = minPossibleValue; i < 255; i++) {
			ip = lookup[i];
			for (int j = minPossibleValue; j < 255; j++) {
				if (((i - g)*(i - g) + (j - r)*(j - r)) < 3000) {
					*(ip + j) = 255;
				}
				else {
					*(ip + j) = 0;
				}
			}
		}
		int color = 0;
		for (int i = 0; i < nRows; i++) {
			p = in.ptr<uchar>(i);
			cp = out.ptr<uchar>(i);
			for (int j = 0; j < nCols; j++) {
				color = j * 3;
				cp[j] = lookup[p[color + 1]][p[color + 2]];
			}
		}
	}

	void dropFire(uchar * pixel, glyphObj &store, int &width, int y, int x, cVector &from) {
		*pixel = store.nr;
		from.x = x;
		from.y = y;
		store.list.push_back(from);


		if (*(pixel + GRIDSIZE) == 255) {
			dropFire(pixel + GRIDSIZE, store, width, y, x + GRIDSIZE, from);
		}
		if (*(pixel + width) == 255) {
			dropFire(pixel + width, store, width, y + GRIDSIZE, x, from);
		}

		if (*(pixel - GRIDSIZE) == 255) {
			dropFire(pixel - GRIDSIZE, store, width, y, x - GRIDSIZE, from);
		}

		if (*(pixel - width) == 255) {
			dropFire(pixel - width, store, width, y - GRIDSIZE, x, from);
		}
	}


	void grassFireBlobDetection(Mat &biImg, vector<glyphObj> &blobs) {
		int nRows = biImg.rows;
		int nCols = biImg.cols;
		int rowSize = nCols * GRIDSIZE;
		uchar * p;
		uchar * passer;
		glyphObj currentBlob;
		cVector assigner;
		int col = 245;
		for (int i = GRIDSIZE + 1; i < nRows - GRIDSIZE - 1; i += GRIDSIZE) {
			p = biImg.ptr<uchar>(i);
			for (int j = GRIDSIZE; j < nCols - GRIDSIZE; j += GRIDSIZE) {
				if (p[j] == 255) {
					blobs.push_back(currentBlob);
					blobs.back().nr = col;
					passer = &p[j];
					dropFire(passer, blobs.back(), rowSize, i, j, assigner);
					col -= 10;
					if (col < 20) {
						col = 245;
					}
				}
			}
		}
	}


	void blobAnalysis(vector<glyphObj> &blobs, Mat &drawImg) {

		//printing out objects
		int minSize = 200 / GRIDSIZE;
		int maxSize = 8000 / GRIDSIZE;
		for (auto &i : blobs) {
			//find center
			int size;
			size = i.list.size();
			if (size < minSize || size > maxSize) { continue; }
			long centerX = 0;
			long centerY = 0;
			int largestX = 0;
			int smallestX = 10000;
			int largestY = 0;
			int smallestY = 10000;
			int radiusDist = 0;
			int searchDist = 0;
			for (auto &v : i.list) {
				if (v.x < smallestX) { smallestX = v.x; }
				if (v.x > largestX) { largestX = v.x; }
				if (v.y < smallestY) { smallestY = v.y; }
				if (v.y > largestY) { largestY = v.y; }
				centerX += v.x;
				centerY += v.y;
			}
			float heightWidth = ((largestX - smallestX) / (float)(largestY - smallestY));

			//check discriminate basedd on height width relation
			if (heightWidth > (1 + discrimHW) || heightWidth <(1 - discrimHW)) { continue; }
			centerX = (smallestX + largestX) / 2;
			centerY = (smallestY + largestY) / 2;
			radiusDist = ((float)((float)(largestX - centerX) + (centerX - smallestX) + (largestY - centerY) + (centerY - smallestY))) / 4;
			i.center.x = centerX;
			i.center.y = centerY;
			circle(drawImg, Point(centerX - GRIDSIZE, centerY - GRIDSIZE), 2, Scalar(0, 0, 255), 5);
			searchDist = (float)radiusDist * 0.7;
			searchDist = searchDist * searchDist;
			//find closest pixel
			int dist = 10000;
			vector<cVector> points;
			for (auto &v : i.list) {
				dist = (v.x - i.center.x) * (v.x - i.center.x) + (v.y - i.center.y) * (v.y - i.center.y);
				if (dist < searchDist) {
					points.push_back(v);
				}
			}
			if (points.size() == 0) { continue; }
			float rotX = 0;
			float rotY = 0;

			for (auto &p : points) {
				rotX += p.x - centerX;
				rotY += p.y - centerY;
			}

			rotX /= points.size();
			rotY /= points.size();

			//set vector size to be radius
			float vecDist = sqrt(rotX * rotX + rotY * rotY);
			if (vecDist == 0) {
				continue;
			}
			vecDist = radiusDist / vecDist;

			i.rotation.x = rotX * vecDist;
			i.rotation.y = rotY * vecDist;

			line(drawImg, Point(i.center.x - GRIDSIZE, i.center.y - GRIDSIZE), Point(i.center.x + i.rotation.x - GRIDSIZE, i.center.y + i.rotation.y - GRIDSIZE), Scalar(0, 255, 0), 2);

			//use vectors to find bit pixels.

			i.center.x = centerX + rotX*0.4;
			i.center.y = centerY + rotY*0.4;


			cVector rotCclock;
			rotCclock.x = -rotY*0.35;
			rotCclock.y = rotX*0.35;
			cVector rotClock;
			rotClock.x = rotY*0.35;
			rotClock.y = -rotX*0.35;
			cVector reverse;
			reverse.x = -rotX*0.9;
			reverse.y = -rotY*0.9;


			const int cirSize = 1;

			vector<cVector> searchPoints;
			cVector point(i.center.x + rotCclock.x - GRIDSIZE, i.center.y + rotCclock.y - GRIDSIZE);
			searchPoints.push_back(point);

			point = cVector(i.center.x + rotClock.x - GRIDSIZE, i.center.y + rotClock.y - GRIDSIZE);
			searchPoints.push_back(point);

			point = cVector(i.center.x + rotCclock.x + reverse.x - GRIDSIZE, i.center.y + rotCclock.y + reverse.y - GRIDSIZE);

			searchPoints.push_back(point);

			point = cVector(i.center.x + rotClock.x + reverse.x - GRIDSIZE, i.center.y + rotClock.y + reverse.y - GRIDSIZE);
			searchPoints.push_back(point);

			point = cVector(i.center.x + rotCclock.x * 3 - GRIDSIZE, i.center.y + rotCclock.y * 3 - GRIDSIZE);
			searchPoints.push_back(point);

			point = cVector(i.center.x + rotClock.x * 3 - GRIDSIZE, i.center.y + rotClock.y * 3 - GRIDSIZE);
			searchPoints.push_back(point);

			point = cVector(i.center.x + rotCclock.x * 3 + reverse.x - GRIDSIZE, i.center.y + rotCclock.y * 3 + reverse.y - GRIDSIZE);
			searchPoints.push_back(point);

			point = cVector(i.center.x + rotClock.x * 3 + reverse.x - GRIDSIZE, i.center.y + rotClock.y * 3 + reverse.y - GRIDSIZE);
			searchPoints.push_back(point);

			int bitCounter = 0;
			uchar * colPtr;
			int iterations = 0;
			for (auto &sp : searchPoints) {

				Vec3b intensity = drawImg.at<Vec3b>(sp.y, sp.x);

				if (intensity[0]< 125 && intensity[1] < 125 && intensity[2] < 125) {
					bitCounter += pow(2, iterations);
					circle(drawImg, Point(sp.x, sp.y), cirSize, Scalar(0, 255, 0), 1);
				}
				else {
					circle(drawImg, Point(sp.x, sp.y), cirSize, Scalar(0, 0, 255), 1);
				}
				iterations++;
			}
			circle(drawImg, Point(centerX - GRIDSIZE, centerY - GRIDSIZE), sqrt(searchDist), Scalar(255, 0, 255), 2);


			i.nr = bitCounter;
			putText(drawImg, to_string(bitCounter), Point(centerX, centerY - sqrt(radiusDist) - 5), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0));

		}

	}

	vector<ObjectData> objectData;

	//Material to hold camera frame
	Mat cameraFrame;
	Mat rgbNormalized;
	Mat Threshold;


	bool showAllWindows = true;


	//Assorted variables for border detection
	int thresh = 100;
	int max_thresh = 255;
	int largest_area = 0;
	int largest_contour_index = 0;
	Rect bounding_rect;
	vector<vector<Point>> biggestContour;
	vector<Vec4i> permHiearchy;
	void findBorder();


	int init(int& outCameraWidth, int& outCameraHeight) {
	
		capture.open(0);
		if (!capture.isOpened())
			return -2;

		outCameraWidth = capture.get(CAP_PROP_FRAME_WIDTH);
		outCameraHeight = capture.get(CAP_PROP_FRAME_HEIGHT);

		capture.read(cameraFrame);
		rgbNormalized = Mat(cameraFrame.rows, cameraFrame.cols, CV_8UC3);
		Threshold = Mat(cameraFrame.rows, cameraFrame.cols, CV_8UC1);
		copyMakeBorder(Threshold, Threshold, GRIDSIZE + 1, GRIDSIZE + 1, GRIDSIZE + 1, GRIDSIZE + 1, BORDER_CONSTANT, 0);


		return 0;
		
	}

	void cap(ObjectData* outMarkers, int maxOutMarkersCount, int& outDetectedMarkersCount) {
		

		capture.read(cameraFrame);

		lookUpBgr2rg(cameraFrame, rgbNormalized);

		imshow("rg norm", rgbNormalized);

		thresholdSpeedy(rgbNormalized, Threshold);

		// Storage for blobs
		vector<glyphObj> blobs;
		grassFireBlobDetection(Threshold, blobs);

		imshow("blobs", Threshold);

		blobAnalysis(blobs, cameraFrame);

		imshow("out", cameraFrame);
		float rotation = 0;
		for (auto &blob : blobs) {
			rotation = acos(blob.rotation.x / sqrt((blob.rotation.x * blob.rotation.x) + (blob.rotation.y * blob.rotation.y)));
			outMarkers[outDetectedMarkersCount] = ObjectData(blob.center.x, blob.center.y, blob.nr, rotation*100);
			outDetectedMarkersCount++;
			if (outDetectedMarkersCount == maxOutMarkersCount)
				break;
		}
	}

	 int stopcap()
	{
		 capture.~VideoCapture();
		 return 0;
	}

	 /*
	void findBorder() {
		 Mat tempImg, canny_output;
		 
		 //Making a clone of the camera feed image
		 if (!cameraFrame.empty()) {
			 tempImg = cameraFrame.clone();
			 vector<vector<Point>> contours;

			 vector<Vec4i> hiearchy;
			 //Converting to HSV
			 cvtColor(tempImg, tempImg, CV_BGR2HSV);

			 //Sensitivity of threshold, higher number = bigger area to take in
			 int sensitivity = 20;
			 //Thresholding
			 inRange(tempImg, Scalar(73 - sensitivity, 18, 18), Scalar(73 + sensitivity, 255, 255), canny_output);
			 //Median blur to remove some noise
			 medianBlur(canny_output, canny_output, 7);

			 //Find the contours, and save them in contours vector vector
			 findContours(canny_output, contours, hiearchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

			 //Empty material to store the drawing of the contour
			 Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
			 // iterate through each contour.
			 for (int i = 0; i < contours.size(); i++) {
				 //  Find the area of contour
				 double contour_area = contourArea(contours[i], false);
				 if (contour_area > largest_area) {
					 //Save the new biggest contour
					 largest_area = contour_area;
					 //Emptying the biggest contour container, as a newer, bigger one has been found
					 biggestContour.empty();
					 biggestContour.insert(biggestContour.begin(), contours[i]);

					 // Store the index of largest contour
					 largest_contour_index = 0;
					 // Find the bounding rectangle for biggest contour
					 bounding_rect = boundingRect(biggestContour[0]);
				 }
			 }
			 //Green colour
			 Scalar color = Scalar(0, 255, 0);
			 if (largest_area > 1000) {
				 //Draw the found contours
				 drawContours(drawing, biggestContour, 0, color, CV_FILLED, 8, hiearchy, 0, Point());
				 //Draw the bounding box the found "object"
				 rectangle(drawing, bounding_rect, Scalar(0, 255, 0), 2, 8, 0);
			 }
			 //Show the resulting biggest contour "object"
			 namedWindow("Contours", CV_WINDOW_AUTOSIZE);
			 imshow("Contours", drawing);
		 }
	 }
	 */
}

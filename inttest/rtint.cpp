#include "rtint.h"

extern "C" {

	bool debugMode = true;

	// parameters for blob detection



	const int GRIDSIZE = 1;
	const int BORDER = GRIDSIZE + 1;

	int r = 180;
	int g = 40;

	bool timeKeeping = true;
	const float discrimHW = 0.2;
	const int rgConvThreshold = 125;


	void preLookUpBgr2rg(Mat &in, Mat &out, int (&divLUT)[766][256]) {
		//convert to normalized rgb space
		int nRows = in.rows;
		int nCols = in.cols * 3;
		int sum = 0;
		uchar * p;
		uchar * cp;

		uchar red;
		uchar green;
		uchar blue;
		int * lutptr;

		for (int i = 0; i < nRows; i += GRIDSIZE) {
			p = in.ptr<uchar>(i);
			cp = out.ptr<uchar>(i);

			for (int j = 0; j < nCols; j += 3 * GRIDSIZE) {
				blue = p[j];
				green = p[j + 1];
				red = p[j + 2];
				sum = blue + green + red;
				lutptr = divLUT[sum];
				cp[j] = *(lutptr + blue);
				cp[j + 1] = *(lutptr + green);
				cp[j + 2] = *(lutptr + red);
			}
		}
	}


	void thresholdSpeedy(Mat &in, Mat &out, int (&lookup)[256][256]) {

		uchar * cp;
		int nRows = in.rows;
		int nCols = in.cols;
		uchar * p = in.ptr<uchar>(0);
		int color = -3;

		for (int i = 0; i < nRows-0; i += GRIDSIZE) {
			p = in.ptr<uchar>(i);
			cp = out.ptr<uchar>(i+BORDER);
			color = -3;
			for (int j = 0; j < nCols; j += GRIDSIZE) {
				color += 3;
				cp[j+BORDER] = lookup[p[color + 1]][p[color + 2]];
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

			for (int j = GRIDSIZE + 1; j < nCols - GRIDSIZE - 1; j += GRIDSIZE) {
				if (p[j] == 255) {
					blobs.push_back(currentBlob);
					blobs.back().nr = col;
					passer = &p[j];
					dropFire(passer, blobs.back(), rowSize, i-BORDER, j-BORDER, assigner);
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
		int minSize = 100 / GRIDSIZE;
		int maxSize = 16000 / GRIDSIZE;
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
	
			}
			float heightWidth = ((largestX - smallestX) / (float)(largestY - smallestY));

			
			//check discriminate basedd on height width relation
			if (heightWidth > (1 + discrimHW) || heightWidth <(1 - discrimHW)) { continue; }
			centerX = (smallestX + largestX) / 2;
			centerY = (smallestY + largestY) / 2;

			radiusDist = (largestX - smallestX + largestY - smallestY) / 4;
			i.center.x = centerX;
			i.center.y = centerY;
			
			searchDist = (float)radiusDist*0.70;
			searchDist = searchDist * searchDist;
			//find closest pixel
			int dist = 10000;
			vector<cVector> points;
			for (auto &v : i.list) {
				dist = (v.x - i.center.x) * (v.x - i.center.x) + (v.y - i.center.y) * (v.y - i.center.y);
				if (dist < searchDist) {
					points.push_back(v);
					drawImg.at<uchar>(v.y, v.x*3) = 255;
				}
			}
			if (points.size() == 0) { continue; }
			float rotX = 0;
			float rotY = 0;

			long holderRotX = 0;
			long holderRotY = 0;
			for (auto &p : points) {
				holderRotX += p.x - centerX;
				holderRotY += p.y - centerY;
			}

			rotX = (holderRotX / (float)points.size());
			rotY = (holderRotY / (float)points.size());

			//set vector size to be radius
			float vecDist = sqrt(rotX * rotX + rotY * rotY);
			if (vecDist < 3) {
				continue;
			}
			vecDist = radiusDist/vecDist;

			rotX *= vecDist;
			rotY *= vecDist;
			i.rotation.x = rotX ;
			i.rotation.y = rotY ;

			//use vectors to find bit pixels.

			int startPointX = centerX + rotX*0.25;
			int startPointY = centerY + rotY*0.25;


			//Bigger is more radius
			float cClockX = -rotY*0.185;
			float cClockY = rotX*0.185;
			
			float clockX = rotY*0.185;
			float clockY= -rotX*0.185;

			//Downwards sample
			float reverseX = -rotX*0.55;
			float reverseY = -rotY*0.55;


			const int cirSize = 1;

			vector<cVector> searchPoints;

			cVector point((startPointX + cClockX) , (startPointY + cClockY));
			searchPoints.push_back(point);

			point = cVector((startPointX + clockX), (startPointY + clockY));
			searchPoints.push_back(point);

			point = cVector((startPointX + cClockX + reverseX) , (startPointY + cClockY + reverseY));
			searchPoints.push_back(point);

			point = cVector((startPointX + clockX + reverseX ), (startPointY + clockY + reverseY));
			searchPoints.push_back(point);

			point = cVector((startPointX + cClockX * 3), (startPointY + cClockY * 3 ));
			searchPoints.push_back(point);

			point = cVector((startPointX + clockX * 3), (startPointY + clockY * 3 ));
			searchPoints.push_back(point);

			point = cVector((startPointX + cClockX * 3 + reverseX), (startPointY + cClockY * 3 + reverseY));
			searchPoints.push_back(point);

			point = cVector((startPointX + clockX * 3 + reverseX) , (startPointY + clockY * 3 + reverseY));
			searchPoints.push_back(point);

			int bitCounter = 0;
			int iterations = 0;
			int thresh = 75;
			for (auto &sp : searchPoints) {

				Vec3b intensity = drawImg.at<Vec3b>(sp.y, sp.x);
				
				if (intensity[0]< thresh && intensity[1] < thresh && intensity[2] < thresh) {
					bitCounter += pow(2, iterations);
					circle(drawImg, Point(sp.x, sp.y), cirSize, Scalar(0, 255, 0), 1);
				}
				else {
					circle(drawImg, Point(sp.x, sp.y), cirSize, Scalar(0, 0, 255), 1);
				}
				iterations++;
			}

			line(drawImg, Point(i.center.x, i.center.y), Point(i.center.x + i.rotation.x, i.center.y + i.rotation.y), Scalar(0, 255, 0), 2);
			if (bitCounter > 0) {
				i.returnable = true;
				i.nr = bitCounter;
				putText(drawImg, to_string(bitCounter), Point(centerX, centerY - sqrt(radiusDist) - 5), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0));
				circle(drawImg, Point(centerX , centerY), 1, Scalar(0, 0, 255),1);
				circle(drawImg, Point(centerX, centerY), sqrt(searchDist), Scalar(255, 0, 255), 1);
				circle(drawImg, Point(centerX, centerY), radiusDist, Scalar(255, 0, 255), 1);
			}
			else {
				i.returnable = false;
			}

			//DRAW BOUNDING BOX

			line(drawImg, Point(smallestX, smallestY), Point(largestX , smallestY), Scalar(0, 255, 0), 1);
			line(drawImg, Point(smallestX, smallestY), Point(smallestX, largestY), Scalar(0, 255, 0), 1);
			line(drawImg, Point(smallestX, largestY), Point(largestX,largestY), Scalar(0, 255, 0), 1);
			line(drawImg, Point(largestX, smallestY), Point(largestX, largestY), Scalar(0, 255, 0), 1);

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
	
		capture.open(1);
		if (!capture.isOpened())
			return -2;

		outCameraWidth = capture.get(CAP_PROP_FRAME_WIDTH);
		outCameraHeight = capture.get(CAP_PROP_FRAME_HEIGHT);

		capture.read(cameraFrame);
		rgbNormalized = Mat(cameraFrame.rows, cameraFrame.cols, CV_8UC3);
		Threshold = Mat(cameraFrame.rows, cameraFrame.cols, CV_8UC1);
		copyMakeBorder(Threshold, Threshold, BORDER, BORDER, BORDER, BORDER, BORDER_CONSTANT, 0);
		

		//initialize the lookup tabels
		for (int i = 0; i < 766; i++) {
			for (int j = 0; j < 256; j++) {
				if (i < rgConvThreshold) {
					divLut[i][j] = 0;
				}
				else {
					divLut[i][j] = (j * 255) / i;
				}
			}
		}

		for (int i = 0; i < 256; i++) {
			for (int j = 0; j < 256; j++) {
				if (((i - g)*(i - g) + (j - r)*(j - r)) < 3600) {
					theLut[i][j] = 255;
				}
				else {
					theLut[i][j] = 0;
				}
			}
		}

		return 0;
		
	}

	void cap(ObjectData* outMarkers, int maxOutMarkersCount, int& outDetectedMarkersCount) {
		

		capture.read(cameraFrame);

		preLookUpBgr2rg(cameraFrame, rgbNormalized, divLut);
	

		thresholdSpeedy(rgbNormalized, Threshold, theLut);

		// Storage for blobs
		vector<glyphObj> blobs;
		grassFireBlobDetection(Threshold, blobs);

		if(debugMode)
			imshow("blobs", Threshold);

		blobAnalysis(blobs, cameraFrame);

		if(debugMode)
			imshow("out", cameraFrame);

		float rotation = 0;
		int screenWidth = capture.get(CAP_PROP_FRAME_WIDTH);
		int screenHeight = capture.get(CAP_PROP_FRAME_HEIGHT);
		for (auto &blob : blobs) {
			if (outDetectedMarkersCount == maxOutMarkersCount)
				break;
			if (blob.returnable == false)
				continue;
			outMarkers[outDetectedMarkersCount] = ObjectData(screenWidth  - blob.center.x,screenHeight - blob.center.y, blob.nr, blob.rotation.x, blob.rotation.y);
			outDetectedMarkersCount++;
			
		}
		if (debugMode)
			imshow("rg norm", rgbNormalized);
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

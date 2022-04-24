/*************************************************************************/
/*** Title: Docu Scanner.cpp                                           ***/
/*** Course: Computational Problem Solving CPET-121                    ***/
/*** Developer: Arianit Sylafeta                                       ***/
/*** Date: 04/24/2022                                                  ***/
/*** Description: The program enables users to upload images           ***/
/*** it scans the images, lays them flat, and grayscales them.         ***/
/*** Finally the user can download the document produced               ***/
/*************************************************************************/

//Import all dependent libraries
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <string>
#include <iomanip>

using namespace cv;
using namespace std;

// Initialize global variables
Mat imgOriginal, imgGray, imgBlur, imgCanny, imgThre, imgDil, imgErode, imgWarp, imgCrop;
vector<Point> initialPoints, docPoints;
float w = 420, h = 596;

// Create the PreProcessing function that grayscales the image, detected the edges and dilates the image to increase sharpness.
Mat preProcessing(Mat img)
{
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);
	//erode(imgDil, imgErode, kernel);
	return imgDil;
}

// Create a function that detects the border of the largest square inside the image.
vector<Point> getContours(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//drawContours(img, contours, -1, Scalar(255, 0, 255), 2);
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point> biggest;
	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		//cout << area << endl;

		string objectType;

		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

			if (area > maxArea && conPoly[i].size() == 4) {

				//drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 5);
				biggest = { conPoly[i][0],conPoly[i][1] ,conPoly[i][2] ,conPoly[i][3] };
				maxArea = area;
			}
			//drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 2);
			//rectangle(imgOriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
		}
	}
	return biggest;
}

// Create a function that correctly numbers and reorders the edge numbers of the image, so the image is upright.
vector<Point> reorder(vector<Point> points)
{
	vector<Point> newPoints;
	vector<int>  sumPoints, subPoints;

	for (int i = 0; i < 4; i++)
	{
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}

	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); // 0
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //1
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //2
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //3

	return newPoints;
}

//Warp the image bound within the points from previous function into a flat A4 shape.
Mat getWarp(Mat img, vector<Point> points, float w, float h)
{
	Point2f src[4] = { points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));

	return imgWarp;
}


int main() {

	//Initialize local variables
	string inputPath, fileName, directoryPath, outputPath;

	//Greet user and explain program
	cout << "/*************************************************************************/" << endl;
	cout << "/***                          Document Scanner                         ***/" << endl;
	cout << "/***                          ----------------                         ***/" << endl;
	cout << "/***                                                                   ***/" << endl;
	cout << "/***  This program's purpose is to help users transform their          ***/" << endl;
	cout << "/***  images into pdf formatted documents in grayscale and flat.       ***/" << endl;
	cout << "/***  Users can then download these documents if desired.              ***/" << endl;
	cout << "/***                                                                   ***/" << endl;
	cout << "/***  REQUIREMENTS: Please make sure that your desired document:       ***/" << endl;
	cout << "/***  1. Fills the majority of the image                               ***/" << endl;
	cout << "/***  2. Is upright(The program doesn't rotate the image).             ***/" << endl;
	cout << "/***  3. Appears rectangular on the screen(Not curved!).               ***/" << endl;
	cout << "/***  4. Is in jpg or png format                                       ***/" << endl;
	cout << "/*************************************************************************/" << endl;

	//Require path to image from user and read from it.
	cout << endl << endl << "Please specify the address to your desired image." << endl;
	getline(cin, inputPath);
	imgOriginal = imread(inputPath);

	// Preprpcessing - Step 1 
	imgThre = preProcessing(imgOriginal);

	// Get Contours - Biggest  - Step 2
	initialPoints = getContours(imgThre);
	//drawPoints(initialPoints, Scalar(0, 0, 255));
	docPoints = reorder(initialPoints);
	//drawPoints(docPoints, Scalar(0, 255, 0));

	// Warp - Step 3 
	imgWarp = getWarp(imgOriginal, docPoints, w, h);

	//Crop - Step 4
	int cropVal = 5;
	Rect roi(cropVal, cropVal, w - (2 * cropVal), h - (2 * cropVal));
	imgCrop = imgWarp(roi);

	imshow("Image Before", imgOriginal);
	//imshow("Image Dilation", imgThre);
	//imshow("Image Warp", imgWarp);
	imshow("Image After", imgCrop);
	waitKey(0);

	//Give user option to download image.
	cout << "Do you wanna download the scanned download the scanned document? Y/N?";
	char answer;
	cin >> answer;

	if (answer == 'Y') {
		cout << "Please specify a path for your document" << endl;
		cin >> directoryPath;
		cout << "Please specify a name for your document(with extension)" << endl;
		cin >> fileName;
		outputPath = directoryPath + '/' + fileName;
		imwrite(outputPath, imgCrop);
		cout << "/*************************************************************************/" << endl;
		cout << "/***                  Thank You for using Docu Scanner!                ***/" << endl;
		cout << "/*************************************************************************/" << endl;
	}
	else {
		cout << "/*************************************************************************/" << endl;
		cout << "/***                  Thank You for using Docu Scanner!                ***/" << endl;
		cout << "/*************************************************************************/" << endl;
		EXIT_SUCCESS;
	}
	return 0;
}
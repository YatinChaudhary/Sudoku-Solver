/*
 * Grid_Detection.cpp
 *
 *  Created on: Dec 2, 2016
 *      Author: yatin
 */

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include "tesseract/baseapi.h"
#include "leptonica/allheaders.h"

#include <iostream>
#include <math.h>
#include <sstream>

using namespace std;
using namespace cv;

vector<Point> orderPointsByRows(vector<Point> points);
vector<Point> orderRectCorners(vector<Point> corners);
vector<Point> shiftpoints(vector<Point> points);
void getalldigits(Mat img, Mat x, int max, int clr);
void prep_digit(Mat img, int a, int b);
void drawLine(Vec2f line, Mat &img, Scalar rgb);
unsigned int recognize_digit(Mat& im, tesseract::TessBaseAPI& tess);
void test(Mat img);

int main() {

	tesseract::TessBaseAPI tess;
	if (tess.Init("/usr/local/share/", "eng")) {
	        fprintf(stderr, "Could not initialize tesseract.\n");
	        exit(1);
	}

	//Read the image file
	Mat sudoku = imread("Sudoku_4.jpg",0);

	//Create windows
	namedWindow("Sudoku", CV_WINDOW_AUTOSIZE);
	namedWindow("Gaussian", CV_WINDOW_AUTOSIZE);
	namedWindow("Outer_box", CV_WINDOW_AUTOSIZE);
	namedWindow("contour_box", CV_WINDOW_AUTOSIZE);

	//Show the original image
	imshow("Sudoku", sudoku);
	waitKey(0);

	//Create boxes for outer box of sudoku and blurred image
	Mat sudoku_box = Mat::zeros(sudoku.size(), sudoku.type());
	Mat gaus, med, bil;

	//Applying blurring
	GaussianBlur(sudoku, gaus, Size(11,11),0,0);

	//Checking blurring
	imshow("Gaussian", gaus);
	waitKey(0);

	//Applying thresholding
	adaptiveThreshold(gaus, sudoku_box, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 3);
	medianBlur(sudoku_box, sudoku_box, 3);
	bitwise_not(sudoku_box, sudoku_box);
	Mat kernel = (Mat_<uchar>(3,3) << 0,1,0,1,1,1,0,1,0);
	dilate(sudoku_box, sudoku_box, kernel);
	imshow("Outer_box", sudoku_box);
	waitKey(0);

	//Blob Detection - Filling of all white space

	//int count=0;
	int max=-1;

	Point maxPt;

	for(int y=0;y<sudoku_box.size().height;y++) {
		uchar *row = sudoku_box.ptr(y);

		for(int x=0;x<sudoku_box.size().width;x++) {

			if(row[x]>=128) {
				int area = floodFill(sudoku_box, Point(x,y), CV_RGB(0,0,64));

				if(area>max) {
					maxPt = Point(x,y);
					max = area;
				}
			}
		}
	}

	//Turn biggest blob white
	floodFill(sudoku_box, maxPt, CV_RGB(0,0,255));

	//Turning every other blob to black
	for(int y=0;y<sudoku_box.size().height;y++) {
			uchar *row = sudoku_box.ptr(y);

			for(int x=0;x<sudoku_box.size().width;x++) {

				if(row[x]==64 && x!=maxPt.x && y!=maxPt.y) {
					floodFill(sudoku_box, Point(x,y), CV_RGB(0,0,0));
				}
			}
	}
	//Eroding the image as we have dilated it earlier to restore the original image
	erode(sudoku_box, sudoku_box, kernel);
	imshow("Outer_box",sudoku_box);
	waitKey(0);

	//Creating a bounding box around sudoku by detecting the biggest contour

	vector<vector<Point> > contours;
	vector<Point> bounding_box;
	vector<Vec4i> heirarchy;
	Mat con = Mat::zeros(sudoku.size(), sudoku.type());

	findContours(sudoku_box, contours, heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	drawContours(con, contours, 0, Scalar(128,128,255), 1, CV_AA, noArray(), 2);
	approxPolyDP(contours[0], bounding_box, 10, true);
	bounding_box = orderRectCorners(bounding_box);
	int k = bounding_box.size();
	int maxdistance(0);
	for (int i=0; i<k; i++) {
		double dist(0);
		if (i != k-1) {
			line(con, Point(bounding_box[i].x, bounding_box[i].y), Point(bounding_box[i+1].x, bounding_box[i+1].y), CV_RGB(0, 0, 255), 1, CV_AA);
			dist = cv::norm(bounding_box[i]-bounding_box[i+1]);
		}
		else {
			line(con, Point(bounding_box[k-1].x, bounding_box[k-1].y), Point(bounding_box[0].x, bounding_box[0].y), CV_RGB(0, 0, 255), 1, CV_AA);
			dist = cv::norm(bounding_box[k-1]-bounding_box[0]);
		}
		if (dist > maxdistance)
			maxdistance = static_cast<int>(dist);
		//cout << "\nCoordinates of Point no " << i << " are: " << bounding_box[i].x << " , " << bounding_box[i].y < "\n";
	}
	maxdistance -= (maxdistance%9);
	cout << "Total no of bounding box points: " << k << "\n";
	cout << "Dimension of extracted sudoku: " << maxdistance << "\n";
	imshow("contour_box", con);
	waitKey(0);

	//Extracting Sudoku from Original Image
	Point2f src[k], dst[k];

	dst[0] = Point(0,0);
	dst[1] = Point(0,maxdistance-1);
	dst[2] = Point(maxdistance-1,maxdistance-1);
	dst[3] = Point(maxdistance-1,0);

	src[0] = bounding_box[0];
	src[1] = bounding_box[1];
	src[2] = bounding_box[2];
	src[3] = bounding_box[3];

	//Mat undistorted = Mat(Size(maxdistance, maxdistance), sudoku.type());
	Mat undistorted = Mat(Size(maxdistance, maxdistance), CV_8UC1);

	warpPerspective(sudoku, undistorted, getPerspectiveTransform(src,dst), Size(maxdistance, maxdistance));
	imwrite("/home/yatin/workspace/Sudoku/Extract_Sudoku.tif", undistorted);
	GaussianBlur(undistorted,undistorted,Size(5,5),0,0);
	adaptiveThreshold(undistorted, undistorted, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 3, 2);
	//threshold(undistorted, undistorted, 127, 255, THRESH_BINARY);
	//erode(undistorted,undistorted,kernel);
	//dilate(undistorted,undistorted,kernel);
	bitwise_not(undistorted, undistorted);

	//Showing extracted sudoku
	namedWindow("Undistorted", CV_WINDOW_AUTOSIZE);
	//namedWindow("180x180", CV_WINDOW_AUTOSIZE);
	imshow("Undistorted",undistorted);
	cout << "Undistorted size: " << undistorted.size() << "\n";
	waitKey(0);

	//Retrieve all digits in extracted sudoku
	int clearance = 3;
	int sudo_size = 9;
	int dim = ((maxdistance/sudo_size)-(2*clearance));
	Mat sudoku_mat = Mat(Size(81, pow(dim,2)), sudoku.type());
	getalldigits(undistorted,sudoku_mat,dim,clearance);
	cout << "bhosad\n";

	//Preprocess all digits obtained
	Mat x = imread("/home/yatin/workspace/Sudoku/Digits/Image_2_8.tif", CV_8UC1);
	cout << "Type of extracted digit is: " << x.type() << "and" << x.size() << "\n";
	cout << "Type of sudoku is: " << sudoku.type() << "and" << sudoku.size() << "\n";
	test(x);
	//cout << "Digit: " << x << "\n";

	char name[100];
	for (unsigned int i=1; i<=9; i++) {
		for (unsigned int j=1; j<=9; j++) {
			sprintf(name, "/home/yatin/workspace/Sudoku/Digits/Image_%d_%d.tif", i, j);
			Mat digit = imread(name, CV_8UC1);
			prep_digit(digit, i, j);
		}
	}
	waitKey();

	//Testing Tesseract
	char name2[100];
	unsigned int dig(0);
	for (unsigned int i=1; i<=9; i++) {
		for (unsigned int j=1; j<=9; j++) {
			sprintf(name2, "/home/yatin/workspace/Sudoku/Digits/Image_%d_%d.tif", i, j);
			Mat digit = imread(name2, CV_8UC1);
			dig = recognize_digit(digit, tess);
			cout << "  " << dig << "  ";
		}
		cout << "\n";
	}

	//Wait for any keypress for infinite time and destroys all windows after that
	waitKey();
	destroyAllWindows();

	return 0;
}

/* Function to recognize a digit in a binarized image using Tesseract
 *  Note that we should limit Tesseract to look for digits only, but I didn't manage to do it from the C++ API... :)
 *  That's why we need to handle the 'I' as a '1', er similar...
 */
unsigned int recognize_digit(Mat& im,tesseract::TessBaseAPI& tess)
{
    tess.SetImage((uchar*)im.data, im.size().width, im.size().height, im.channels(), (int)im.step1());
    tess.Recognize(0);
    const char* out = tess.GetUTF8Text();
    if (out)
        if(out[0]=='1' or out[0]=='I' or out[0]=='i' or out[0]=='/' or out[0]=='|' or out[0]=='l' or out[0]=='t')
            return 1;
        else if(out[0]=='2')
            return 2;
        else if(out[0]=='3')
            return 3;
        else if(out[0]=='4')
            return 4;
        else if(out[0]=='5' or out[0]=='S' or out[0]=='s')
            return 5;
        else if(out[0]=='6')
            return 6;
        else if(out[0]=='7')
            return 7;
        else if(out[0]=='8')
            return 8;
        else if(out[0]=='9')
            return 9;
        else
            return 0;
    else
        return 0;
}

vector<Point> orderRectCorners(vector<Point> corners) {
	vector<Point> ordCorners;
	cout << "in\n";
	int i, k, min, max, a(0), b(0);
	int* d;
	k = corners.size();
	d = new (nothrow) int [k];
	if (d == NULL) {
		cout << "error assigning memory. Take measures.\n";
	}
	for (i=0; i<k; i++) {
		d[i] = static_cast<int>(sqrt(pow(corners[i].x,2) + pow(corners[i].y,2)));
	}
	cout << "1\n";
	min = *std::min_element(d,d+k);
	max = *std::max_element(d,d+k);
	cout << "2\n";
	cout << "min and max value: " << min << " and " << max << "\n";
	cout << "Point 1 " << corners[0].x << " and " << corners[0].y << "\n";
	cout << "Point 2 " << corners[1].x << " and " << corners[1].y << "\n";
	cout << "Point 3 " << corners[2].x << " and " << corners[2].y << "\n";
	cout << "Point 4 " << corners[3].x << " and " << corners[3].y << "\n";
 	for (i=0; i<k; i++) {
 		cout << "check " << d[i] << "\n";
		if (d[i]==min)
			min = i;
		else if (d[i]==max)
			max = i;
		else if (a==0)
			a = i;
		else
			b = i;
	}
 	ordCorners.push_back(corners[min]);
	cout << "3\n";
	if (corners[a].x < corners[b].x) {
		ordCorners.push_back(corners[a]);
		ordCorners.push_back(corners[max]);
		ordCorners.push_back(corners[b]);
	}
	else {
		ordCorners.push_back(corners[b]);
		ordCorners.push_back(corners[max]);
		ordCorners.push_back(corners[a]);
	}
	cout << "out\n";
	delete [] d;
	return ordCorners;
}

vector<Point> orderPointsByRows(vector<Point> points) {
	int k = points.size();
	int key, i, j;
	for (i=1; i<k; i++) {
		key = points[i].y;
		for (j=i-1; (j>=0) && (points[j].y > key); j--) {
			points[j+1].y = points[j].y;
		}
		points[j+1].y = key;
	}
    return points;
}

vector<Point> shiftpoints(vector<Point> points) {
	int k(points.size()), i;
	Point last = points[0];
	for (i=1; i<k; i++) {
		points[i-1] = points[i];
	}
	points[k-1] = last;
	return points;
}

void getalldigits(Mat img, Mat x, int max, int clr) {
	cout << "inside getalldigits\n";
	int size = (img.size().height)/9;
	int a = clr;
	int i,j,k,l;
	cout << "cp1\n";
	Mat digits = Mat(Size(81,pow(max,2)), img.type());
	Mat digit = Mat(Size(size-2*a,size-2*a), img.type());
	cout << "cp2\n";
	for (i=0; i<9; i++) {
		for (j=0; j<9; j++) {
			for (k=a;k<size-a;k++) {
				for (l=a;l<size-a;l++) {
					//cout << "cp3\n";
					x.at<uchar>(Point(9*i+j,size*(k-a)+(l-a))) = img.at<uchar>(Point(size*i+k,size*j+l));
					//cout << "cp4\n";
					digit.at<uchar>(Point((k-a),(l-a))) = img.at<uchar>(Point(size*i+k,size*j+l));
					cout << i << " " << j << " " << k << " " << l << "\n";
				}
			}
			ostringstream name;
			name << "/home/yatin/workspace/Sudoku/Digits/Image_" << (j+1) << "_" << (i+1) << ".tif";
			imwrite(name.str(), digit);
		}
	}
	//cout << "cp3\n";
	cout << "Out of getalldigits\n";
	//return digits;
}

void prep_digit(Mat img, int a, int b) {
	cout << "inside prep_digit\n";
	//Mat filt = Mat(img.size(), img.type());
	//Mat lar_con = Mat(img.size(), img.type());
	//Mat show = Mat(Size(180,180), img.type());
	Mat org = Mat(Size(100,100), img.type());
	namedWindow("Showdigit", CV_WINDOW_AUTOSIZE);
	//namedWindow("orgdigit", CV_WINDOW_AUTOSIZE);
	//namedWindow("Showcanny", CV_WINDOW_AUTOSIZE);

	//Floodfilling from sides to remove extra whitespace
	int i,j;
	for (i=0; i<img.size().height; i++) {
		floodFill(img, Point(0,i), CV_RGB(0,0,0));
		floodFill(img, Point(img.size().width-1,i), CV_RGB(0,0,0));
	}

	for (i=0; i<img.size().width; i++) {
		floodFill(img, Point(i,0), CV_RGB(0,0,0));
		floodFill(img, Point(i,img.size().height-1), CV_RGB(0,0,0));
	}

	for (i=0; i<img.size().height; i++) {
		for (j=0; j<img.size().width; j++) {
			if(img.at<uchar>(Point(j,i)) <= 250)
				floodFill(img, Point(j,i), CV_RGB(0,0,0));
		}
	}

	ostringstream name;
	name << "/home/yatin/workspace/Sudoku/Digits/Image_" << a << "_" << b << ".tif";
	resize(img, org, org.size(), 0, 0, CV_INTER_LINEAR);
	Mat kernel = (Mat_<uchar>(3,3) << 0,1,0,1,1,1,0,1,0);
	erode(org, org, kernel);
	dilate(org, org, kernel);
	//imshow("Showdigit",org);
	imwrite(name.str(), org);

	/*
	bilateralFilter(img,filt,3,5,5);
	threshold(filt, filt, 128, 255, THRESH_BINARY);
	resize(filt, show, show.size(), 0, 0, CV_INTER_CUBIC);
	imshow("Showdigit",show);
	waitKey();


	vector<vector<Point> > contours;
	vector<Vec4i> heirarchy;
	int threshold = 50;
	double largest_area = 0;
	int la_co_ind = 0;
	Rect bounding_rect;
	//Canny(img, img, threshold, threshold*3, 3);
	//imshow("Showcanny", img);
	//waitKey();
	findContours(img, contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE); //Try RETR_TREE also
	cout << "\nNumber of contours: " << contours.size() << "\n";
	for(unsigned int i = 0; i< contours.size(); i++ ) // iterate through each contour.
	{
		double a=contourArea( contours[i],false);  //  Find the area of contour
	    if(a>largest_area) {
	    	largest_area=a;
	    	la_co_ind=i;                //Store the index of largest contour
	    	bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
	    }
	}
	drawContours(lar_con, contours, la_co_ind, Scalar(0,0,255), 1, 8, heirarchy, 0);
	//rectangle(lar_con, bounding_rect,  Scalar(0,0,255),1, 8,0);
	resize(lar_con, show, show.size(), 0, 0, CV_INTER_CUBIC);
	imshow("Showdigit", show);
	waitKey(0);
	destroyAllWindows();
	*/
	cout << "Out of prep_digit\n";
}

void drawLine(Vec2f line, Mat &img, Scalar rgb) {
	if(line[1]!=0) {
		float m = -1/tan(line[1]);
		float c = line[0]/sin(line[1]);
		cv::line(img, Point(0, c), Point(img.size().width, m*img.size().width+c), rgb);
	}
	else {
		cv::line(img, Point(line[0], 0), Point(line[0], img.size().height), rgb);
	}
}

void test(Mat img) {
	Mat gray = Mat(img.size(), img.type());
	Mat cont = Mat::zeros(img.size(), img.type());
	//cvtColor(img, gray, CV_BGR2GRAY);
	threshold(img, gray, 127, 255, THRESH_BINARY);
	cout << "Digit: " << gray << "\n";

	vector<vector<Point> > contours;
	vector<Vec4i> heirarchy;
	findContours(gray, contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE); //Try RETR_TREE also
	drawContours(cont, contours, -1, Scalar(0,0,255), 1, 8, heirarchy, 1);
	cout << "\n Number of contours" << contours.size() << " \n";
	cout << "Digit: " << cont << "\n";
	waitKey();
}

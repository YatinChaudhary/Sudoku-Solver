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
void prep_digit(Mat img);
void drawLine(Vec2f line, Mat &img, Scalar rgb);
void mergeRelatedLines(vector<Vec2f> *lines, Mat &img);

int main() {
	//Read the image file
	Mat sudoku = imread("Sudoku_6.jpg",0);

	//Create windows
	namedWindow("Sudoku", CV_WINDOW_AUTOSIZE);
	namedWindow("Gaussian", CV_WINDOW_AUTOSIZE);
	namedWindow("Outer_box", CV_WINDOW_AUTOSIZE);
	namedWindow("contour_box", CV_WINDOW_AUTOSIZE);

	//Show the original image
	imshow("Sudoku", sudoku);
	waitKey();

	//Create boxes for outer box of sudoku and blurred image
	Mat sudoku_box = Mat::zeros(sudoku.size(), sudoku.type());
	Mat gaus, med, bil;

	//Applying blurring
	GaussianBlur(sudoku, gaus, Size(11,11),0,0);

	//Checking blurring
	imshow("Gaussian", gaus);
	waitKey();

	//Applying thresholding
	adaptiveThreshold(gaus, sudoku_box, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5, 2);
	medianBlur(sudoku_box, sudoku_box, 3);
	bitwise_not(sudoku_box, sudoku_box);
	Mat kernel = (Mat_<uchar>(3,3) << 0,1,0,1,1,1,0,1,0);
	dilate(sudoku_box, sudoku_box, kernel);
	imshow("Outer_box", sudoku_box);
	waitKey();

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
	waitKey();

	//Alternate method by detecting Hough Lines
	vector<Vec2f> lines;
	HoughLines(sudoku_box, lines, 1, CV_PI/180, 300);
	mergeRelatedLines(&lines, sudoku_box);

	// Now detect the lines on extremes
	Vec2f topEdge = Vec2f(1000,1000);
	double topYIntercept=100000, topXIntercept=0;
	Vec2f bottomEdge = Vec2f(-1000,-1000);
	double bottomYIntercept=0, bottomXIntercept=0;
	Vec2f leftEdge = Vec2f(1000,1000);
	double leftXIntercept=100000, leftYIntercept=0;
	Vec2f rightEdge = Vec2f(-1000,-1000);
	double rightXIntercept=0, rightYIntercept=0;
	for(int i=0;i<lines.size();i++) {
		Vec2f current = lines[i];
		float p=current[0];
		float theta=current[1];
		if(p==0 && theta==-100)  continue;
		double xIntercept, yIntercept;
		xIntercept = p/cos(theta);
		yIntercept = p/(cos(theta)*sin(theta));
		if(theta>CV_PI*80/180 && theta<CV_PI*100/180) {
			if(p<topEdge[0])
				topEdge = current;
			if(p>bottomEdge[0])
				bottomEdge = current;
		}
		else if(theta<CV_PI*10/180 || theta>CV_PI*170/180) {
			if(xIntercept>rightXIntercept) {
				rightEdge = current;
				rightXIntercept = xIntercept;
			}
			else if(xIntercept<=leftXIntercept) {
				leftEdge = current;
				leftXIntercept = xIntercept;
			}
		}
	}
	drawLine(topEdge, sudoku_box, CV_RGB(0,0,255));
	drawLine(bottomEdge, sudoku_box, CV_RGB(0,0,255));
	drawLine(leftEdge, sudoku_box, CV_RGB(0,0,255));
	drawLine(rightEdge, sudoku_box, CV_RGB(0,0,255));

	//for(unsigned int i=0;i<lines.size();i++) {
	//	drawLine(lines[i], sudoku_box, CV_RGB(0,0,128));
	//}
	imshow("Outer_box",sudoku_box);

	//Extracting Sudoku from Original Image
	/*
	Point2f src[k], dst[k];

	dst[0] = Point(0,0);
	dst[1] = Point(0,maxdistance-1);
	dst[2] = Point(maxdistance-1,maxdistance-1);
	dst[3] = Point(maxdistance-1,0);

	src[0] = bounding_box[0];
	src[1] = bounding_box[1];
	src[2] = bounding_box[2];
	src[3] = bounding_box[3];

	Mat undistorted = Mat(Size(maxdistance, maxdistance), sudoku.type());

	warpPerspective(sudoku, undistorted, getPerspectiveTransform(src,dst), Size(maxdistance, maxdistance));
	GaussianBlur(undistorted,undistorted,Size(5,5),0,0);
	adaptiveThreshold(undistorted, undistorted, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, 2);
	//dilate(undistorted,undistorted,kernel);
	bitwise_not(undistorted, undistorted);

	//Showing extracted sudoku
	namedWindow("Undistorted", CV_WINDOW_AUTOSIZE);
	namedWindow("180x180", CV_WINDOW_AUTOSIZE);
	imshow("Undistorted",undistorted);
	cout << "Undistorted size: " << undistorted.size() << "\n";
	waitKey(0);

	//Retrieve all digits in extracted sudoku
	int clearance = 2;
	int sudo_size = 9;
	int dim = ((maxdistance/sudo_size)-(2*clearance));
	Mat sudoku_mat = Mat(Size(81, pow(dim,2)), sudoku.type());
	getalldigits(undistorted,sudoku_mat,dim,clearance);
	cout << "bhosad\n";

	//Preprocess all digits obtained
	Mat test = imread("/home/yatin/workspace/Sudoku/Digits/Image_1_7.jpg", 0);
	cout << "Type of extracted digit is: " << test.type() << "and" << test.size() << "\n";
	cout << "Type of sudoku is: " << sudoku.type() << "and" << sudoku.size() << "\n";
	//prep_digit(test);
	 *
	 */

	//Wait for any keypress for infinite time and destroys all windows after that
	waitKey();
	destroyAllWindows();

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
			name << "/home/yatin/workspace/Sudoku/Digits/Image_" << (j+1) << "_" << (i+1) << ".jpg";
			imwrite(name.str(), digit);
		}
	}
	//cout << "cp3\n";
	cout << "Out of getalldigits\n";
	//return digits;
}

void prep_digit(Mat img) {
	cout << "inside prep_digit\n";
	Mat mask = Mat::ones(img.size(), img.type());
	Mat lar_con = Mat(img.size(), img.type());
	Mat show = Mat(Size(180,180), img.type());
	Mat org = Mat(Size(180,180), img.type());
	namedWindow("Showdigit", CV_WINDOW_AUTOSIZE);
	namedWindow("orgdigit", CV_WINDOW_AUTOSIZE);
	namedWindow("Showcanny", CV_WINDOW_AUTOSIZE);

	//Floodfilling from sides to remove extra whitespace
	unsigned int i,j;
	for (i=0; i<img.size().height; i++) {
		floodFill(img, Point(0,i), CV_RGB(0,0,0));
		floodFill(img, Point(img.size().width-1,i), CV_RGB(0,0,0));
	}

	for (i=0; i<img.size().height; i++) {
		for (j=0; j<img.size().width; j++) {
			if(img.at<uchar>(Point(j,i)) <= 250)
				floodFill(img, Point(j,i), CV_RGB(0,0,0));
		}
	}

	resize(img, org, org.size(), 0, 0, CV_INTER_LINEAR);
	imshow("orgdigit", org);
	waitKey();

	vector<vector<Point> > contours;
	vector<Vec4i> heirarchy;
	double largest_area = 0;
	int la_co_ind = 0;
	Rect bounding_rect;
	findContours(img, contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE); //Try RETR_TREE also
	for(i = 0; i< contours.size(); i++ ) // iterate through each contour.
	{
		double a=contourArea( contours[i],false);  //  Find the area of contour
	    if(a>largest_area) {
	    	largest_area=a;
	    	la_co_ind=i;                //Store the index of largest contour
	    	bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
	    }
	}
	drawContours(lar_con, contours, la_co_ind, CV_RGB(0,0,255), 1, 8, heirarchy, 0);
	//rectangle(lar_con, bounding_rect,  Scalar(0,0,255),1, 8,0);
	resize(lar_con, show, show.size(), 0, 0, CV_INTER_LINEAR);
	imshow("Showdigit", show);
	waitKey(0);
	destroyAllWindows();
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

void mergeRelatedLines(vector<Vec2f> *lines, Mat &img) {
	vector<Vec2f>::iterator current;
	for(current=lines->begin();current!=lines->end();current++) {
		if((*current)[0]==0 && (*current)[1]==-100) continue;
		float p1 = (*current)[0];
		float theta1 = (*current)[1];
		Point pt1current, pt2current;
		if(theta1>CV_PI*45/180 && theta1<CV_PI*135/180) {
			pt1current.x=0;
			pt1current.y = p1/sin(theta1);
			pt2current.x=img.size().width;
			pt2current.y=-pt2current.x/tan(theta1) + p1/sin(theta1);
		}
		else {
			pt1current.y=0;
			pt1current.x=p1/cos(theta1);
			pt2current.y=img.size().height;
			pt2current.x=-pt2current.y/tan(theta1) + p1/cos(theta1);
		}
		vector<Vec2f>::iterator pos;
		for(pos=lines->begin(); pos!=lines->end(); pos++) {
			if(*current==*pos) continue;
			if(fabs((*pos)[0]-(*current)[0])<30 && fabs((*pos)[1]-(*current)[1])<CV_PI*10/180) {
				float p = (*pos)[0];
				float theta = (*pos)[1];
				Point pt1, pt2;
				if((*pos)[1]>CV_PI*45/180 && (*pos)[1]<CV_PI*135/180) {
					pt1.x=0;
					pt1.y = p/sin(theta);
					pt2.x=img.size().width;
					pt2.y=-pt2.x/tan(theta) + p/sin(theta);
				}
				else {
					pt1.y=0;
					pt1.x=p/cos(theta);
					pt2.y=img.size().height;
					pt2.x=-pt2.y/tan(theta) + p/cos(theta);
				}
				if(((double)(pt1.x-pt1current.x)*(pt1.x-pt1current.x) + (pt1.y-pt1current.y)*(pt1.y-pt1current.y)<64*64) && ((double)(pt2.x-pt2current.x)*(pt2.x-pt2current.x) + (pt2.y-pt2current.y)*(pt2.y-pt2current.y)<64*64)) {
					// Merge the two
					(*current)[0] = ((*current)[0]+(*pos)[0])/2;
					(*current)[1] = ((*current)[1]+(*pos)[1])/2;
					(*pos)[0]=0;
					(*pos)[1]=-100;
				}
			}
		}
	}
}

#include "Sudoku_Solve.h"

Sudoku_Solve::Sudoku_Solve()
	:	s_util()
{
	src = nullptr;
	dst = nullptr;

	k = 0;
	maxdistance = 0;
	clearance = 3;
	sudo_size = 9;
}


Sudoku_Solve::~Sudoku_Solve()
{
	delete[] src;
	delete[] dst;
}

//----------------------------------------------------------------------------------
//					Member Functions
//----------------------------------------------------------------------------------

void Sudoku_Solve::init()
{
	//Read the image file
	cv::Mat sudoku = cv::imread("Sudoku_4.jpg", 0);

	//Create windows
	cv::namedWindow("Sudoku", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("Gaussian", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("Outer_box", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("contour_box", CV_WINDOW_AUTOSIZE);

	//Show the original image
	cv::imshow("Sudoku", sudoku);

	//Create boxes for outer box of sudoku and blurred image
	cv::Mat sudoku_box = cv::Mat::zeros(sudoku.size(), sudoku.type());
}

void Sudoku_Solve::blurring_and_thresholding()
{
	//Applying blurring
	GaussianBlur(sudoku, gauss, cv::Size(11, 11), 0, 0);

	//Checking blurring
	cv::imshow("Gaussian", gauss);

	//Applying thresholding
	cv::adaptiveThreshold(gauss, sudoku_box, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 3);
	medianBlur(sudoku_box, sudoku_box, 3);
	bitwise_not(sudoku_box, sudoku_box);
	kernel = (cv::Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
	dilate(sudoku_box, sudoku_box, kernel);
	imshow("Outer_box", sudoku_box);
}

void Sudoku_Solve::blob_detection()
{
	//Blob Detection - Filling of all white space

	//int count=0;
	int max = -1;

	cv::Point maxPt;

	for (int y = 0; y<sudoku_box.size().height; y++) {
		uchar *row = sudoku_box.ptr(y);

		for (int x = 0; x<sudoku_box.size().width; x++) {

			if (row[x] >= 128) {
				int area = floodFill(sudoku_box, cv::Point(x, y), CV_RGB(0, 0, 64));

				if (area>max) {
					maxPt = cv::Point(x, y);
					max = area;
				}
			}
		}
	}

	//Turn biggest blob white
	floodFill(sudoku_box, maxPt, CV_RGB(0, 0, 255));

	//Turning every other blob to black
	for (int y = 0; y<sudoku_box.size().height; y++) {
		uchar *row = sudoku_box.ptr(y);

		for (int x = 0; x<sudoku_box.size().width; x++) {

			if (row[x] == 64 && x != maxPt.x && y != maxPt.y) {
				floodFill(sudoku_box, cv::Point(x, y), CV_RGB(0, 0, 0));
			}
		}
	}
	//Eroding the image as we have dilated it earlier to restore the original image
	erode(sudoku_box, sudoku_box, kernel);
	imshow("Outer_box", sudoku_box);
}

void Sudoku_Solve::contour()
{
	//Creating a bounding box around sudoku by detecting the biggest contour
	con = cv::Mat::zeros(sudoku.size(), sudoku.type());

	findContours(sudoku_box, contours, heirarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	drawContours(con, contours, 0, cv::Scalar(128, 128, 255), 1, CV_AA, cv::noArray(), 2);
	approxPolyDP(contours[0], bounding_box, 10, true);
	bounding_box = s_util.order_rect_corners(bounding_box);
	k = bounding_box.size();
	maxdistance = 0;

	for (int i = 0; i<k; i++)
	{
		double dist(0);
		if (i != k - 1) {
			line(con, cv::Point(bounding_box[i].x, bounding_box[i].y), cv::Point(bounding_box[i + 1].x, bounding_box[i + 1].y), CV_RGB(0, 0, 255), 1, CV_AA);
			dist = cv::norm(bounding_box[i] - bounding_box[i + 1]);
		}
		else {
			line(con, cv::Point(bounding_box[k - 1].x, bounding_box[k - 1].y), cv::Point(bounding_box[0].x, bounding_box[0].y), CV_RGB(0, 0, 255), 1, CV_AA);
			dist = cv::norm(bounding_box[k - 1] - bounding_box[0]);
		}
		if (dist > maxdistance)
			maxdistance = static_cast<int>(dist);
		//cout << "\nCoordinates of Point no " << i << " are: " << bounding_box[i].x << " , " << bounding_box[i].y < "\n";
	}

	maxdistance -= (maxdistance % 9);
	std::cout << "Total no of bounding box points: " << k << "\n";
	std::cout << "Dimension of extracted sudoku: " << maxdistance << "\n";
	cv::imshow("contour_box", con);
}

void Sudoku_Solve::extract_sudoku()
{
	//Extracting Sudoku from Original Image
	src = new cv::Point2f[k];
	dst = new cv::Point2f[k];

	dst[0] = cv::Point(0, 0);
	dst[1] = cv::Point(0, maxdistance - 1);
	dst[2] = cv::Point(maxdistance - 1, maxdistance - 1);
	dst[3] = cv::Point(maxdistance - 1, 0);

	src[0] = bounding_box[0];
	src[1] = bounding_box[1];
	src[2] = bounding_box[2];
	src[3] = bounding_box[3];

	//Mat undistorted = Mat(Size(maxdistance, maxdistance), sudoku.type());
	cv::Mat undistorted = cv::Mat(cv::Size(maxdistance, maxdistance), CV_8UC1);

	cv::warpPerspective(sudoku, undistorted, cv::getPerspectiveTransform(src, dst), cv::Size(maxdistance, maxdistance));
	cv::imwrite("/home/yatin/workspace/Sudoku/Extract_Sudoku.tif", undistorted);
	cv::GaussianBlur(undistorted, undistorted, cv::Size(5, 5), 0, 0);
	cv::adaptiveThreshold(undistorted, undistorted, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 3, 2);
	//threshold(undistorted, undistorted, 127, 255, THRESH_BINARY);
	//erode(undistorted,undistorted,kernel);
	//dilate(undistorted,undistorted,kernel);
	bitwise_not(undistorted, undistorted);

	//Showing extracted sudoku
	cv::namedWindow("Undistorted", CV_WINDOW_AUTOSIZE);
	//namedWindow("180x180", CV_WINDOW_AUTOSIZE);
	cv::imshow("Undistorted", undistorted);
	std::cout << "Undistorted size: " << undistorted.size() << "\n";
}

void Sudoku_Solve::get_all_digits()
{
	//Retrieve all digits in extracted sudoku
	int dim = ((maxdistance / sudo_size) - (2 * clearance));
	cv::Mat sudoku_mat = cv::Mat(cv::Size(81, pow(dim, 2)), sudoku.type());
	s_util.get_digits(undistorted, sudoku_mat, dim, clearance);
	std::cout << "bhosad\n";

	//Preprocess all digits obtained
	cv::Mat x = cv::imread("/home/yatin/workspace/Sudoku/Digits/Image_2_8.tif", CV_8UC1);
	std::cout << "Type of extracted digit is: " << x.type() << "and" << x.size() << "\n";
	std::cout << "Type of sudoku is: " << sudoku.type() << "and" << sudoku.size() << "\n";
	s_util.test(x);
	//cout << "Digit: " << x << "\n";

	char name[100];
	for (unsigned int i = 1; i <= 9; i++)
	{
		for (unsigned int j = 1; j <= 9; j++)
		{
			sprintf(name, "/home/yatin/workspace/Sudoku/Digits/Image_%d_%d.tif", i, j);
			cv::Mat digit = cv::imread(name, CV_8UC1);
			s_util.prep_digit(digit, i, j);
		}
	}
}
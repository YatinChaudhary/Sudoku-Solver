#include "Sudoku_Solve.h"

Sudoku_Solve::Sudoku_Solve()
	:	s_util()
{
	src = nullptr;
	dst = nullptr;

	k = 0;
	maxdistance = 0;
	clearance = 2;
	sudo_size = 9;

	kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

	// windows names
	w_sudoku = "Sudoku";
	w_gaussian = "Gaussian";
	w_outer_box = "Outer box";
	w_contour_box = "Contour box";
	w_undistorted = "Undistorted";

	max_area = -1;
	dim = 0;

	GetCurrentDirectory(MAX_PATH_, pth);
	cwd_path.operator=(pth);
}


Sudoku_Solve::~Sudoku_Solve()
{
	delete[] src;
	delete[] dst;
}

//----------------------------------------------------------------------------------
//					Member Functions
//----------------------------------------------------------------------------------

void Sudoku_Solve::test()
{
	init();

	cv::waitKey(2000);

	blurring_and_thresholding();

	cv::waitKey(2000);

	blob_detection();

	cv::waitKey(2000);

	contour();

	cv::waitKey(2000);

	extract_sudoku();

	cv::waitKey(2000);

	get_all_digits();
}

void Sudoku_Solve::init()
{
	//Read the image file
	sudoku = cv::imread("Sudoku_Images/Sudoku_4.jpg", 0);

	//Create windows
	cv::namedWindow(w_sudoku, CV_WINDOW_AUTOSIZE);
	cv::namedWindow(w_gaussian, CV_WINDOW_AUTOSIZE);
	cv::namedWindow(w_outer_box, CV_WINDOW_AUTOSIZE);
	cv::namedWindow(w_contour_box, CV_WINDOW_AUTOSIZE);

	//Show the original image
	cv::imshow(w_sudoku, sudoku);

	//Create boxes for outer box of sudoku and blurred image
	sudoku_box = cv::Mat::zeros(sudoku.size(), sudoku.type());
}

void Sudoku_Solve::blurring_and_thresholding()
{
	//Applying blurring
	GaussianBlur(sudoku, gauss, cv::Size(11, 11), 0, 0);

	//Checking blurring
	cv::imshow(w_gaussian, gauss);

	//Applying thresholding
	cv::adaptiveThreshold(gauss, sudoku_box, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 3);
	cv::medianBlur(sudoku_box, sudoku_box, 3);
	cv::bitwise_not(sudoku_box, sudoku_box);
	//kernel = (cv::Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
	cv::erode(sudoku_box, sudoku_box, kernel);
	cv::dilate(sudoku_box, sudoku_box, kernel);
	cv::imshow(w_outer_box, sudoku_box);
}

void Sudoku_Solve::blob_detection()
{
	//Blob Detection - Filling of all white space

	for (int y = 0; y<sudoku_box.size().height; y++) {
		uchar *row = sudoku_box.ptr(y);

		for (int x = 0; x<sudoku_box.size().width; x++) {

			if (row[x] >= 128) {
				int area = cv::floodFill(sudoku_box, cv::Point(x, y), CV_RGB(0, 0, 64));

				if (area>max_area) {
					max_pt = cv::Point(x, y);
					max_area = area;
				}
			}
		}
	}

	//Turn biggest blob white
	cv::floodFill(sudoku_box, max_pt, CV_RGB(0, 0, 255));

	//Turning every other blob to black
	for (int y = 0; y<sudoku_box.size().height; y++) {
		uchar *row = sudoku_box.ptr(y);

		for (int x = 0; x<sudoku_box.size().width; x++) {

			if (row[x] == 64 && x != max_pt.x && y != max_pt.y) {
				cv::floodFill(sudoku_box, cv::Point(x, y), CV_RGB(0, 0, 0));
			}
		}
	}
	//Eroding the image as we have dilated it earlier to restore the original image
	
	cv::imshow(w_outer_box, sudoku_box);
}

void Sudoku_Solve::contour()
{
	//Creating a bounding box around sudoku by detecting the biggest contour
	con = cv::Mat::zeros(sudoku.size(), sudoku.type());

	cv::findContours(sudoku_box, contours, heirarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(con, contours, 0, cv::Scalar(128, 128, 255), 1, CV_AA, cv::noArray(), 2);
	cv::approxPolyDP(contours[0], bounding_box, 10, true);

	s_util.order_rect_corners(bounding_box);

	k = bounding_box.size();
	maxdistance = 0;

	for (int i = 0; i<k; i++)
	{
		double dist(0);
		if (i != k - 1) {
			cv::line(con, cv::Point(bounding_box[i].x, bounding_box[i].y), cv::Point(bounding_box[i + 1].x, bounding_box[i + 1].y), CV_RGB(0, 0, 255), 1, CV_AA);
			dist = cv::norm(bounding_box[i] - bounding_box[i + 1]);
		}
		else {
			cv::line(con, cv::Point(bounding_box[k - 1].x, bounding_box[k - 1].y), cv::Point(bounding_box[0].x, bounding_box[0].y), CV_RGB(0, 0, 255), 1, CV_AA);
			dist = cv::norm(bounding_box[k - 1] - bounding_box[0]);
		}
		if (dist > maxdistance)
			maxdistance = static_cast<int>(dist);
		//cout << "\nCoordinates of Point no " << i << " are: " << bounding_box[i].x << " , " << bounding_box[i].y < "\n";
	}

	maxdistance -= (maxdistance % 9);
	cv::imshow(w_contour_box, con);
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
	undistorted = cv::Mat(cv::Size(maxdistance, maxdistance), CV_8UC1);

	cv::warpPerspective(sudoku, undistorted, cv::getPerspectiveTransform(src, dst), cv::Size(maxdistance, maxdistance));
	cv::String path_ = cwd_path + "/Sudoku_Images/Extract_Sudoku.tif";
	cv::imwrite(path_, undistorted);
	cv::GaussianBlur(undistorted, undistorted, cv::Size(5, 5), 0, 0);
	cv::adaptiveThreshold(undistorted, undistorted, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 3, 2);
	//threshold(undistorted, undistorted, 127, 255, THRESH_BINARY);
	cv::erode(undistorted,undistorted,kernel);
	cv::dilate(undistorted,undistorted,kernel);
	cv::bitwise_not(undistorted, undistorted);

	//Showing extracted sudoku
	cv::namedWindow(w_undistorted, CV_WINDOW_AUTOSIZE);
	cv::imshow(w_undistorted, undistorted);
}

void Sudoku_Solve::get_all_digits()
{
	//Retrieve all digits in extracted sudoku
	dim = (maxdistance / sudo_size);
	sudoku_mat = cv::Mat(cv::Size(81, pow(dim-2*clearance, 2)), sudoku.type());
	//cv::Mat image = cv::imread("Sudoku_Images/Extract_Sudoku.tif", CV_8UC1);
	//std::string test = std::to_string(image.rows) + "   " + std::to_string(image.cols);
	s_util.get_digits(undistorted, sudoku_mat, dim, clearance, cwd_path);

	//Preprocess all digits obtained
	cv::String path_ = cwd_path + "/Digits/Image_2_8.tif";
	cv::Mat x = cv::imread(path_, CV_8UC1);
	s_util.test(x);
	
	cv::String digit_path;
	for (unsigned int i = 1; i <= 9; i++)
	{
		for (unsigned int j = 1; j <= 9; j++)
		{
			digit_path = "Digits/Image_" + std::to_string(i) + "_" + std::to_string(j) + ".tif";
			cv::Mat digit = cv::imread(digit_path, CV_8UC1);
			s_util.prep_digit(digit, i, j);
		}
	}
}
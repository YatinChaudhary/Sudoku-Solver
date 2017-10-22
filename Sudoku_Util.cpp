#include "Sudoku_Util.h"

Sudoku_Util::Sudoku_Util()
{
	log_file.open("log.txt");
}


Sudoku_Util::~Sudoku_Util()
{
	log_file.close();
}

//----------------------------------------------------------------------------------
//					Member Functions
//----------------------------------------------------------------------------------

void Sudoku_Util::order_points_by_rows(std::vector<cv::Point>& points)
{
	status = errorCode::NO_ERROR;
	try
	{
		int k = points.size();
		int key, i, j;
		for (i = 1; i<k; i++) {
			key = points[i].y;
			for (j = i - 1; (j >= 0) && (points[j].y > key); j--) {
				points[j + 1].y = points[j].y;
			}
			points[j + 1].y = key;
		}
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::orderPointsByRows: " <<  ex.what() << "\n\n";
	}
}

std::vector<cv::Point> Sudoku_Util::order_rect_corners(std::vector<cv::Point> corners)
{
	status = errorCode::NO_ERROR;
	try
	{
		std::vector<cv::Point> ordCorners;
		std::cout << "in\n";
		int i, k, min, max, a(0), b(0);
		int* d;
		k = corners.size();
		d = new (std::nothrow) int[k];
		if (d == NULL) {
			std::cout << "error assigning memory. Take measures.\n";
		}
		for (i = 0; i<k; i++) {
			d[i] = static_cast<int>(sqrt(pow(corners[i].x, 2) + pow(corners[i].y, 2)));
		}
		std::cout << "1\n";
		min = *std::min_element(d, d + k);
		max = *std::max_element(d, d + k);
		std::cout << "2\n";
		std::cout << "min and max value: " << min << " and " << max << "\n";
		std::cout << "Point 1 " << corners[0].x << " and " << corners[0].y << "\n";
		std::cout << "Point 2 " << corners[1].x << " and " << corners[1].y << "\n";
		std::cout << "Point 3 " << corners[2].x << " and " << corners[2].y << "\n";
		std::cout << "Point 4 " << corners[3].x << " and " << corners[3].y << "\n";
		for (i = 0; i<k; i++) {
			std::cout << "check " << d[i] << "\n";
			if (d[i] == min)
				min = i;
			else if (d[i] == max)
				max = i;
			else if (a == 0)
				a = i;
			else
				b = i;
		}
		ordCorners.push_back(corners[min]);
		std::cout << "3\n";
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
		std::cout << "out\n";
		delete[] d;
		return ordCorners;
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::orderRectCorners: " << ex.what() << "\n\n";
	}
}

void Sudoku_Util::shift_points(std::vector<cv::Point>& points)
{
	status = errorCode::NO_ERROR;
	try
	{
		int k(points.size()), i;
		cv::Point last = points[0];
		for (i = 1; i<k; i++) {
			points[i - 1] = points[i];
		}
		points[k - 1] = last;
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::shift_points: " << ex.what() << "\n\n";
	}
}

void Sudoku_Util::get_digits(cv::Mat img, cv::Mat x, int max, int clr)
{
	status = errorCode::NO_ERROR;
	try
	{
		std::cout << "inside get_digits\n";
		int size = (img.size().height) / 9;
		int a = clr;
		int i, j, k, l;
		std::cout << "cp1\n";
		cv::Mat digits = cv::Mat(cv::Size(81, pow(max, 2)), img.type());
		cv::Mat digit = cv::Mat(cv::Size(size - 2 * a, size - 2 * a), img.type());
		std::cout << "cp2\n";
		for (i = 0; i<9; i++) {
			for (j = 0; j<9; j++) {
				for (k = a; k<size - a; k++) {
					for (l = a; l<size - a; l++) {
						//cout << "cp3\n";
						x.at<uchar>(cv::Point(9 * i + j, size*(k - a) + (l - a))) = img.at<uchar>(cv::Point(size*i + k, size*j + l));
						//cout << "cp4\n";
						digit.at<uchar>(cv::Point((k - a), (l - a))) = img.at<uchar>(cv::Point(size*i + k, size*j + l));
						std::cout << i << " " << j << " " << k << " " << l << "\n";
					}
				}
				std::ostringstream name;
				name << "/home/yatin/workspace/Sudoku/Digits/Image_" << (j + 1) << "_" << (i + 1) << ".tif";
				cv::imwrite(name.str(), digit);
			}
		}
		//cout << "cp3\n";
		std::cout << "Out of get_digits\n";
		//return digits;
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::get_digits: " << ex.what() << "\n\n";
	}
}

void Sudoku_Util::prep_digit(cv::Mat img, int a, int b)
{
	status = errorCode::NO_ERROR;
	try
	{
		std::cout << "inside prep_digit\n";
		//Mat filt = cv::Mat(img.size(), img.type());
		//Mat lar_con = cv::Mat(img.size(), img.type());
		//Mat show = cv::Mat(cv::Size(180,180), img.type());
		cv::Mat org = cv::Mat(cv::Size(100, 100), img.type());
		cv::namedWindow("Showdigit", CV_WINDOW_AUTOSIZE);
		//cv::namedWindow("orgdigit", CV_WINDOW_AUTOSIZE);
		//cv::namedWindow("Showcanny", CV_WINDOW_AUTOSIZE);

		//Floodfilling from sides to remove extra whitespace
		int i, j;
		for (i = 0; i<img.size().height; i++) {
			floodFill(img, cv::Point(0, i), CV_RGB(0, 0, 0));
			floodFill(img, cv::Point(img.size().width - 1, i), CV_RGB(0, 0, 0));
		}

		for (i = 0; i<img.size().width; i++) {
			floodFill(img, cv::Point(i, 0), CV_RGB(0, 0, 0));
			floodFill(img, cv::Point(i, img.size().height - 1), CV_RGB(0, 0, 0));
		}

		for (i = 0; i<img.size().height; i++) {
			for (j = 0; j<img.size().width; j++) {
				if (img.at<uchar>(cv::Point(j, i)) <= 250)
					floodFill(img, cv::Point(j, i), CV_RGB(0, 0, 0));
			}
		}

		std::ostringstream name;
		name << "/home/yatin/workspace/Sudoku/Digits/Image_" << a << "_" << b << ".tif";
		resize(img, org, org.size(), 0, 0, CV_INTER_LINEAR);
		cv::Mat kernel = (cv::Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
		erode(org, org, kernel);
		dilate(org, org, kernel);
		//imshow("Showdigit",org);
		cv::imwrite(name.str(), org);

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
		std::cout << "Out of prep_digit\n";
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::prep_digit: " << ex.what() << "\n\n";
	}
}

void Sudoku_Util::draw_line(cv::Vec2f line, cv::Mat &img, cv::Scalar rgb)
{
	status = errorCode::NO_ERROR;
	try
	{
		if (line[1] != 0) {
			float m = -1 / tan(line[1]);
			float c = line[0] / sin(line[1]);
			cv::line(img, cv::Point(0, c), cv::Point(img.size().width, m*img.size().width + c), rgb);
		}
		else {
			cv::line(img, cv::Point(line[0], 0), cv::Point(line[0], img.size().height), rgb);
		}
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::draw_line: " << ex.what() << "\n\n";
	}
}

// Recognize the digits
unsigned int Sudoku_Util::recognize_digit()
{
	status = errorCode::NO_ERROR;
	try
	{

	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::recognize_digit: " << ex.what() << "\n\n";
	}
}

void Sudoku_Util::test(cv::Mat img)
{
	status = errorCode::NO_ERROR;
	try
	{
		cv::Mat gray = cv::Mat(img.size(), img.type());
		cv::Mat cont = cv::Mat::zeros(img.size(), img.type());
		//cvtColor(img, gray, CV_BGR2GRAY);
		threshold(img, gray, 127, 255, cv::THRESH_BINARY);
		std::cout << "Digit: " << gray << "\n";

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> heirarchy;
		cv::findContours(gray, contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE); //Try RETR_TREE also
		cv::drawContours(cont, contours, -1, cv::Scalar(0, 0, 255), 1, 8, heirarchy, 1);
		std::cout << "\n Number of contours" << contours.size() << " \n";
		std::cout << "Digit: " << cont << "\n";
		cv::waitKey();
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::test: " << ex.what() << "\n\n";
	}
}
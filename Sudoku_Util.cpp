#include "Sudoku_Util.h"

Sudoku_Util::Sudoku_Util()
{
	log_file.open("log.txt");
	if (log_file.is_open()) { 
		//std::cout << "File not open\n"; 
	}
	else { 
		//std::cout << "File open\n"; 
	}
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

void Sudoku_Util::order_rect_corners(std::vector<cv::Point>& corners)
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
		corners.operator=(ordCorners);
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

void Sudoku_Util::get_digits(cv::Mat& img, cv::Mat& s_mat, int dim, int clr, cv::String pth_)
{
	status = errorCode::NO_ERROR;
	try
	{
		int height = img.rows;
		int width = img.cols;
		int i = 0,
			j = 0,
			dy = dim,
			dx = dim;
		dim -= 2*clr;			// dimension of ROI

		cv::Mat digit;
		cv::String d_path;

		for (int y = 0; y < height; y += dy)
		{
			j = 0;
			for (int x = 0; x < width; x += dx)
			{
				cv::Rect rect = cv::Rect(x + clr, y + clr, dim, dim);
				digit = cv::Mat(img, rect);
				d_path = pth_ + "/Digits/Image_" + std::to_string(i) + "_" + std::to_string(j) + ".tif";
				cv::imwrite(d_path, digit);
				j++;
			}
			i++;
		}
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
		cv::Mat tmp;
		cv::Mat org = cv::Mat(cv::Size(100, 100), img.type());
		cv::threshold(img, tmp, 150, 255, cv::THRESH_BINARY);

		//Floodfilling from sides to remove extra whitespace
		int i, j;
		for (i = 0; i<tmp.size().height; i++) {
			floodFill(tmp, cv::Point(0, i), cv::Scalar(0));
			floodFill(tmp, cv::Point(tmp.size().width - 1, i), cv::Scalar(0));
		}

		for (i = 0; i<tmp.size().width; i++) {
			floodFill(tmp, cv::Point(i, 0), cv::Scalar(0));
			floodFill(tmp, cv::Point(i, tmp.size().height - 1), cv::Scalar(0));
		}
		/*
		for (i = 0; i<tmp.size().height; i++) {
			for (j = 0; j<tmp.size().width; j++) {
				if (tmp.at<uchar>(cv::Point(j, i)) <= 250)
					floodFill(tmp, cv::Point(j, i), cv::Scalar(0));
			}
		}
		*/
		cv::String name;
		name = "Digits_Prep/Image_" + std::to_string(a) + "_" + std::to_string(b) + ".tif";
		cv::resize(tmp, org, org.size(), 0, 0, CV_INTER_CUBIC);
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
		erode(org, org, kernel);
		dilate(org, org, kernel);
		cv::imwrite(name, org);

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
		return 0;
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::recognize_digit: " << ex.what() << "\n\n";
	}
	return 0;
}

void Sudoku_Util::test(cv::Mat img)
{
	status = errorCode::NO_ERROR;
	try
	{
		int thresh = 100;
		cv::Mat gray;
		cv::Mat edges;

		//cv::cvtColor(img, gray, CV_BGR2GRAY);
		cv::blur(img, gray, cv::Size(3, 3));
		cv::Canny(gray, edges, thresh, thresh * 2, 3);

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> heirarchy;
		cv::findContours(edges, contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cv::Point(0, 0)); //Try RETR_TREE also
		
		cv::Mat cont = cv::Mat::zeros(edges.size(), CV_8UC1);
		
		cv::drawContours(cont, contours, -1, cv::Scalar(255), 1, 8, heirarchy, 1, cv::Point(0, 0));
		
		cv::imwrite("test/cont.tif", cont);
		cv::imwrite("test/gray.tif", gray);
		cv::imwrite("test/edges.tif", edges);
	}
	catch (const std::exception& ex)
	{
		status = errorCode::ERROR;
		log_file << "Sudoku_Util::test: " << ex.what() << "\n\n";
	}
}
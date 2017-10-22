#ifndef SUDOKU_UTIL_H
#define SUDOKU_UTIL_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>
#include <string>
#include <fstream>

#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\core\types.hpp"
#include "opencv2\core\mat.hpp"
#include "opencv2\core.hpp"

class Sudoku_Solve;

enum errorCode
{
	NO_ERROR,
	ERROR
};

class Sudoku_Util
{
// Member variables
private:
	std::fstream log_file;

public:
	int status = 0;

// Member functions
public:
	Sudoku_Util();
	virtual ~Sudoku_Util();

private:
	std::vector<cv::Point> order_rect_corners(std::vector<cv::Point> corners);
	void order_points_by_rows(std::vector<cv::Point>& points);
	void shift_points(std::vector<cv::Point>& points);
	void get_digits(cv::Mat img, cv::Mat x, int max, int clr);
	void prep_digit(cv::Mat img, int a, int b);
	void draw_line(cv::Vec2f line, cv::Mat &img, cv::Scalar rgb);
	unsigned int recognize_digit();												// Recognize the digits
	void test(cv::Mat img);

	friend class Sudoku_Solve;
};

#endif
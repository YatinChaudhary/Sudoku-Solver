#ifndef SUDOKU_SOLVE_H
#define SUDOKU_SOLVE_H

#include "Sudoku_Util.h"

class Sudoku_Solve
{
//Member variables
private:
	cv::Mat sudoku,
			gauss,
			med,
			bil,
			sudoku_box,
			undistorted;

	// Names of windows showing images
	std::string w_sudoku,
				w_gaussian,
				w_outer_box,
				w_contour_box;

	cv::Mat kernel,
			con;

	cv::Point2f *src,
				*dst;

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> bounding_box;
	std::vector<cv::Vec4i> heirarchy;

	int k,						// Total no of bounding box points
		maxdistance,			// Dimension of extracted sudoku
		clearance,
		sudo_size;

	Sudoku_Util s_util;

//Member functions
public:
	Sudoku_Solve();
	virtual ~Sudoku_Solve();

private:
	void init();
	void blurring_and_thresholding();
	void blob_detection();
	void contour();
	void extract_sudoku();
	void get_all_digits();
};

#endif
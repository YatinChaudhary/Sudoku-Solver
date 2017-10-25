#include "Sudoku_Solve.h"

using namespace cv;
using namespace std;

int main()
{
	//cv::Mat test = cv::imread("test/gray.tif", CV_8UC1);
	//std::cout << "test = " << std::endl << " " << test << std::endl << std::endl;
	//MessageBox(NULL, pwd, pwd, 0);

	Sudoku_Solve ss;

	ss.test();

	//cv::waitKey(0);
	/*
	Mat image;

	image = imread("Sudoku_Images/Extract_Sudoku.tif", 1);
	if (image.empty())
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}


	namedWindow("Image", CV_WINDOW_AUTOSIZE);
	imshow("Image", image);


	// get the image data
	int height = image.rows;
	int width = image.cols;
	int dim = 9;
	int i = 0, j = 0;
	int clr = 0;

	cv::Size smallSize(35, 35);
	cv::Mat digit;
	cv::String d_path;

	std::vector <cv::Mat> smallImages;
	namedWindow("smallImages ", CV_WINDOW_AUTOSIZE);

	for (int y = 0; y < height; y += smallSize.height)
	{
		j = 0;
		for (int x = 0; x < width; x += smallSize.width)
		{
			cv::Rect rect = cv::Rect(x+clr, y+clr, smallSize.width-clr, smallSize.height-clr);
			digit = cv::Mat(image, rect).clone();
			smallImages.push_back(cv::Mat(image, rect));
			d_path = "Digits/Image_" + std::to_string(i) + "_" + std::to_string(j) + ".tif";
			cv::imwrite(d_path, digit);
			j++;
		}
		i++;
	}
	*/
	return 0;
}
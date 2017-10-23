#include "Sudoku_Solve.h"


int main()
{
	TCHAR pwd[MAX_PATH_];
	GetCurrentDirectory(MAX_PATH_, pwd);
	//MessageBox(NULL, pwd, pwd, 0);

	Sudoku_Solve ss;

	ss.test();

	cv::waitKey(0);

	return 0;
}
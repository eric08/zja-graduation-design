#pragma once
#include "opencv2/opencv.hpp"
//#include "ulti/gco/GCoptimization.h"

class COptBox
{
public:
	COptBox(void);
	~COptBox(void);

	//AX = B
	void sovleLinearEqu(CvMat* A, CvMat* B, CvMat* X);

	//Graphcuts


	//Belife Propagation
};

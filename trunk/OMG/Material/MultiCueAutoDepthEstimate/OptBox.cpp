#include "stdafx.h"
#include "OptBox.h"

COptBox::COptBox(void)
{
}

COptBox::~COptBox(void)
{
}

void COptBox::sovleLinearEqu(CvMat* A, CvMat* B, CvMat* X)
{
	if (cvSolve(A, B, X, CV_LU)==0)
	{
		//cvSolve(A, B, X, CV_SVD);
		cvSolve(A, B, X, CV_SVD_SYM);
	}
	
}

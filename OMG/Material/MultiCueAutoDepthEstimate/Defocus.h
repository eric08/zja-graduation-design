#pragma once
#include "opencv2/opencv.hpp"


//shape/depth from defocus

class CDefocus
{
public:
	CDefocus(void);
	~CDefocus(void);

	void run(IplImage* src, int scale);

	void depthFromDefocus(IplImage* src, IplImage* depthMap);
	int m_nWaveletLayer;
	int m_nWinSize;

////2D wavelet transform for blur region evaluation
	void wavelet2DColor(IplImage* src, int nLayer, IplImage* pWavelet);
	void DWT(IplImage *pImage, int nLayer);

	void adaptiveFocusFilter(IplImage* srcImg, IplImage* srcDepthImage, int winSize, IplImage* dstDepthImage);
	void imageExtending(IplImage* src, int winSize, IplImage* dst);

	void createDistanceWeightMat(int winSize, CvMat* weightMat, float sensitivetyParam);
	void computeColorSimiWeightMat(int winSize, CvMat* srcMat, int srcChannel, CvMat* clrWeightMat, float sensitivetyParam);
	void computeGradientWeightMat(int winSize, CvMat* srcMat, CvMat* graWeightMat, float sensitivetyParam);
};

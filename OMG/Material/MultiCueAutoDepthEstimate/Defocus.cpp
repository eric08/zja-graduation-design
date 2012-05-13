#include "stdafx.h"
#include "Defocus.h"
using namespace std;

CDefocus::CDefocus(void)
{
	m_nWaveletLayer = 1;
	m_nWinSize = 15;
}

CDefocus::~CDefocus(void)
{
}

void CDefocus::depthFromDefocus(IplImage* src, IplImage* DstDepthMap)
{
 //	DWORD start = GetTickCount();
	int nLayer = m_nWaveletLayer;
	IplImage *pSrc = cvCreateImage(cvSize(src->width/1, src->height/1), 8, 3);
	cvResize(src, pSrc, CV_INTER_LINEAR);

	CvSize size = cvGetSize(pSrc);
	if ((pSrc->width >> nLayer) << nLayer != pSrc->width)
	{
		size.width = ((pSrc->width >> nLayer) + 1) << nLayer;
	}
	if ((pSrc->height >> nLayer) << nLayer != pSrc->height)
	{
		size.height = ((pSrc->height >> nLayer) + 1) << nLayer;
	}
	
	IplImage *pWavelet = cvCreateImage(size, IPL_DEPTH_32F, pSrc->nChannels);

	
	cvSetImageROI(pWavelet, cvRect(0, 0, pSrc->width, pSrc->height));
	cvConvertScale(pSrc, pWavelet, 1, -128);
	cvResetImageROI(pWavelet);
	
	// 彩色图像小波变换
	IplImage *pImage = cvCreateImage(cvGetSize(pWavelet), IPL_DEPTH_32F, 1);
	if (pImage)
	{
		for (int i = 1; i <= pWavelet->nChannels; i++)
		{
			cvSetImageCOI(pWavelet, i);
			cvCopy(pWavelet, pImage, NULL);
			DWT(pImage, nLayer);
			cvCopy(pImage, pWavelet, NULL);
		}
		cvSetImageCOI(pWavelet, 0);
		cvReleaseImage(&pImage);
	}
	// 小波变换图象
	cvSetImageROI(pWavelet, cvRect(0, 0, pSrc->width, pSrc->height));

// 	DWORD Elapse1 = GetTickCount()-start;
	//cvConvertScale(pWavelet, pSrc, 1, 128);
	IplImage* temp = cvCreateImage(cvGetSize(pWavelet), IPL_DEPTH_32F, 1);
	IplImage* temp1 = cvCreateImage(cvGetSize(pWavelet), 8, 1);
	
	IplImage* waveletImg = cvCreateImage(cvGetSize(pWavelet), 8, 3);
	for (int i=1; i<=pWavelet->nChannels; i++)
	{
		cvSetImageCOI(pWavelet, i);
		cvCopy(pWavelet, temp);
		cvSetImageCOI(waveletImg, i);
		double maxV, minV;
		cvMinMaxLoc(temp, &minV, &maxV);
		cvConvertScale(temp, temp1, 255.0/(maxV-minV), -255.0*minV/(maxV-minV));
		cvCopy(temp1, waveletImg);
	}

	cvResetImageROI(pWavelet);

	
	IplImage* depthMap = cvCreateImage( cvSize(pWavelet->width/2, pWavelet->height/2), IPL_DEPTH_32F, 1);
	cvSetZero(depthMap);

	IplImage* depthImage = cvCreateImage( cvSize(pWavelet->width/2, pWavelet->height/2), IPL_DEPTH_8U, 1);
	IplImage* depthTemp = cvCreateImage( cvSize(pWavelet->width/2, pWavelet->height/2), IPL_DEPTH_32F, 1);
	IplImage* depthTemp1 = cvCreateImage( cvSize(pWavelet->width/2, pWavelet->height/2), IPL_DEPTH_32F, 1);
	IplImage* tempSub = cvCreateImage(cvSize(pWavelet->width/2, pWavelet->height/2), IPL_DEPTH_32F, pWavelet->nChannels);
	IplImage* tempSub1Channel = cvCreateImage(cvSize(pWavelet->width/2, pWavelet->height/2), IPL_DEPTH_32F, 1);

// 	DWORD Elapse2 = GetTickCount()-start;
	for (int i=1; i<=nLayer; i++)
	{
		CvRect r;
		CvMat *m, mat;
		r.width = pWavelet->width / pow(2.0, i);
		r.height = pWavelet->height / pow(2.0, i);	
		///
		r.x = r.width;
		r.y = 0;
		m = cvGetSubRect(pWavelet, &mat, r);	
		cvResize(m, tempSub, CV_INTER_CUBIC);

		for (int j=1; j<=pWavelet->nChannels; j++ )
		{
			cvSetImageCOI(tempSub, j);
			cvCopy(tempSub, tempSub1Channel);
			cvAbs(tempSub1Channel,tempSub1Channel);		
			cvAdd(tempSub1Channel,depthMap, depthMap );
			cvSetImageCOI(tempSub, 0);
		}
		r.x = r.width;
		r.y = r.height;
		m = cvGetSubRect(pWavelet, &mat, r);	
		cvResize(m, tempSub, CV_INTER_LINEAR);
		for (int j=1; j<=pWavelet->nChannels; j++ )
		{
			cvSetImageCOI(tempSub, j);
			cvCopy(tempSub, tempSub1Channel);
			cvAbs(tempSub1Channel,tempSub1Channel);
			cvAdd(tempSub1Channel,depthMap, depthMap );
			cvSetImageCOI(tempSub, 0);
		}
		r.x = 0;
		r.y = r.height;
		m = cvGetSubRect(pWavelet, &mat, r);	
		cvResize(m, tempSub, CV_INTER_LINEAR);
		for (int j=1; j<=pWavelet->nChannels; j++ )
		{
			cvSetImageCOI(tempSub, j);
			cvCopy(tempSub, tempSub1Channel);
			cvAbs(tempSub1Channel,tempSub1Channel);
			cvAdd(tempSub1Channel,depthMap, depthMap );
			cvSetImageCOI(tempSub, 0);
		}
	}
//	DWORD Elapse3 = GetTickCount()-start;

	double maxv, minv;
	cvMinMaxLoc(depthMap, &minv, &maxv);
	if(minv == maxv)
	{
		;//do nothing
	}
	else
	{
		cvConvertScale( depthMap, depthImage, 255.0/(maxv-minv), -255.0*minv/(maxv-minv) );
	}
	
	cvReleaseImage(&tempSub);
	cvReleaseImage(&tempSub1Channel);
//	DWORD Elapse4 = GetTickCount()-start;


	IplImage* refinedDepth = cvCreateImage(cvGetSize(pSrc), 8, 1);
	adaptiveFocusFilter(pSrc, depthImage, 15, refinedDepth);

	cvResize(refinedDepth, DstDepthMap);
//	DWORD Elapse5 = GetTickCount()-start;

	cvReleaseImage(&refinedDepth);
	cvReleaseImage(&pSrc);
	cvReleaseImage(&pWavelet);
	//cvReleaseImage(&pImage);
	cvReleaseImage(&waveletImg);


	cvReleaseImage(&depthTemp);
	cvReleaseImage(&depthTemp1);
	cvReleaseImage(&depthMap);
	cvReleaseImage(&depthImage);

	cvReleaseImage(&temp);
	cvReleaseImage(&temp1);
//	DWORD Elapse6 = GetTickCount()-start;
//  	char *p = new char[200];
// //  	sprintf(p, "ep1: lu, ep2: lu, ep3: %lu, ep4: %lu, ep5: lu, ep6: lu\n", Elapse1, Elapse2, Elapse3, Elapse4, Elapse5, Elapse6);
// 	sprintf(p, "ep1: lu, ep2: lu, ep3: %lu, ep4: %lu, ep5: lu, ep6: lu\n", Elapse3, Elapse4);
// 
//  	FILE* hf = fopen("time.txt", "a+");
//  	fprintf(hf, p);
//  	fclose(hf);
	//OutputDebugString(p);
}

void CDefocus::wavelet2DColor(IplImage* src, int nLayer, IplImage* pWavelet)
{
	IplImage* pSrc = src;
	if (pWavelet && pSrc)
	{
		// 小波图象赋值
		cvSetImageROI(pWavelet, cvRect(0, 0, pSrc->width, pSrc->height));
		cvConvertScale(pSrc, pWavelet, 1, -128);
		cvResetImageROI(pWavelet);
		// 彩色图像小波变换
		IplImage *pImage = cvCreateImage(cvGetSize(pWavelet), IPL_DEPTH_32F, 1);
		if (pImage)
		{
			for (int i = 1; i <= pWavelet->nChannels; i++)
			{
				cvSetImageCOI(pWavelet, i);
				cvCopy(pWavelet, pImage, NULL);
				// 二维离散小波变换
				DWT(pImage, nLayer);
				// 二维离散小波恢复
				// IDWT(pImage, nLayer);
				cvCopy(pImage, pWavelet, NULL);
			}
			cvSetImageCOI(pWavelet, 0);
			cvReleaseImage(&pImage);
		}
		// 小波变换图象
		cvSetImageROI(pWavelet, cvRect(0, 0, pSrc->width, pSrc->height));
		cvConvertScale(pWavelet, pSrc, 1, 128);
		cvResetImageROI(pWavelet); // 本行代码有点多余，但有利用养成良好的编程习惯
		//cvReleaseImage(&pWavelet);
	}
	// 显示图像pSrc
#ifdef DISPLAYIMG
	cvNamedWindow("dwt",1);
	cvShowImage("dwt",pSrc);
	cvWaitKey(0);
	cvDestroyWindow("dwt");

#endif


}


void CDefocus::DWT(IplImage *pImage, int nLayer)
{
	// 执行条件
	if (pImage)
	{
		if (pImage->nChannels == 1 &&
			pImage->depth == IPL_DEPTH_32F &&
			((pImage->width >> nLayer) << nLayer) == pImage->width &&
			((pImage->height >> nLayer) << nLayer) == pImage->height)
		{
			int     i, x, y, n;
			float   fValue   = 0;
			float   fRadius  = sqrt(2.0f);
			int     nWidth   = pImage->width;
			int     nHeight  = pImage->height;
			int     nHalfW   = nWidth / 2;
			int     nHalfH   = nHeight / 2;
			float **pData    = new float*[pImage->height];
			float  *pRow     = new float[pImage->width];
			float  *pColumn  = new float[pImage->height];
			for (i = 0; i < pImage->height; i++)
			{
				pData[i] = (float*) (pImage->imageData + pImage->widthStep * i);
			}
			// 多层小波变换
			for (n = 0; n < nLayer; n++, nWidth /= 2, nHeight /= 2, nHalfW /= 2, nHalfH /= 2)
			{
				// 水平变换
				for (y = 0; y < nHeight; y++)
				{
					// 奇偶分离
					memcpy(pRow, pData[y], sizeof(float) * nWidth);
					for (i = 0; i < nHalfW; i++)
					{
						x = i * 2;
						pData[y][i] = pRow[x];
						pData[y][nHalfW + i] = pRow[x + 1];
					}
					// 提升小波变换
					for (i = 0; i < nHalfW - 1; i++)
					{
						fValue = (pData[y][i] + pData[y][i + 1]) / 2;
						pData[y][nHalfW + i] -= fValue;
					}
					fValue = (pData[y][nHalfW - 1] + pData[y][nHalfW - 2]) / 2;
					pData[y][nWidth - 1] -= fValue;
					fValue = (pData[y][nHalfW] + pData[y][nHalfW + 1]) / 4;
					pData[y][0] += fValue;
					for (i = 1; i < nHalfW; i++)
					{
						fValue = (pData[y][nHalfW + i] + pData[y][nHalfW + i - 1]) / 4;
						pData[y][i] += fValue;
					}
					// 频带系数
					for (i = 0; i < nHalfW; i++)
					{
						pData[y][i] *= fRadius;
						pData[y][nHalfW + i] /= fRadius;
					}
				}
				// 垂直变换
				for (x = 0; x < nWidth; x++)
				{
					// 奇偶分离
					for (i = 0; i < nHalfH; i++)
					{
						y = i * 2;
						pColumn[i] = pData[y][x];
						pColumn[nHalfH + i] = pData[y + 1][x];
					}
					for (i = 0; i < nHeight; i++)
					{
						pData[i][x] = pColumn[i];
					}
					// 提升小波变换
					for (i = 0; i < nHalfH - 1; i++)
					{
						fValue = (pData[i][x] + pData[i + 1][x]) / 2;
						pData[nHalfH + i][x] -= fValue;
					}
					fValue = (pData[nHalfH - 1][x] + pData[nHalfH - 2][x]) / 2;
					pData[nHeight - 1][x] -= fValue;
					fValue = (pData[nHalfH][x] + pData[nHalfH + 1][x]) / 4;
					pData[0][x] += fValue;
					for (i = 1; i < nHalfH; i++)
					{
						fValue = (pData[nHalfH + i][x] + pData[nHalfH + i - 1][x]) / 4;
						pData[i][x] += fValue;
					}
					// 频带系数
					for (i = 0; i < nHalfH; i++)
					{
						pData[i][x] *= fRadius;
						pData[nHalfH + i][x] /= fRadius;
					}
				}
			}
			delete[] pData;
			delete[] pRow;
			delete[] pColumn;
		}
	}
}



void CDefocus::adaptiveFocusFilter(IplImage* srcImg, IplImage* srcDepthImage, int winSize, IplImage* dstDepthImage)
{
	IplImage* srcP = srcImg;
	IplImage* depthP = srcDepthImage;

	int flag = -1;
	if (srcImg->width > srcDepthImage->width)
	{
		srcDepthImage = cvCreateImage(cvGetSize(srcImg), srcDepthImage->depth, srcDepthImage->nChannels);
		cvResize( depthP, srcDepthImage, CV_INTER_LINEAR );
		flag = 0;
	}
	if (srcImg->width<srcDepthImage->width)
	{
		srcImg = cvCreateImage(cvGetSize(srcDepthImage), srcImg->depth, srcImg->nChannels);
		cvResize( srcP, srcImg, CV_INTER_LINEAR );
		flag = 1;
	}
	
	IplImage* tempSrc = NULL;
	IplImage* tempDepth = NULL;

	if (srcImg)
	{
		tempSrc = cvCreateImage(cvSize(srcImg->width+winSize-1, srcImg->height+winSize-1), srcImg->depth, srcImg->nChannels);
		imageExtending(srcImg, winSize, tempSrc);
#if 0
		cvNamedWindow("extSrc");
		cvShowImage("extSrc", tempSrc);
#endif

	}

	if (!srcDepthImage)
	{
		return;
	}
	else
	{
		tempDepth = cvCreateImage(cvSize(srcImg->width+winSize-1, srcImg->height+winSize-1), srcDepthImage->depth, srcDepthImage->nChannels);
		imageExtending(srcDepthImage, winSize, tempDepth);
#ifdef DISPLAYIMG
		cvNamedWindow("extDepth");
		cvShowImage("extDepth", tempDepth);
#endif
	}

	CvMat* DistWeightMat = cvCreateMat(winSize,winSize, CV_32FC1);
	CvMat* ClrWeightMat = cvCreateMat(winSize,winSize, CV_32FC1);
	CvMat* GraWeightMat = cvCreateMat(winSize,winSize, CV_32FC1);
	CvMat* totalWeightMat = cvCreateMat(winSize, winSize, CV_32FC1);

	CvMat* refinedDepthMap = cvCreateMat(dstDepthImage->height, dstDepthImage->width, CV_32FC1);
	float* pData = refinedDepthMap->data.fl;

	float distSensParam = 0.2;
	float clrSensParam = 2;
	float graSensParam = 0.5;
	createDistanceWeightMat(winSize, DistWeightMat,distSensParam);

	CvRect roiRect;
	roiRect.width = winSize;
	roiRect.height = winSize;
	CvMat roiM, *roiMat;
	int step = winSize * 0.5;
	CvMat* tempMat = cvCreateMat(winSize, winSize, CV_32FC1);
	for (int y=0; y<srcImg->height; y++)
	{
		if (y==319)
		{
			int a=0;
		}
		roiRect.y = y;
		for (int x=0; x<srcImg->width; x++)
		{
			roiRect.x = x;
			roiMat = cvGetSubRect(tempSrc, &roiM, roiRect);
			computeColorSimiWeightMat(winSize, roiMat, tempSrc->nChannels, ClrWeightMat, clrSensParam);
			roiMat = cvGetSubRect(tempDepth, &roiM, roiRect);
			computeGradientWeightMat(winSize, roiMat, GraWeightMat, graSensParam);

			cvAdd(ClrWeightMat, DistWeightMat, totalWeightMat);
			cvAdd(GraWeightMat, totalWeightMat, totalWeightMat);
			
			double minv, maxv;
			cvMinMaxLoc(totalWeightMat, &minv, &maxv);

			cvConvertScale(roiMat, tempMat, 1, 0);
			cvMul(tempMat, totalWeightMat, totalWeightMat, 1.0/3);
			CvScalar sum = cvSum(totalWeightMat);
			pData[x] = sum.val[0];
		}
		pData += srcImg->width;
		cout<<"line "<<y<<endl;
	}

	double maxv, minv;
	cvMinMaxLoc(refinedDepthMap, &minv, &maxv);

	cvConvertScale(refinedDepthMap, dstDepthImage, 255.0/(maxv-minv), -minv/(float)(maxv-minv));
	cvReleaseMat(&refinedDepthMap);

	cvReleaseMat(&tempMat);
	cvReleaseMat(&DistWeightMat);
	cvReleaseMat(&ClrWeightMat);
	cvReleaseMat(&GraWeightMat);
	cvReleaseMat(&totalWeightMat);

#if 0
	cvNamedWindow("weightedDepth");
	cvShowImage("weightedDepth", dstDepthImage);

#endif
			
	
	if (flag == 0)
	{
		cvReleaseImage(&srcDepthImage);
	}
	if (flag == 1)
	{
		cvReleaseImage(&srcImg);
	}
	srcImg = srcP;
	srcDepthImage = depthP;

	if (tempDepth)
	{
		cvReleaseImage(&tempDepth);
	}
	if (tempSrc)
	{
		cvReleaseImage(&tempSrc);
	}

}


//li
void CDefocus::createDistanceWeightMat(int winSize, CvMat* weightMat/*cv_32fc1*/, float sensitivetyParam)
{
	int cx =  0.5*winSize;
	int cy = 0.5*winSize;

	float* pdata = weightMat->data.fl;
	for (int y=0; y<winSize; y++)
	{
		for (int x=0; x<winSize; x++)
		{
			float d = sqrt( (float) ( (x-cx)*(x-cx) + (y-cy)*(y-cy) ) );
			pdata[x] =  1.0/ pow((1 + d),sensitivetyParam);
		}
		pdata += winSize;
	}

	CvScalar sum = cvSum(weightMat);
	cvConvertScale(weightMat, weightMat, 1.0/sum.val[0], 0);
	

}

void CDefocus::computeColorSimiWeightMat(int winSize, CvMat* srcMat, int srcChannel, CvMat* clrWeightMat, float sensitivetyParam)
{
	int matType;
	if (srcChannel == 1)
	{
		matType = CV_32FC1;
	}
	if (srcChannel == 3)
	{
		matType = CV_32FC3;
	}
	CvMat* tempMat = cvCreateMat(srcMat->rows, srcMat->cols, matType);
	cvConvertScale(srcMat, tempMat, 1, 0);

	CvScalar c_pixel = cvGet2D(tempMat, 0.5*winSize, 0.5*winSize);
	//CvScalar avg = cvAvg(tempMat);

	//cvSubS(tempMat, avg, tempMat);
	cvSubS(tempMat, c_pixel, tempMat);
	cvAbs(tempMat, tempMat);

	CvMat** tempPlane  = new CvMat*[srcChannel];
	for (int i=0; i<srcChannel; i++)
	{
		tempPlane[i] = cvCreateMat(srcMat->rows, srcMat->cols, CV_32FC1);
	}
	if (srcChannel == 1)
	{
		cvSplit(tempMat,tempPlane[0],0, 0, 0);
	}
	if (srcChannel == 3)
	{
		cvSplit(tempMat,tempPlane[0],tempPlane[1],tempPlane[2], 0);
	}

	cvSetZero(clrWeightMat);
	for (int i=0; i<srcChannel; i++)
	{
		cvAdd(tempPlane[i], clrWeightMat, clrWeightMat);
	}
	

	CvMat* identityMat = cvCreateMat(clrWeightMat->rows, clrWeightMat->cols, CV_32FC1);
	cvSet(identityMat, cvScalar(1));
	
	cvAdd(clrWeightMat, identityMat, clrWeightMat);
	cvPow(clrWeightMat, clrWeightMat, sensitivetyParam);

	cvDiv(identityMat, clrWeightMat, clrWeightMat, 1);

	
	CvScalar sum = cvSum( clrWeightMat );
    if (sum.val[0] != 0)
    {
		cvConvertScale(clrWeightMat, clrWeightMat, 1.0/sum.val[0], 0);
    }
	else
		//cvSet(clrWeightMat, cvScalar(1.0/(winSize*winSize)), 0);
		cvSetZero(clrWeightMat);

	for (int i=0; i<srcChannel; i++)
	{
		cvReleaseMat(&tempPlane[i]);
	}
	delete tempPlane;
	cvReleaseMat(&tempMat);
	cvReleaseMat(&identityMat);
		

}

void CDefocus::computeGradientWeightMat(int winSize, CvMat* srcMat, CvMat* graWeightMat, float sensitivetyParam)
{
	CvMat* temp = cvCreateMat(srcMat->rows, srcMat->cols, CV_32FC1);
	cvConvertScale(srcMat, temp, 1, 0);
	CvScalar c_pixel = cvGet2D(srcMat, 0.5*winSize, 0.5*winSize);
	cvSubS(temp, c_pixel, graWeightMat);
	cvAbs(graWeightMat, graWeightMat);

	CvScalar sum = cvSum(graWeightMat);
	CvMat* idenMat = cvCloneMat(temp);
	cvSet(idenMat, cvScalar(1));
	if(sum.val[0]!=0)
	{
		cvConvertScale(graWeightMat, graWeightMat, 1.0/sum.val[0], 0);
	}

	//cvAdd(graWeightMat,idenMat, graWeightMat);
	cvSubRS(graWeightMat, cvScalar(2), graWeightMat);
	cvPow(graWeightMat, graWeightMat, sensitivetyParam);
	
	cvDiv(idenMat, graWeightMat, graWeightMat, 1);


	
	cvReleaseMat(&temp);
	cvReleaseMat(&idenMat);

/*
	CvScalar sum = cvSum(srcMat);
	if (sum.val[0]!=0)
	{
		cvConvertScale(srcMat, graWeightMat, 1.0/sum.val[0], 0);	
	}
	else
		//cvSet(graWeightMat, cvScalar(1.0/(winSize*winSize)), 0);
		cvSetZero(graWeightMat);
*/
}



void CDefocus::imageExtending(IplImage* src, int winSize, IplImage* dst)
{
	CvRect r;
	r.x = 0.5*winSize;
	r.y = 0.5*winSize;
	r.height =src->height;
	r.width = src->width;

	CvMat *mat, m;
	mat = cvGetSubRect(dst, &m, r);
	cvCopy(src, mat);

	CvRect rt;
	rt.x = 0;
	rt.y = 0;
	rt.height = 0.5*winSize;
	rt.width = 0.5*winSize;

	CvRect rt1;
	CvMat *mat1, m1;

	//ltc
	rt1.x = 0;
	rt1.y = 0;
	rt1.height = 0.5*winSize;
	rt1.width = 0.5*winSize;
	mat = cvGetSubRect(dst,&m, rt);
	mat1 = cvGetSubRect(src, &m1, rt1);
	cvCopy(mat1, mat);
	cvFlip(mat, 0, -1);

	//rtc
	rt.x = dst->width - rt.width;
	rt1.x = src->width - rt1.width;
	mat = cvGetSubRect(dst,&m, rt);
	mat1 = cvGetSubRect(src, &m1, rt1);
	cvCopy(mat1, mat);
	cvFlip(mat, 0, -1);

	//rbc
	rt.y = dst->height - rt.height;
	rt1.y = src->height - rt1.height;
	mat = cvGetSubRect(dst,&m, rt);
	mat1 = cvGetSubRect(src, &m1, rt1);
	cvCopy(mat1, mat);
	cvFlip(mat, 0, -1);

	//lbc
	rt.x = 0;
	rt1.x = 0;
	mat = cvGetSubRect(dst,&m, rt);
	mat1 = cvGetSubRect(src, &m1, rt1);
	cvCopy(mat1, mat);
	cvFlip(mat, 0, -1);


	//tb
	rt.x = 0.5*winSize;
	rt.y = 0;
	rt.width = src->width;
	rt.height = 0.5*winSize;

	rt1.x = 0;
	rt1.y = 0;
	rt1.width = src->width;
	rt1.height = 0.5*winSize;
	mat = cvGetSubRect(dst,&m, rt);
	mat1 = cvGetSubRect(src, &m1, rt1);
	cvCopy(mat1, mat);
	cvFlip(mat, 0, 0);
	
	
	//bb
	rt.y = dst->height - rt.height;
	rt1.y = src->height - rt1.height;
	mat = cvGetSubRect(dst,&m, rt);
	mat1 = cvGetSubRect(src, &m1, rt1);
	cvCopy(mat1, mat);
	cvFlip(mat, 0, 0);


	//lb
	rt.x = 0;
	rt.y = 0.5*winSize;
	rt.width = 0.5*winSize;
	rt.height = src->height;

	rt1.x = 0;
	rt1.y = 0;
	rt1.width = 0.5*winSize;
	rt1.height = src->height;
	mat = cvGetSubRect(dst,&m, rt);
	mat1 = cvGetSubRect(src, &m1, rt1);
	cvCopy(mat1, mat);
	cvFlip(mat, 0, 1);

	//rb
	rt.x = dst->width - rt.width;
	rt1.x = src->width - rt1.width;
	mat = cvGetSubRect(dst,&m, rt);
	mat1 = cvGetSubRect(src, &m1, rt1);
	cvCopy(mat1, mat);
	cvFlip(mat, 0, 1);

}

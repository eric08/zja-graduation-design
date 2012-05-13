#include "stdafx.h"
#include "PhotometricCues.h"
using namespace std;

CPhotometricCues::CPhotometricCues(void)
{
}

CPhotometricCues::~CPhotometricCues(void)
{
}
void CPhotometricCues::Haze_patch(IplImage* imgSrc, IplImage* imgHaze)
{
	int h = imgSrc->height;
	int w = imgSrc->width;
	//IplImage* darkChn = cvCreateImage(cvGetSize(imgSrc), imgSrc->depth, 1);
	DarkChannel_patch(imgSrc, imgHaze);
}
void CPhotometricCues::HazeDepth_patch(IplImage* imgSrc, IplImage* imgDst)
{
	int h = imgSrc->height;
	int w = imgSrc->width;
	IplImage* darkChn = cvCreateImage(cvGetSize(imgSrc), imgSrc->depth, 1);
	DarkChannel_patch(imgSrc, darkChn);

#if 0
	cvNamedWindow("darkChannel");
	cvShowImage("darkChannel", darkChn);
	//cvSaveImage("photometric/haze/darkChn.jpg", darkChn);
	//cvWaitKey(0);
	//cvDestroyWindow("darkChannel");
#endif

	////to normalize the dark channel by atomospheric intensity
	double atoIntenVal = 0;
	estiAtomospheric(imgSrc, darkChn, &atoIntenVal);
	//double a = 0;
	//a = 1.0 / atoIntenVal;
	CvMat* normalizedDch = cvCreateMat( h, w, CV_32FC1);
	cvConvertScale(darkChn, normalizedDch, 1.0/atoIntenVal, 0);

	//compute the transmap  t= 1-dark/A
	CvMat* transMap = cvCreateMat( h, w, CV_32FC1 );
	cvSet(transMap, cvScalar(1), 0);
	cvSub(transMap, normalizedDch, transMap, 0);

	//compute the depth according to the transmap  d= -beta*ln(t)
	CvMat* depthMap = cvCreateMat( h, w, CV_32FC1 );
	cvLog( transMap, depthMap);
	cvConvertScale( depthMap, depthMap, -1, 0);

	//display the depth map
	double maxv=0, minv=0;
	cvMinMaxLoc(depthMap, &minv, &maxv);
	//double scale = 1.0 / (maxv-minv);
	//double shift = - minv * scale;
	//CvMat* tempMap = cvCreateMat( h, w, CV_32FC1 );
	cvConvertScale( depthMap, imgDst, 255.0 / (maxv-minv), - minv * 255.0 / (maxv-minv) );



	cvReleaseImage(&darkChn);
	cvReleaseMat(&normalizedDch);
	cvReleaseMat(&transMap);
	cvReleaseMat(&depthMap);

}
void CPhotometricCues::HazeDepth(IplImage* imgSrc, IplImage* imgDst)
{
	int h = imgSrc->height;
	int w = imgSrc->width;
	IplImage* darkChn = cvCreateImage(cvGetSize(imgSrc), imgSrc->depth, 1);
	DarkChannel_patch(imgSrc, darkChn);

#if 1
	cvNamedWindow("darkChannel");
	cvShowImage("darkChannel", darkChn);
	cvSaveImage("photometric/haze/darkChn.jpg", darkChn);
	cvWaitKey(0);
	cvDestroyWindow("darkChannel");
#endif

	////to normalize the dark channel by atomospheric intensity
	double atoIntenVal = 0;
	estiAtomospheric(imgSrc, darkChn, &atoIntenVal);
	//double a = 0;
	//a = 1.0 / atoIntenVal;
	CvMat* normalizedDch = cvCreateMat( h, w, CV_32FC1);
	cvConvertScale(darkChn, normalizedDch, 1.0/atoIntenVal, 0);

	//compute the transmap  t= 1-dark/A
	CvMat* transMap = cvCreateMat( h, w, CV_32FC1 );
	cvSet(transMap, cvScalar(1), 0);
	cvSub(transMap, normalizedDch, transMap, 0);

	//refine the transmap by considering the neighborhood and image
	CvMat* refinedTransMap = cvCreateMat( h, w, CV_32FC1 );
	cvSetZero(refinedTransMap);
	refineResult(imgSrc, transMap, refinedTransMap);
	//double maxv1, minv1;
	//cvMinMaxLoc(refinedTransMap, &minv1, &maxv1);


	//compute the depth according to the transmap  d= -beta*ln(t)
	CvMat* depthMap = cvCreateMat( h, w, CV_32FC1 );
	cvLog( refinedTransMap, depthMap);
	cvConvertScale( depthMap, depthMap, -1, 0);

	//display the depth map
	double maxv=0, minv=0;
	cvMinMaxLoc(depthMap, &minv, &maxv);
	//double scale = 1.0 / (maxv-minv);
	//double shift = - minv * scale;
	//CvMat* tempMap = cvCreateMat( h, w, CV_32FC1 );
	cvConvertScale( depthMap, imgDst, 255.0 / (maxv-minv), - minv * 255.0 / (maxv-minv) );



	cvReleaseImage(&darkChn);
	cvReleaseMat(&normalizedDch);
	cvReleaseMat(&transMap);

}
void CPhotometricCues::estiAtomospheric(IplImage* src, IplImage* darkChn, double* atomosVal)
{
	int h=src->height;
	int w=src->width;
	int pixelNum = h*w;
	float atomosPixelNumThreshold = 0.001;
	int maxPixelNum = pixelNum * atomosPixelNumThreshold;
	double maxv=0;
	cvMinMaxLoc(darkChn, 0, &maxv);

	CvPoint* pointTemp = new CvPoint[maxPixelNum];
	IplImage* grayImage = cvCreateImage(cvGetSize(src), src->depth, 1);
	cvCvtColor(src, grayImage, CV_BGR2GRAY);

	CvMat m, *mat;
	CvMat* sortMat = cvCreateMat(1, pixelNum, CV_8UC1);
	CvMat* idxMat = cvCreateMat(1, pixelNum, CV_32SC1);
	mat = cvReshape(darkChn, &m, 0, 1);
	cvSort(mat, sortMat, idxMat, CV_SORT_DESCENDING);

	uchar* dValue = (uchar*) sortMat->data.ptr;
	int* idValue = idxMat->data.i;
	maxv = 0;
	for (int i=0; i<maxPixelNum; i++)
	{
		pointTemp[i].x = idValue[i] % w;
		pointTemp[i].y = idValue[i] / w;
		uchar intensity = grayImage->imageData[ pointTemp[i].y*w + pointTemp[i].x ];
		maxv = max<double>( maxv, intensity );
	}

	*atomosVal= maxv;

	delete [] pointTemp;
	cvReleaseMat(&sortMat);
	cvReleaseMat(&idxMat);
	cvReleaseImage(&grayImage);

}
void CPhotometricCues::DarkChannel_patch(IplImage* src, IplImage* darkChn)
{
	int nchannel = src->nChannels;
	if (nchannel!=3)
	{
		return;
	}

	int h = src->height;
	int w = src->width;

	//CvMat* bch = cvCreateMat(h, w, CV_8UC1);
	//CvMat* gch = cvCreateMat(h, w, CV_8UC1);
	//CvMat* rch = cvCreateMat(h, w, CV_8UC1);
	IplImage* bch = cvCreateImage(cvSize(w, h), 8, 1);
	IplImage* gch = cvCreateImage(cvSize(w, h), 8, 1);
	IplImage* rch = cvCreateImage(cvSize(w, h), 8, 1);

	cvSplit(src,bch, gch, rch, 0);

	CvRect patch;
	patch.height = HAZE_PATCH_SIZE;
	patch.width = HAZE_PATCH_SIZE;
	int half_size = HAZE_PATCH_SIZE / 2;
	double minb = 0;
	double ming = 0; 
	double minr = 0;
	CvMat mb, mg, mr;

	uchar* darkValue = (uchar*)darkChn->imageData;
	for (int y=0; y<h; y++)
	{
		patch.y = max(0, y-half_size);
		if (y>=h-half_size)
		{
			patch.height = HAZE_PATCH_SIZE - (y-(h-half_size)+1);
		}
		for (int x=0; x<w; x++)
		{
			patch.x = max(0, x-half_size);
			if (x>=w-half_size)
			{
				patch.width = HAZE_PATCH_SIZE - (x-(w-half_size)+1);
			}
			cvMinMaxLoc( cvGetSubRect(bch, &mb, patch), &minb, 0 );
			cvMinMaxLoc( cvGetSubRect(gch, &mg, patch), &ming, 0 );
			cvMinMaxLoc( cvGetSubRect(rch, &mr, patch), &minr, 0 );
			darkValue[x] = (uchar)( min( minb, min(ming, minr) ) );
		}
		darkValue += darkChn->widthStep;
	}

	cvReleaseImage(&bch);
	cvReleaseImage(&gch);
	cvReleaseImage(&rch);

// 	cvShowImage("darkchn", darkChn);
// 	cvSaveImage("darkchn.bmp", darkChn);
// 	cvWaitKey();
}

void CPhotometricCues::refineResult(IplImage* src, CvMat* iniTrans, CvMat* refinedTrans)
{
	int h=src->height;
	int w=src->width;
	int N = h*w;

	//uchar* largeMat = new uchar[N*N];

	//cv::Mat m(N, N, CV_32FC1);

	CvMat* lapMatrix = cvCreateMat(N, N, CV_32FC1);
	calHazeLaplacian(src, lapMatrix);
	COptBox opt;


	CvMat* iniTVec = cvCreateMat(N, 1, CV_32FC1);
	cvReshape(iniTrans, iniTVec, 0, N);
	CvMat* refinedVec = cvCreateMat(N, 1, CV_32FC1);
	cvSetZero(refinedVec);

	opt.sovleLinearEqu( lapMatrix, iniTVec, refinedVec);
	cvReleaseMat(&lapMatrix);
	cvReleaseMat(&iniTVec);


	cvReshape(refinedVec, refinedTrans, 0, h);
	cvReleaseMat(&refinedVec);

}

void CPhotometricCues::calHazeLaplacian(IplImage* src, CvMat* LapMatrix)
{
	//lapMatrix is a N X N matrix


	int h = src->height;
	int w = src->width;
	int N = h * w;

	//if (LapMatrix==NULL)
	//{
	//	LapMatrix = cvCreateMat(N, N, CV_32FC1);
	//}
	//else
	//if ( LapMatrix->cols != LapMatrix->rows || LapMatrix->cols != N || LapMatrix->rows != N )
	//{
	//	cvReleaseMat(&LapMatrix);
	//	LapMatrix = cvCreateMat(N, N, CV_32FC1);
	//}

	cvZero(LapMatrix);
	float* LData = LapMatrix->data.fl;
	for (int i=0; i<N; i++)
	{
		LData[i*N + i] = LAMDA;
	}


	int localWindowSize = 3;

	CvRect lpRect;
	lpRect.height = localWindowSize;
	lpRect.width = localWindowSize;
	int half_h = localWindowSize / 2;
	int half_w = localWindowSize / 2;

	CvMat* convar = cvCreateMat(3, 3, CV_32FC1);

	CvMat* reguMat = cvCreateMat(3, 3, CV_32FC1);
	CvMat* zeroMat = cvCreateMat(3, 3, CV_32FC1);

	float epsilon = 0.0000001;
	float scaleFac = 1.0/(localWindowSize*localWindowSize);
	cvSetIdentity(reguMat);
	cvZero(zeroMat);
	cvAddWeighted(reguMat, epsilon*scaleFac, zeroMat, 0, 0, reguMat);

	CvMat* srcMat = cvCreateMat(h, w, CV_32FC3);
	cvConvertScale(src, srcMat);

	CvMat* pixelMatI = cvCreateMat(1, 3, CV_32FC1);
	CvMat* pixelMatJ = cvCreateMat(3, 1, CV_32FC1);
	
	CvMat subm, *subMat;
	CvScalar meanVector;
	CvMat* tmpMat = cvCreateMat(localWindowSize, localWindowSize, CV_32FC3);
	CvMat* tmpMatB = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	CvMat* tmpMatG = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	CvMat* tmpMatR = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	CvMat* tmpMatMean = cvCreateMat(localWindowSize, localWindowSize, CV_32FC3);
	CvScalar tmpMatMeanVector;

	CvMat* tmpMatBB = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	CvMat* tmpMatGG = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	CvMat* tmpMatRR = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	CvMat* tmpMatBG = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	CvMat* tmpMatBR = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	CvMat* tmpMatGR = cvCreateMat(localWindowSize, localWindowSize, CV_32FC1);
	
	CvMat* ProductTmp1 = cvCreateMat(1, 3, CV_32FC1);
	CvMat* ProductTmp2 = cvCreateMat(1, 1, CV_32FC1);

	for (int y=half_h; y<h-half_h-1; y++)
	{
		lpRect.y = y-half_h;
		for (int x=half_w; x<w-half_w-1; x++)
		{
			lpRect.x = x-half_w;
			subMat = cvGetSubRect( srcMat, &subm, lpRect );
			meanVector =  cvAvg(subMat);

			cvSubS(subMat, meanVector, tmpMat);
			cvSplit(tmpMat, tmpMatB, tmpMatG, tmpMatR, 0);

			cvMul(tmpMatB, tmpMatB, tmpMatBB);
			cvMul(tmpMatG, tmpMatG, tmpMatGG);
			cvMul(tmpMatR, tmpMatR, tmpMatRR);
			cvMul(tmpMatB, tmpMatG, tmpMatBG);
			cvMul(tmpMatB, tmpMatR, tmpMatBR);
			cvMul(tmpMatG, tmpMatR, tmpMatGR);

			CvScalar bb = cvAvg(tmpMatBB);
			CvScalar gg = cvAvg(tmpMatGG);
			CvScalar rr = cvAvg(tmpMatRR);
			CvScalar bg = cvAvg(tmpMatBG);
			CvScalar br = cvAvg(tmpMatBR);
			CvScalar gr = cvAvg(tmpMatGR);

			convar->data.fl[0] = bb.val[0];
			convar->data.fl[1] = bg.val[0];
			convar->data.fl[2] = br.val[0];
			convar->data.fl[3] = bg.val[0];
			convar->data.fl[4] = gg.val[0];
			convar->data.fl[5] = gr.val[0];
			convar->data.fl[6] = br.val[0];
			convar->data.fl[7] = gr.val[0];
			convar->data.fl[8] = rr.val[0];

			cvAdd(convar, reguMat, convar);
			cvInvert(convar, convar);

			int wpIdx = y*w + x;

			for (int mi=0; mi<lpRect.height; mi++)
			{				
				for (int ni=0; ni<lpRect.width; ni++)
				{
					pixelMatI->data.fl[0] = tmpMatB->data.fl[mi*lpRect.width + ni];
					pixelMatI->data.fl[1] = tmpMatG->data.fl[mi*lpRect.width + ni + 1];
					pixelMatI->data.fl[2] = tmpMatR->data.fl[mi*lpRect.width + ni + 2];

					cvMatMul(pixelMatI, convar, ProductTmp1); // 1*3 * 3*3

					int idxI = wpIdx + mi*w + ni;
					
					for (int mj=0; mj<lpRect.height; mj++)
					{				
						for (int nj=0; nj<lpRect.width; nj++)
						{
							pixelMatJ->data.fl[0] = tmpMatB->data.fl[mj*lpRect.width + nj];	
							pixelMatJ->data.fl[1] = tmpMatG->data.fl[mj*lpRect.width + nj + 1];	
							pixelMatJ->data.fl[2] = tmpMatR->data.fl[mj*lpRect.width + nj + 2];	
							
							int idxJ = wpIdx + mj*w + nj;

							cvMatMul(ProductTmp1, pixelMatJ, ProductTmp2); // 1*3 * 3*1

							float val = 1 + ProductTmp2->data.fl[0];
							val = (idxI==idxJ) - val * scaleFac;

							LData[idxI*N + idxJ] += val;  //sum all the possible windows around i and j
							//LData[idxI*N + idxJ] = val; 

						}
					}
					
				}
			}
						
		}
	}

	cvReleaseMat(&convar);
	cvReleaseMat(&reguMat);
	cvReleaseMat(&zeroMat);
	cvReleaseMat(&srcMat);
	cvReleaseMat(&pixelMatI);
	cvReleaseMat(&pixelMatJ);
	cvReleaseMat(&tmpMat);
	cvReleaseMat(&tmpMatB);
	cvReleaseMat(&tmpMatG);
	cvReleaseMat(&tmpMatR);
	cvReleaseMat(&tmpMatMean);
	cvReleaseMat(&tmpMatBB);
	cvReleaseMat(&tmpMatGG);
	cvReleaseMat(&tmpMatRR);
	cvReleaseMat(&tmpMatBG);
	cvReleaseMat(&tmpMatBR);
	cvReleaseMat(&tmpMatGR);
	cvReleaseMat(&ProductTmp1);
	cvReleaseMat(&ProductTmp2);
}

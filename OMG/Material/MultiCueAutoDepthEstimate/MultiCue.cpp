#include "stdafx.h"
#include "MultiCue.h"
//#include "ulti\Video.h"
//#include "..\DepthFromHaze.h"
#include "..\DepthFromHaze\DepthFromHaze.h"
using namespace std;

CMultiCue::CMultiCue(void)
{
}

CMultiCue::~CMultiCue(void)
{
}

// void CMultiCue::runExample_video(char* videoPath, char* strVideoPath)
// {
// 	runExample_video(videoPath, strVideoPath, 1.0, 22);
// }
// void CMultiCue::runExample_video(char* videoPath, char* strVideoPath, float scale, int deviceSize)
// {
// 	CVideo monoVideo;
// 	CvCapture* cap = monoVideo.ReadVideoCapture(videoPath);
// 	IplImage* srcFrame = NULL;
// 	IplImage* frame = NULL;
// 	IplImage* finalStr = cvCreateImage(cvSize(1080, 720), 8, 3);
// 	bool err = false;
// 	if (cap == NULL)
// 	{
// 		cout<<"ERROR: 视频文件读取失败！"<<endl;
// 		err = true;
// 		return;
// 	}
// 
// 	cvNamedWindow("src");
// 	cvNamedWindow("ana");
// 	cvNamedWindow("depth");
// 	cvNamedWindow("disparity");
// 
// 	IplImage* depthImg = NULL;
// 	IplImage* dispImg = NULL;
// 	CvMat* disp = NULL;
// 	CvMat* depth = NULL;
// 	IplImage* stereoImg = NULL;
// 	IplImage* anagView = NULL;
// 
// 	int frameId = 0;
// 	int h, w;
// 	CvVideoWriter* stereoV = NULL;
// 
// 	CvMat* tmpDisp = NULL;
// 	while (srcFrame = cvQueryFrame(cap))
// 	{
// 		if (frameId>0)
// 		{
// 			cvResize(srcFrame, frame, CV_INTER_LINEAR);
// 		}
// 		else if (frameId==0)
// 		{
// 			frame = cvCreateImage( cvSize(srcFrame->width*scale, srcFrame->height*scale), 8, 3 );
// 			cvResize(srcFrame, frame, CV_INTER_LINEAR);
// 			h = frame->height;
// 			w = frame->width;
// 			depthImg = cvCreateImage(cvGetSize(frame), 8, 3);
// 			dispImg = cvCreateImage(cvGetSize(frame), 8, 3);
// 			disp = cvCreateMat(h, w, CV_32FC1);
// 			depth = cvCreateMat(h, w, CV_32FC1);
// 			
// 			stereoImg = cvCreateImage( cvSize(srcFrame->width*2, srcFrame->height), 8, 3 );
// 			stereoV = cvCreateVideoWriter( strVideoPath, CV_FOURCC('X','V', 'I', 'D'), 24, cvGetSize(finalStr), 1);
// 			tmpDisp = cvCreateMat(srcFrame->height, srcFrame->width, CV_32FC1);
// 			anagView = cvCreateImage( cvGetSize(srcFrame), 8, 3 );
// 			
// 		}
// 		multiCueFusion(frame, 0.25, 0.5, disp, depth);
// 		depthVisualization(depth, depthImg);
// 		
// 
// 
// 		
// 		cvResize(disp, tmpDisp, CV_INTER_CUBIC);
// 		cvConvertScale(tmpDisp, tmpDisp, 1);//1/scale);
// 		cvSmooth(tmpDisp, tmpDisp, CV_GAUSSIAN, 13, 13, 7, 0);
// 
// 		disparityVisualization(tmpDisp, dispImg);
// 		cvShowImage("depth", depthImg);
// 		cvShowImage("disparity", dispImg);
// 
// 		To3D_new(srcFrame, tmpDisp, stereoImg, deviceSize);
// 		cvResize(stereoImg, finalStr);
// 		//To3D(frame, disp, stereoImg);
// 		showAnaglyphy(stereoImg, anagView);
// 		cvWriteFrame(stereoV, finalStr);
// 
// 		cvShowImage("src", frame);
// 		cvShowImage("ana", anagView);
// 		cvWaitKey(1);
// 		frameId++;
// 		/*if ( frameId>150)
// 		{
// 			break;
// 		}*/
// 	}
// 
// 	cvReleaseMat(&disp);
// 	cvReleaseMat(&depth);
// 	cvReleaseImage(&depthImg);
// 	cvReleaseImage(&dispImg);
// 	cvReleaseVideoWriter(&stereoV);
// 	
// }

void CMultiCue::runExample_img(IplImage* src)
{
	int h = src->height;
	int w = src->width;
	CvMat* disp = cvCreateMat(h, w, CV_32FC1);
	CvMat* depth = cvCreateMat(h, w, CV_32FC1);

	multiCueFusion(src, 0.125, 1.0, disp, depth);

	IplImage* depthImg = cvCreateImage(cvGetSize(src), 8, 3);
	IplImage* dispImg = cvCreateImage(cvGetSize(src), 8, 3);

	depthVisualization(depth, depthImg);
	disparityVisualization(disp, dispImg);

	cvNamedWindow("depth");
	cvShowImage("depth", depthImg);
	cvNamedWindow("disparity");
	cvShowImage("disparity", dispImg);

	IplImage* likeliImg = cvCreateImage(cvGetSize(src), 8, 3);
	cvAddWeighted(src, 0.3, dispImg, 0.7, 0, likeliImg);
	cvNamedWindow("likeli");
	cvShowImage("likeli", likeliImg);
	cvNamedWindow("src");
	cvShowImage("src", src);

	IplImage* stereoImg = cvCreateImage( cvSize(depthImg->width*2, depthImg->height), 8, 3 );

	To3D(src, disp, stereoImg);

	cvNamedWindow("str");
	cvShowImage("str", stereoImg);
	cvSaveImage("str.jpg", stereoImg);

	IplImage* anagView = cvCreateImage( cvGetSize(src), 8, 3 );
	showAnaglyphy(stereoImg, anagView);
	cvNamedWindow("ana");
	cvShowImage("ana", anagView);

	cvWaitKey(0);
	cvReleaseMat(&disp);
	cvReleaseMat(&depth);
	cvReleaseImage(&depthImg);
	cvReleaseImage(&dispImg);
	cvReleaseImage(&likeliImg);
	cvReleaseImage(&stereoImg);
}

void CMultiCue::ProcOneFrame(IplImage* srcFrm, IplImage* depthImg, IplImage* dispImg, IplImage* likeliImg, IplImage*stereoImg)
{
	int h = srcFrm->height;
	int w = srcFrm->width;
	CvMat* disp = cvCreateMat(h, w, CV_32FC1);
	CvMat* depth = cvCreateMat(h, w, CV_32FC1);

	multiCueFusion(srcFrm, disp, depth);

	depthVisualization(depth, depthImg);
	disparityVisualization(disp, dispImg);

	cvAddWeighted(srcFrm, 0.3, dispImg, 0.7, 0, likeliImg);

	cvReleaseMat(&disp);
	cvReleaseMat(&depth);
}

void CMultiCue::multiCueFusion(IplImage* src, float scale_focus, float scale_haze, CvMat* disp, CvMat* depth)
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////only for test
// #define DISPLAYIMG
// 	//////////////////////////////////////////////////////////////////////////
//  	{//previous depth
//  		IplImage* psrc_h = cvCreateImage(cvSize(src->width, src->height), 8, 3);
// 		cvCopy(src, psrc_h);
//  		//cvResize(src, psrc_h, CV_INTER_LINEAR); 
//  		CPhotometricCues photo;
//  
//  		IplImage* darkDepth = cvCreateImage(cvGetSize(psrc_h), 8, 1);
//  		photo.DarkChannel_patch(psrc_h, darkDepth);
//  
//  		CvMat* tempMat = cvCreateMat(src->height, src->width, CV_32FC1);
//  
//  		CvMat* nrmDark = cvCreateMat(darkDepth->height, darkDepth->width, CV_32FC1);
//  		double minv, maxv;
//  
//  // 		cvMinMaxLoc(darkDepth, &minv, &maxv);
//  // 		cvSubRS(darkDepth, cvScalar(maxv),darkDepth);
//  		cvMinMaxLoc(darkDepth, &minv, &maxv);
//  		cvConvertScale(darkDepth, nrmDark, 1.0/(maxv-minv), -minv/(maxv-minv)); //normalize to 0~1  ~ 1/depth
//  
//  		cv::Mat TempSave(darkDepth->height, darkDepth->width, CV_8UC1);
//  		((cv::Mat)(nrmDark)).convertTo(TempSave, TempSave.type(), 255);
//  		imwrite("PrevDepth.bmp", TempSave);
//  		TempSave.release();
//  
//  		cvReleaseImage(&psrc_h);
//  		cvReleaseMat(&tempMat);
//  		cvReleaseMat(&nrmDark);
//  	}
// 	//////////////////////////////////////////////////////////////////////////
// 	//
// 	CDepthFromHaze dfh;
// 	cv::Mat DepthMap(src->height, src->width, CV_32FC1);
// 	dfh.CalculateDepthFromHaze(src, DepthMap);
// 
// 	{
// 		cv::Mat TempSave(DepthMap.rows, DepthMap.cols, CV_8UC1);
// 		(DepthMap).convertTo(TempSave, TempSave.type(), 255);
// 		//TempSave = 255 - TempSave;
// 		imwrite("CurDepth.bmp", TempSave);
// 		TempSave.release();
// 	}
// 
// 	cvCopy(&(static_cast<CvMat>(DepthMap)), depth);	//the normalized depth
// 
// 	if(disp!=NULL)
// 		depthToDisparity(depth, disp);

// 	cvReleaseMat(&tempMat);
// 	cvReleaseMat(&nrmDark);
// 	cvReleaseMat(&nrmFocus);
// 
// #ifdef DISPLAYIMG
// 	cvNamedWindow("d_dark");
// 	cvShowImage("d_dark", darkDepth);
// 	cvNamedWindow("d_focus");
// 	cvShowImage("d_focus", focusDepth);
// 	cvWaitKey(0);
// #endif
// 	cvReleaseImage(&darkDepth);
// 	cvReleaseImage(&focusDepth);
// 	cvReleaseImage(&psrc_f);
// 	cvReleaseImage(&psrc_h);
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

// #define DISPLAYIMG
 	IplImage* psrc_f = cvCreateImage(cvSize(src->width*scale_focus, src->height*scale_focus), 8, 3);
 	IplImage* psrc_h = cvCreateImage(cvSize(src->width*scale_haze, src->height*scale_haze), 8, 3);
 
 	cvResize(src, psrc_f, CV_INTER_LINEAR);
 	cvResize(src, psrc_h, CV_INTER_LINEAR);
 
 	CPhotometricCues photo;
 
 	IplImage* darkDepth = cvCreateImage(cvGetSize(psrc_h), 8, 1);
 	photo.DarkChannel_patch(psrc_h, darkDepth);
 
 	CDefocus df;
 	IplImage* focusDepth = cvCreateImage(cvGetSize(psrc_f), 8, 1);
 #ifdef DISPLAYIMG
 	cvShowImage("psrc-f", psrc_f);
 #endif
 	df.depthFromDefocus(psrc_f, focusDepth);
 
 	CvMat* tempMat = cvCreateMat(src->height, src->width, CV_32FC1);
 
 	CvMat* nrmDark = cvCreateMat(darkDepth->height, darkDepth->width, CV_32FC1);
 	CvMat* nrmFocus = cvCreateMat(focusDepth->height, focusDepth->width, CV_32FC1);
 	
 
 	double minv, maxv;
 	cvMinMaxLoc(darkDepth, &minv, &maxv);
 	cvConvertScale(darkDepth, nrmDark, 1.0/(maxv-minv), -minv/(maxv-minv)); //normalize to 0~1  ~ 1/depth
 
	cvMinMaxLoc(focusDepth, &minv, &maxv);
	cvSubRS(focusDepth, cvScalar(maxv),focusDepth);
 	cvMinMaxLoc(focusDepth, &minv, &maxv);
 	cvConvertScale(focusDepth, nrmFocus, 1.0/(maxv-minv), -minv/(maxv-minv)); //normalize to 0~1  ~ 1/depth
 
 
 	//fusing the dark and focus cues
 	fuse_dark_focus(nrmDark, nrmFocus, tempMat);
 
 	cvCopy(tempMat, depth);	//the normalized depth
 
 	if(disp!=NULL)
 		depthToDisparity(depth, disp);
 
 	cvReleaseMat(&tempMat);
 	cvReleaseMat(&nrmDark);
 	cvReleaseMat(&nrmFocus);
 
 #ifdef DISPLAYIMG
 	cvNamedWindow("d_dark");
 	cvShowImage("d_dark", darkDepth);
 	cvNamedWindow("d_focus");
 	cvShowImage("d_focus", focusDepth);
 	cvWaitKey(0);
 #endif
 	cvReleaseImage(&darkDepth);
 	cvReleaseImage(&focusDepth);
 	cvReleaseImage(&psrc_f);
 	cvReleaseImage(&psrc_h);
}
void CMultiCue::multiCueFusion(IplImage* src, CvMat* disp, CvMat* depth)
{
	CPhotometricCues photo;

	IplImage* darkDepth = cvCreateImage(cvGetSize(src), 8, 1);
	photo.DarkChannel_patch(src, darkDepth);

	CDefocus df;
	IplImage* focusDepth = cvCreateImage(cvGetSize(src), 8, 1);
	df.depthFromDefocus(src, focusDepth);

	CvMat* tempMat = cvCreateMat(src->height, src->width, CV_32FC1);

	CvMat* nrmDark = cvCreateMat(darkDepth->height, darkDepth->width, CV_32FC1);
	CvMat* nrmFocus = cvCreateMat(darkDepth->height, darkDepth->width, CV_32FC1);
	double minv, maxv;
	
	cvMinMaxLoc(darkDepth, &minv, &maxv);
	cvSubRS(darkDepth, cvScalar(maxv),darkDepth);
	cvMinMaxLoc(darkDepth, &minv, &maxv);
	cvConvertScale(darkDepth, nrmDark, 1.0/(maxv-minv), -minv/(maxv-minv)); //normalize to 0~1  ~ 1/depth

	cvMinMaxLoc(focusDepth, &minv, &maxv);
	cvConvertScale(focusDepth, nrmFocus, 1.0/(maxv-minv), -minv/(maxv-minv)); //normalize to 0~1  ~ 1/depth
	

	//fusing the dark and focus cues
	fuse_dark_focus(nrmDark, nrmFocus, tempMat);
	
	cvCopy(tempMat, depth);	//the normalized depth

	depthToDisparity(depth, disp);

	cvReleaseMat(&tempMat);
	cvReleaseMat(&nrmDark);
	cvReleaseMat(&nrmFocus);

#if 1
	cvNamedWindow("d_dark");
	cvShowImage("d_dark", darkDepth);
	cvNamedWindow("d_focus");
	cvShowImage("d_focus", focusDepth);
	//cvWaitKey(0);
#endif
	cvReleaseImage(&darkDepth);
	cvReleaseImage(&focusDepth);
}

void CMultiCue::multiCueFusionWithSoftMatting( IplImage* src, float scale_focus, float scale_haze, CvMat* disp, CvMat* depth )
{
	using namespace cv;
	//depth from haze;
	Mat ScaledOrig;
	resize(src, ScaledOrig, Size(src->width*scale_haze, src->height*scale_haze));
	Mat ScaledDep(ScaledOrig.rows, ScaledOrig.cols, CV_32FC1);

	CDepthFromHaze dfh;
	dfh.CalculateDepthFromHaze(ScaledOrig, ScaledDep);

	cvResize(&(static_cast<CvMat>(ScaledDep)), depth);
	ScaledOrig.release();
	ScaledDep.release();

//  	IplImage* psrc_f = cvCreateImage(cvSize(src->width*scale_focus, src->height*scale_focus), 8, 3);
// 	cvResize(src, psrc_f, CV_INTER_LINEAR);
// 	CDefocus df;
// 	IplImage* focusDepth = cvCreateImage(cvGetSize(psrc_f), 8, 1);
// #ifdef DISPLAYIMG
// 	cvShowImage("psrc-f", psrc_f);
// #endif
// 	df.depthFromDefocus(psrc_f, focusDepth);
// 
// 	CvMat* tempMat = cvCreateMat(src->height, src->width, CV_32FC1);
// 
// 	CvMat* nrmDark = cvCreateMat(darkDepth->height, darkDepth->width, CV_32FC1);
// 	CvMat* nrmFocus = cvCreateMat(focusDepth->height, focusDepth->width, CV_32FC1);
// 	double minv, maxv;
// 
// 	cvMinMaxLoc(darkDepth, &minv, &maxv);
// 	cvSubRS(darkDepth, cvScalar(maxv),darkDepth);
// 	cvMinMaxLoc(darkDepth, &minv, &maxv);
// 	cvConvertScale(darkDepth, nrmDark, 1.0/(maxv-minv), -minv/(maxv-minv)); //normalize to 0~1  ~ 1/depth
// 
// 	cvMinMaxLoc(focusDepth, &minv, &maxv);
// 	cvConvertScale(focusDepth, nrmFocus, 1.0/(maxv-minv), -minv/(maxv-minv)); //normalize to 0~1  ~ 1/depth
// 
// 
// 	//fusing the dark and focus cues
// 	fuse_dark_focus(nrmDark, nrmFocus, tempMat);
// 
// 	cvCopy(tempMat, depth);	//the normalized depth
// 
// 	if(disp!=NULL)
// 		depthToDisparity(depth, disp);
// 
// 	cvReleaseMat(&tempMat);
// 	cvReleaseMat(&nrmDark);
// 	cvReleaseMat(&nrmFocus);
// 
// #ifdef DISPLAYIMG
// 	cvNamedWindow("d_dark");
// 	cvShowImage("d_dark", darkDepth);
// 	cvNamedWindow("d_focus");
// 	cvShowImage("d_focus", focusDepth);
// 	cvWaitKey(0);
// #endif
// 	cvReleaseImage(&darkDepth);
// 	cvReleaseImage(&focusDepth);
// 	cvReleaseImage(&psrc_f);
// 	cvReleaseImage(&psrc_h);
}

void CMultiCue::fuse_dark_focus(IplImage* darkMat, IplImage* focusMat, CvMat* depthMat)
{
	CvMat m1, m2;
	
	fuse_dark_focus(cvGetMat(darkMat,&m1), cvGetMat(focusMat,&m2), depthMat);
}

void CMultiCue::fuse_dark_focus(CvMat* darkMat, CvMat* focusMat, CvMat* depthMat)
{
	//weighted sum
   	float darkW = 0.7;
   	float focusW = 0.3;

//  	float darkW = 0.5;
//  	float focusW = 0.5;

	CvMat* darkTmp = cvCreateMat(depthMat->rows, depthMat->cols, CV_32FC1);
	CvMat* focusTmp = cvCreateMat(depthMat->rows, depthMat->cols, CV_32FC1);
	cvResize(darkMat, darkTmp, CV_INTER_LINEAR);
	cvResize(focusMat, focusTmp, CV_INTER_LINEAR);

	cvAddWeighted(darkTmp, darkW, focusTmp, focusW, 0, depthMat);
	cvReleaseMat(&darkTmp);
	cvReleaseMat(&focusTmp);

}

void CMultiCue::fuse_df_weighted(CvMat* darkMat, CvMat* focusMat, CvMat* depthMat)
{

}

void CMultiCue::depthToDisparity(CvMat* depth, CvMat* disp)
{

	double minv, maxv;
	cvMinMaxLoc(depth, &minv, &maxv);

	CvMat* temp = cvCreateMat(depth->rows, depth->cols, cvGetElemType(depth));

	cvSubRS(depth, cvScalar(maxv), temp);
	cvMinMaxLoc(temp, &minv, &maxv);
	double cvg = minv + (maxv-minv)*0.3;

	cvSubS(temp, cvScalar(cvg), disp);
	cvReleaseMat(&temp);

}

void CMultiCue::depthVisualization(CvMat* depth, IplImage* depthImg)
{
	cvSetZero(depthImg);
	cv::Mat temp(depth->rows, depth->cols, CV_8UC1);
	((cv::Mat)(depth)).convertTo(temp, temp.type(), 255);
	//cvSetImageCOI(depthImg, 3);

// 	double minv, maxv;
// 	cvMinMaxLoc(depth, &minv, &maxv);
// 
// 	CvMat* temp = cvCreateMat(depth->rows, depth->cols, CV_8UC1);
// 	cvConvertScale(depth, temp, 255.0/(maxv-minv),-255.0*minv/(maxv-minv));

// 	CvMat* tempWhite = cvCreateMat(depth->rows, depth->cols, CV_8UC1);
// 	cvSet(tempWhite, cvScalar(255));
// 
// 	cvSub(tempWhite, temp, temp);
	cvCopy(&CvMat(temp), depthImg);
	temp.release();

	//cvSetImageCOI(depthImg, 0);
#ifdef DISPLAYIMG
	//cvShowImage("depthimg", depthImg);
#endif
	//cvReleaseMat(&temp);
	//cvReleaseMat(&tempWhite);
}

void CMultiCue::disparityVisualization(CvMat* disp, IplImage* dispImg)
{
	//cvSetZero(dispImg);
	//cvSet(dispImg, cvScalar(255));
	double minv, maxv;
	cvMinMaxLoc(disp, &minv, &maxv);
	float* dispV = disp->data.fl;
	uchar* data = (uchar*) dispImg->imageData;
	float scale = 255.0/(maxv-minv);
	for (int i=0; i<dispImg->height; i++)
	{
		for (int j=0; j<dispImg->width; j++)
		{
			if (dispV[j]>0)
			{
				data[3*j] = fabs((dispV[j]-maxv)*scale);
				data[3*j+1] = 0;
				data[3*j+2] = 0;
			}
			else if (dispV[j]<0)
			{
				data[3*j] = 0;
				data[3*j+1] = 0;
				data[3*j+2] = (uchar)255 - ( fabs( (dispV[j]-minv)*scale ));
			}
			else
			{
				data[3*j] = 0;
				data[3*j+1] = 255;
				data[3*j+2] = 0;
			}

			//data[3*j+1] = (uchar) ( fabs( 255 - dispV[j]*scale ) );
		}
		dispV+= dispImg->width;
		data += dispImg->width*3;
	}
}
void CMultiCue::To3D(IplImage* srcView, CvMat* srcDisp, IplImage* stereoView, int deviceSize)
{

	float s = 0.02;
	if (deviceSize<15)
	{
		s = 0.05;
	}
	else if (deviceSize<35)
	{
		s = 0.04;
	}
	else if (deviceSize<75)
	{
		s = 0.02;
	}
	else
	{
		s = 0.01;
	}


	float scale = srcView->width * s * 0.5;

	uchar* src = (uchar*)srcView->imageData;
	CvRect rectL, rectR;
	CvMat ml, mr, *matL, *matR;

	rectL.x = 0;
	rectL.y = 0;
	rectL.width = stereoView->width / 2;
	rectL.height = stereoView->height;
	rectR.x = rectL.width;
	rectR.y = 0;
	rectR.width = stereoView->width / 2;
	rectR.height = stereoView->height;

	matL = cvGetSubRect(stereoView, &ml, rectL);
	matR = cvGetSubRect(stereoView, &mr, rectR);

	uchar* dstL = (uchar*) matL->data.ptr;
	uchar* dstR = (uchar*) matR->data.ptr;
	int w = srcView->width;
	int h = srcView->height;

	CvMat* tempMat = cvCreateMat(srcDisp->rows, srcDisp->cols, CV_32FC1);

	

	cvConvertScale(srcDisp, tempMat, scale);


	float* dis = tempMat->data.fl;

	//cvNamedWindow("tst");
	for (int i=0; i<h; i++)
	{
		for (int j=0; j<w; j++)
		{
			int xl = abs( (int) (j+dis[j]) );
			int xr = abs( (int) (j-dis[j]) );

			if (xl>=w)
			{
				xl = j-dis[j];
			}
			if (xr>=w)
			{
				xr = j+dis[j];
			}

			dstL[ 3*j ] = src[ 3*xl ];
			dstL[ 3*j+1 ] = src[ 3*xl+1 ];
			dstL[ 3*j+2 ] = src[ 3*xl+2 ];
			dstR[ 3*j ] = src[ 3*xr ];
			dstR[ 3*j+1 ] = src[ 3*xr+1 ];
			dstR[ 3*j+2 ] = src[ 3*xr+2 ];
		}
		src += 3*w;
		dstL += 3*w*2;
		dstR += 3*w*2;
		dis += w;
		//std::cout<<"line: "<<i<<" of "<<h<<endl;
		//cvShowImage("tst", stereoView);
		//cvWaitKey(10);
	}
	//cvWaitKey(0);



	cvReleaseMat(&tempMat);
}
void CMultiCue::To3D(IplImage* srcView, CvMat* srcDisp, IplImage* stereoView)
{
	uchar* src = (uchar*)srcView->imageData;
	CvRect rectL, rectR;
	CvMat ml, mr, *matL, *matR;

	rectL.x = 0;
	rectL.y = 0;
	rectL.width = stereoView->width / 2;
	rectL.height = stereoView->height;
	rectR.x = rectL.width;
	rectR.y = 0;
	rectR.width = stereoView->width / 2;
	rectR.height = stereoView->height;

	matL = cvGetSubRect(stereoView, &ml, rectL);
	matR = cvGetSubRect(stereoView, &mr, rectR);

	uchar* dstL = (uchar*) matL->data.ptr;
	uchar* dstR = (uchar*) matR->data.ptr;
	int w = srcView->width;
	int h = srcView->height;

	CvMat* tempMat = cvCreateMat(srcDisp->rows, srcDisp->cols, CV_32FC1);

	float scale = srcView->width * 0.04 * 0.5;

	cvConvertScale(srcDisp, tempMat, scale);


	float* dis = tempMat->data.fl;

	//cvNamedWindow("tst");
	for (int i=0; i<h; i++)
	{
		for (int j=0; j<w; j++)
		{
			int xl = abs( (int) (j+dis[j]) );
			int xr = abs( (int) (j-dis[j]) );

			if (xl>=w)
			{
				xl = j-dis[j];
			}
			if (xr>=w)
			{
				xr = j+dis[j];
			}

			dstL[ 3*j ] = src[ 3*xl ];
			dstL[ 3*j+1 ] = src[ 3*xl+1 ];
			dstL[ 3*j+2 ] = src[ 3*xl+2 ];
			dstR[ 3*j ] = src[ 3*xr ];
			dstR[ 3*j+1 ] = src[ 3*xr+1 ];
			dstR[ 3*j+2 ] = src[ 3*xr+2 ];
		}
		src += 3*w;
		dstL += 3*w*2;
		dstR += 3*w*2;
		dis += w;
		//std::cout<<"line: "<<i<<" of "<<h<<endl;
		//cvShowImage("tst", stereoView);
		//cvWaitKey(10);
	}
	//cvWaitKey(0);

	

	cvReleaseMat(&tempMat);

}

void CMultiCue::To3D_new(IplImage* srcView, CvMat* srcDisp, IplImage* stereoView, int deviceSize)
{
	float s = 0.02;
	if (deviceSize<15)
	{
		//s = 0.1;
		s = 0.05;
	}
	else if (deviceSize<35)
	{
		s = 0.04;
	}
	else if (deviceSize<75)
	{
		s = 0.02;
	}
	else
	{
		s = 0.01;
	}

	cvSetZero(stereoView);
	uchar* src = (uchar*)srcView->imageData;
	CvRect rectL, rectR;
	CvMat ml, mr, *matL, *matR;

	rectL.x = 0;
	rectL.y = 0;
	rectL.width = stereoView->width / 2;
	rectL.height = stereoView->height;
	rectR.x = rectL.width;
	rectR.y = 0;
	rectR.width = stereoView->width / 2;
	rectR.height = stereoView->height;

	matL = cvGetSubRect(stereoView, &ml, rectL);
	matR = cvGetSubRect(stereoView, &mr, rectR);

	uchar* dstL = (uchar*) matL->data.ptr;
	uchar* dstR = (uchar*) matR->data.ptr;
	int w = srcView->width;
	int h = srcView->height;

	CvMat* tempMat = cvCreateMat(srcDisp->rows, srcDisp->cols, CV_32FC1);

	float scale = srcView->width * s * 0.5;

	cvConvertScale(srcDisp, tempMat, scale);


	float* dis = tempMat->data.fl;

	//cvNamedWindow("tst");
	int xl_pre = 0;
	int xr_pre = 0;
	int dis_pre = 0;
	CvMat* fillMat_l = cvCreateMat(1, srcDisp->cols, CV_8UC1);
	uchar* isFill_l = (uchar*) fillMat_l->data.ptr;
	CvMat* fillMat_r = cvCreateMat(1, srcDisp->cols, CV_8UC1);
	uchar* isFill_r = (uchar*) fillMat_r->data.ptr;
	for (int i=0; i<h; i++)
	{
		cvSetZero(fillMat_l);
		cvSetZero(fillMat_r);
		for (int j=0; j<w; j++)
		{
			if ((int)dis[j]>w-1||(int)dis[j]<1-w)
			{
				dis[j] = 0;
			}
			int xl = abs( (int) (j-dis[j]) );
			int xr = abs( (int) (j+dis[j]) );

			if (xl>=w)
			{
				xl = j+dis[j];
			}
			if (xr>=w)
			{
				xr = j-dis[j];
			}



			{
				dstL[ 3*xl ] = src[ 3*j ];
				dstL[ 3*xl+1 ] = src[ 3*j+1 ];
				dstL[ 3*xl+2 ] = src[ 3*j+2 ];
			}

			{
				dstR[ 3*xr ] = src[ 3*j ];
				dstR[ 3*xr+1 ] = src[ 3*j+1 ];
				dstR[ 3*xr+2 ] = src[ 3*j+2 ];
			}

			isFill_l[xl]=1;
			isFill_r[xr]=1;


			xl_pre = xl;
			xr_pre = xr;
			dis_pre = dis[j];
		}
		for (int j=0; j<w; j++)
		{
			if (isFill_l[j]==0)
			{
				dstL[ 3*j ]   = dstL[ 3*((j-1)<0?0:(j-1)) ];
				dstL[ 3*j+1 ] = dstL[ 3*((j-1)<0?0:(j-1))+1 ];
				dstL[ 3*j+2 ] = dstL[ 3*((j-1)<0?0:(j-1))+2 ];
			}
			if (isFill_r[w-1-j]==0)
			{
				dstR[ 3*(w-1-j) ]   = dstR[ 3*( (w-1-j+1)>(w-1) ? (w-1) : (w-1-j+1) ) ];
				dstR[ 3*(w-1-j)+1 ] = dstR[ 3*( (w-1-j+1)>(w-1) ? (w-1) : (w-1-j+1) ) +1 ];
				dstR[ 3*(w-1-j)+2 ] = dstR[ 3*( (w-1-j+1)>(w-1) ? (w-1) : (w-1-j+1) ) +2 ];
			}
		}
		src += 3*w;
		dstL += 3*w*2;
		dstR += 3*w*2;
		dis += w;
		//std::cout<<"line: "<<i<<" of "<<h<<endl;
		//cvShowImage("tst", stereoView);
		//cvWaitKey(10);
	}
	//cvWaitKey(0);



	cvReleaseMat(&tempMat);
	cvReleaseMat(&fillMat_l);
	cvReleaseMat(&fillMat_r);

}

void CMultiCue::showAnaglyphy(IplImage* lview, IplImage* rview, IplImage* anagView)
{
	int h=lview->height;
	int w=lview->width;
	uchar* ldata = (uchar*) lview->imageData;
	uchar* rdata = (uchar*) rview->imageData;
	uchar* angData = (uchar*) anagView->imageData;
	for (int i=0; i<h; i++)
	{
		for (int j=0; j<w; j++)
		{
			angData[3*j] = rdata[3*j];
			angData[3*j+1] = rdata[3*j+1];
			angData[3*j+2] = ldata[3*j+2];
		}
		ldata += 3*w;
		rdata += 3*w;
		angData += 3*w;
	}
}
void CMultiCue::showAnaglyphy(IplImage* strView, IplImage* anagView)
{
	IplImage* lview = cvCreateImage(cvGetSize(anagView), 8, 3);
	IplImage* rview = cvCreateImage(cvGetSize(anagView), 8, 3);
 
	CvMat ml, mr, *matl, *matr;
	CvRect r;
	r.x = 0;
	r.y = 0;
	r.width = anagView->width;
	r.height = anagView->height;
	matl =  cvGetSubRect( strView, &ml, r );
	r.x = r.width;
	matr = cvGetSubRect( strView, &mr, r);
	cvCopy(matl, lview);
	cvCopy(matr, rview);

	showAnaglyphy(lview, rview, anagView);
	cvReleaseImage(&lview);
	cvReleaseImage(&rview);

}

void CMultiCue::To3D_Synthesis(IplImage* srcView, CvMat* srcDisp, IplImage* stereoView)
{

	uchar* src = (uchar*)srcView->imageData;
	CvRect rectL, rectR;
	CvMat ml, mr, *matL, *matR;

	rectL.x = 0;
	rectL.y = 0;
	rectL.width = stereoView->width / 2;
	rectL.height = stereoView->height;
	rectR.x = rectL.width;
	rectR.y = 0;
	rectR.width = stereoView->width / 2;
	rectR.height = stereoView->height;

	matL = cvGetSubRect(stereoView, &ml, rectL);
	matR = cvGetSubRect(stereoView, &mr, rectR);

	uchar* dstL = (uchar*) matL->data.ptr;
	uchar* dstR = (uchar*) matR->data.ptr;
	int w = srcView->width;
	int h = srcView->height;

	CvMat* tempMat = cvCreateMat(srcDisp->rows, srcDisp->cols, CV_32FC1);

	float scale = srcView->width * 0.5;

	cvConvertScale(srcDisp, tempMat, scale);


	float* dis = tempMat->data.fl;

	CvMat* flagL = cvCreateMat(srcDisp->rows, srcDisp->cols, CV_8UC1);
	CvMat* flagR = cvCreateMat(srcDisp->rows, srcDisp->cols, CV_8UC1);
	cvSetZero(flagL);
	cvSetZero(flagR);
	uchar* flagLData = (uchar*)flagL->data.ptr;
	uchar* flagRData = (uchar*)flagR->data.ptr;
	CvMat* dispL = cvCreateMat(srcDisp->rows, srcDisp->cols, CV_32FC1);
	CvMat* dispR = cvCreateMat(srcDisp->rows, srcDisp->cols, CV_32FC1);
	float* dispLData = dispL->data.fl;
	float* dispRData = dispR->data.fl;
	cvSet(dispL, cvScalar(-10000));
	cvSet(dispR, cvScalar(-10000));


	//cvNamedWindow("tst");

	for (int i=0; i<h; i++)
	{
		for (int j=0; j<w; j++)
		{
			int xl = abs( (int) (j-dis[j]) );
			int xr = abs( (int) (j+dis[j]) );

			if (xl>=0 && xl <w  )
			{
				if (!flagLData[xl])
				{
					dstL[ 3*xl ] = src[ 3*j ];
					dstL[ 3*xl+1 ] = src[ 3*j+1 ];
					dstL[ 3*xl+2 ] = src[ 3*j+2 ];
					flagLData[xl] = 255;
					dispLData[xl] = dis[j];
				}
				else if(dis[j]>dispLData[xl])
				{
					dstL[ 3*xl ] = src[ 3*j ];
					dstL[ 3*xl+1 ] = src[ 3*j+1 ];
					dstL[ 3*xl+2 ] = src[ 3*j+2 ];
					dispLData[xl] = dis[j];
				}
			}

			if (xr>=0 && xr <w)
			{
				if (!flagRData[xr])
				{
					dstR[ 3*xr ] = src[ 3*j ];
					dstR[ 3*xr+1 ] = src[ 3*j+1 ];
					dstR[ 3*xr+2 ] = src[ 3*j+2 ];
					flagRData[xr] = 255;
					dispRData[xr] = dis[j];
				}
				else if (dis[j]>dispRData[xr])
				{
					dstR[ 3*xr ] = src[ 3*j ];
					dstR[ 3*xr+1 ] = src[ 3*j+1 ];
					dstR[ 3*xr+2 ] = src[ 3*j+2 ];
					dispRData[xr] = dis[j];
				}

			}
		}
		src += 3*w;
		dstL += 3*w*2;
		dstR += 3*w*2;
		dis += w;
		flagLData += w;
		flagRData += w;
		//std::cout<<"line: "<<i<<" of "<<h<<endl;
		//cvShowImage("tst", stereoView);
		//cvWaitKey(10);
	}
	//cvWaitKey(0);


	dispLData = dispL->data.fl;
	flagLData = flagL->data.ptr;
	dstL = (uchar*) matL->data.ptr;
	dispRData = dispR->data.fl;
	flagRData = flagR->data.ptr;
	dstR = (uchar*) matR->data.ptr;
	for (int i=0; i<h; i++)
	{
		float dispS = -10000;
		float dispE = -10000;
		int Sp = 0;
		int Ep = 0;

		for (int j=0; j<w; j++)
		{
			if (flagLData[j]==0)
			{
				if (j>0)
				{
					dispS = dispLData[j-1];
					Sp = j-1;
				}
				int offset = 1;
				while(flagLData[j+offset]==0 && j+offset<w-1)
				{
					offset++;
				}
				if ( j+offset < w-1)
				{
					dispE = dispLData[j+offset];
					Ep = j+offset;
					float disp;
					uchar pixel[3];
					if (dispE>dispS)
					{
						disp = dispE;
						pixel[0] = dstL[3*Ep];
						pixel[1] = dstL[3*Ep+1];
						pixel[2] = dstL[3*Ep+2];
					}
					else
					{
						disp = dispS;
						pixel[0] = dstL[3*Sp];
						pixel[1] = dstL[3*Sp+1];
						pixel[2] = dstL[3*Sp+2];
					}

					for (int k=j; k<Ep; k++)
					{
						dispLData[k] = disp;
						dstL[k*3] = pixel[0];
						dstL[k*3+1] = pixel[1];
						dstL[k*3+2] = pixel[2];
						flagLData[k] = 255;

					}
				}
				else if(j+offset==w-1)
				{
					for (int k=j; k<w; k++)
					{
						dispLData[k] = dispS;
						dstL[k*3] = dstL[3*Sp];
						dstL[k*3+1] = dstL[3*Sp];
						dstL[k*3+2] = dstL[3*Sp];
						flagLData[k] = 255;
					}
				}
			}
		}
		if (!flagLData[w-1])
		{
			int a=0;
		}
		for (int j=0; j<w; j++)
		{
			if (flagRData[j]==0)
			{
				if (j>0)
				{
					dispS = dispRData[j-1];
					Sp = j-1;
				}
				int offset = 1;
				while(flagRData[j+offset]==0 && j+offset<w-1)
				{
					offset++;
				}
				if ( j+offset < w-1)
				{
					dispE = dispRData[j+offset];
					Ep = j+offset;
					float disp;
					uchar pixel[3];
					if (dispE>dispS)
					{
						disp = dispE;
						pixel[0] = dstR[3*Ep];
						pixel[1] = dstR[3*Ep+1];
						pixel[2] = dstR[3*Ep+2];
					}
					else
					{
						disp = dispS;
						pixel[0] = dstR[3*Sp];
						pixel[1] = dstR[3*Sp+1];
						pixel[2] = dstR[3*Sp+2];
					}

					for (int k=j; k<Ep; k++)
					{
						dispRData[k] = disp;
						dstR[k*3] = pixel[0];
						dstR[k*3+1] = pixel[1];
						dstR[k*3+2] = pixel[2];
						flagRData[k] = 255;

					}
				}
				else if(j+offset==w-1)
				{
					for (int k=j; k<w; k++)
					{
						dispRData[k] = dispS;
						dstR[k*3] = dstR[3*Sp];
						dstR[k*3+1] = dstR[3*Sp];
						dstR[k*3+2] = dstR[3*Sp];
						flagRData[k] = 255;
					}
				}
			}

		}
		dstL += 3*w*2;
		dstR += 3*w*2;
		flagLData += w;
		dispLData += w;
		flagRData += w;
		dispRData += w;

	}

#if 1
	cvNamedWindow("maskL");
	cvShowImage("maskL", flagL);
	cvNamedWindow("maskR");
	cvShowImage("maskR", flagR);
#endif

	cvSubRS(flagL, cvScalar(255), flagL);
	cvSubRS(flagR, cvScalar(255), flagR);

	//cvWaitKey(0);
	//cvInpaint(matL, flagL, matL, 3, CV_INPAINT_TELEA);
	//cvInpaint(matR, flagR, matR, 3, CV_INPAINT_TELEA);


	cvReleaseMat(&flagL);
	cvReleaseMat(&flagR);
	cvReleaseMat(&dispL);
	cvReleaseMat(&dispR);

	cvReleaseMat(&tempMat);

}

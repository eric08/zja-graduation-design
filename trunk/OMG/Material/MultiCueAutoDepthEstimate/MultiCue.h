#pragma once
#include "opencv2/opencv.hpp"
//#include "ulti/ProcColor.h"
//#include "ulti/Overseg.h"
#include "Defocus.h"
#include "PhotometricCues.h"
#include "../Depth2Disparity/DepthToDisparity.h"


class CMultiCue
{
public:
	CMultiCue(void);
	~CMultiCue(void);

	void runExample_video(char* videoPath, char* strVideoPath);
	void runExample_video(char* videoPath, char* strVideoPath, float scale, int deviceSize);
	void runExample_img(IplImage* src);
	void ProcOneFrame(IplImage* srcFrm, IplImage* depthImg, IplImage* dispImg, IplImage* likiImg, IplImage*stereoImg);

	void depthVisualization(CvMat* depth, IplImage* depthImg);
	void disparityVisualization(CvMat* disp, IplImage* dispImg);


	void multiCueFusion(IplImage* src, CvMat* disp, CvMat* depth);
	void multiCueFusion(IplImage* src, float scale_focus, float scale_haze, CvMat* disp, CvMat* depth);
	void multiCueFusionWithSoftMatting(IplImage* src, float scale_focus, float scale_haze, CvMat* disp, CvMat* depth);
	void fuse_dark_focus(CvMat* darkMat, CvMat* focusMat, CvMat* depthMat);
	void fuse_dark_focus(IplImage* darkMat, IplImage* focusMat, CvMat* depthMat);

	void fuse_df_weighted(CvMat* darkMat, CvMat* focusMat, CvMat* depthMat);


	void depthToDisparity(CvMat* depth, CvMat* disp);
	void depthToDisparity(CvMat* depth, CvMat* disp, int deviceSize);

	void To3D(IplImage* srcView, CvMat* srcDisp, IplImage* stereoView);
	void To3D(IplImage* srcView, CvMat* srcDisp, IplImage* stereoView, int deviceSize);
	void To3D_new(IplImage* srcView, CvMat* srcDisp, IplImage* stereoView, int deviceSize);
	void To3D_Synthesis(IplImage* srcView, CvMat* srcDisp, IplImage* stereoView);
	
	void showAnaglyphy(IplImage* lview, IplImage* rview, IplImage* anagView);
	void showAnaglyphy(IplImage* strView, IplImage* anagView);
};

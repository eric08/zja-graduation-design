//extrct photometric cues from images
// 1 fog/haze
// 2 focus/defocus
// 3 BRDF
// 4 shadow
// 5 time variants


#pragma once
#include "opencv2/opencv.hpp"
#include "OptBox.h"

#define HAZE_PATCH_SIZE 15
#define LAMDA 0.0001

class CPhotometricCues
{
public:
	CPhotometricCues(void);
	~CPhotometricCues(void);


	///1 haze part: use hazing for depth
	void Haze_patch(IplImage* imgSrc, IplImage* imgHaze);
	void HazeDepth_patch(IplImage* imgSrc, IplImage* imgDst);
	void HazeDepth(IplImage* imgSrc, IplImage* imgDst);
	void DarkChannel_patch(IplImage* src, IplImage* darkChn);	//for haze
	void estiAtomospheric(IplImage* src, IplImage* darkChn, double* atomosVal);
	void calHazeLaplacian(IplImage* src, CvMat* LapMatrix);
	void refineResult(IplImage* src, CvMat* iniTrans, CvMat* refinedTrans);


	///2 focus part:
};

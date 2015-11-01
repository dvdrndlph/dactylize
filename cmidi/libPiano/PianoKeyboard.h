// PianoKeyboard.h: interface for the CPianoKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIANOKEYBOARD_H__3E3EC0BF_9D20_4758_A0C9_06AA655DD34B__INCLUDED_)
#define AFX_PIANOKEYBOARD_H__3E3EC0BF_9D20_4758_A0C9_06AA655DD34B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////ARJUN MAY 30, 2005
//#define IMAGE_WIDTH 160
//#define IMAGE_HEIGHT 120
//#define IMAGE_WIDTH 320
//#define IMAGE_HEIGHT 240

#ifndef PI
#define PI CV_PI // 3.1415926535
//#define PI 3.1415926535
#endif


#define BACKGROUND_HOUGH_THRESHOLD	35		//Hough Transform Threshold
#define BACKGROUND_EDGE_THRESHOLD	2		//Edge Detection Threshold
#define BACKGROUND_POLARIZE_THRESHOLD 60	//Threshold to Polarize Black and White Keys

#define BACKGROUND_KEYTRACE_THRESHOLD 100		//Threshold to determine the bottom of keyboard
#define KEY_COLOUR_DIFFERENCE_THRESHOLD 10	//Intensity Difference between Black and White Keys

#define SLOPE_ERROR 0.01	//Error in finding slopes

#include "../PVS/pvi-utility.h"
#include "ImageSize.h"

class CPianoKeyboard  
{
public:
	CPianoKeyboard();
	CPianoKeyboard(CIplImage *image);
	virtual ~CPianoKeyboard();

	void computeBlob(CIplImage *iplImage, CIplImage *m_imgBlob, int rootX, int rootY, int value, int *x1, int *y1, int *x2, int *y2);

	void PolarizeImage(IplImage *image, IplImage *dest, int threshold);

	void FindShapes(CIplImage *image);
	void BestLine(int *x, int *y, int totalPoints, float maxError, int *x1, int *y1, 
		int *x2, int *y2, int *elem);

	int BestSize(int *cornerY, int *bottomCornerY, int totalLines);

	void AnalyzeKeys(CIplImage *image);
	void KeyPattern(int *x, int *y, int totalPoints, float maxError, int *elemList, int *elemCount);
	int findTop(IplImage *image, int startX, int startY, int threshold);

	void ClearImage(IplImage *pImg);

	void UpdateOctave(int key, int octave);
	CvRect ReturnKeyBounds(int key, int octave);
	char SelectKey(int x, int y);

	void DrawBox(IplImage *pImg, int x1, int y1, int x2, int y2, int bpp);
	void DrawLine(IplImage *pImg, int x, int y, int x2, int y2, int bpp);

	void UpdateBackground(CIplImage *image, CIplImage *bg, CIplImage *mask);

	int FirstColourChange(CIplImage *image, int x, int y, int dx);

	CIplImage boxes;
	CIplImage polarizedImage;
	CIplImage bwImage;

	double m_dDegreeRotation;
	int m_nKeyboardY1, m_nKeyboardY2;
	int m_nOrigC_X1, m_nOrigC_X2;
	int m_nC_X1, m_nC_X2;
	int m_nBlackKeyLength;
	
	int m_nOctaveOffset;
};

#endif // !defined(AFX_PIANOKEYBOARD_H__3E3EC0BF_9D20_4758_A0C9_06AA655DD34B__INCLUDED_)

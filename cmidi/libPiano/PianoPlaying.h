// PianoPlaying.h: interface for the CPianoPlaying class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIANOPLAYING_H__DD73302F_5CB0_4F06_A0A3_4AAE32DB72E4__INCLUDED_)
#define AFX_PIANOPLAYING_H__DD73302F_5CB0_4F06_A0A3_4AAE32DB72E4__INCLUDED_


#include "../PVS/pvi-utility.h"
#include "stdio.h"

#include "pianokeyboard.h"
#include "pianohand.h"
#include "pianofinger.h"

#include "pointseries.h"
#include "ImageSize.h"

#include "../PVS/ColourChannel.h"


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

#define DETECT_WRITE 0
#define DETECT_READ 1


//IF using 320x240, set appropriate template coordinates
#if IMAGE_WIDTH == 320
	#define X1 30
	#define Y1 20
	#define X2 135
	#define Y2 100
#else
	#define X1 20
	#define Y1 20
	#define X2 70
	#define Y2 60
#endif

#define SHOW_TEMPLATE 1

#define BACKGROUND_HOUGH_THRESHOLD	35		//Hough Transform Threshold
#define BACKGROUND_EDGE_THRESHOLD	2		//Edge Detection Threshold
#define BACKGROUND_POLARIZE_THRESHOLD 60	//Threshold to Polarize Black and White Keys

//#define BACKGROUND_KEYTRACE_THRESHOLD 25		//Threshold to determine the bottom of keyboard
#define KEY_COLOUR_DIFFERENCE_THRESHOLD 10	//Intensity Difference between Black and White Keys
#define USE_CPIANOKEYBOARD	0




class CPianoPlaying  
{
public:
	CPianoPlaying();
	virtual ~CPianoPlaying();

////////////////////////////////FUNCTIONS////////////////////////////////

	bool updateImages(CIplImage *pImage);

	
	//Sets Background Image Permanently
	void setBackgroundImage(CIplImage *pImage, int nPianoHeight=PVS_DEFAULT);
	
	//Main Hand Processing Function
	void createHandsImage();
	
	//Checks if one colour is shade of another colour
	bool isShadow(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2, float threshold);

	//Detects Changes in Images, using Colour
	void detectChange_Colour(IplImage *pImg, IplImage *pImg1, IplImage *pImb_dI, int nThresh);

	//Calculates Average Colour of Difference Map
	//Also calculates Centre of Change (likely the Hand)
	void CalculateAverageColour(IplImage *pImg, IplImage *pImg1, 
											unsigned char *avgR, unsigned char *avgG, 
											unsigned char *avgB, int *avgX, int *avgY);
	
	//Locates all Average Colours, and adds them to Difference Map
	void AddAverageColour(IplImage *pImg, IplImage *pImg1, 
											unsigned char avgR, unsigned char avgG, 
											unsigned char avgB, int threshold);

	//Edge Detection Routines
	void DetectEdges(IplImage *pImg, IplImage *pImg1, int threshold);
	void DetectHorizontalEdges(IplImage *pImg, IplImage *pImg1, int threshold);
	void DetectVerticalEdges(IplImage *pImg, IplImage *pImg1, int threshold);

	//Rotates and Masks Images to appropriate ROI
	void AdjustImages(void);

	//Drawing Functions
	void DrawLineRad(IplImage *pImg, int x, float theta);
	void DrawLine(IplImage *pImg, int x, int y, int x2, int y2, int bpp);
	void DrawBlackLine(IplImage *pImg, int x, int y, int x2, int y2, int bpp);
	void DrawBox(IplImage *pImg, int x1, int y1, int x2, int y2, int bpp);
	void ClearImage(IplImage *pImg);

	//Finds Bounding Box around solid object
	void FindBoundingBox(IplImage *pImg, int origX, int origY, 
						int *x1, int *y1, int *x2, int *y2, int threshold);

	//Area Filling Functions
	void MaskRegion(IplImage *pImg, int x1, int y1, int x2, int y2);
	void ClearRegion(IplImage *pImg, int x1, int y1, int x2, int y2);

	//Locating Hands
	void LocateHands(CIplImage *image, CPianoHand *hands, int *numHands);
	
	//Which Hand does rect belong To
	int SelectHand(CvRect *rect);

	
	//General Purpose
	void computeBlob(CIplImage *iplImage, CIplImage *m_imgBlob, int rootX, int rootY, int value, int *x1, int *y1, int *x2, int *y2);

	
	///////////////////////////////VARIABLES//////////////////////////////////

	//Working Images
	CIplImage m_imc_backgroundImage; //Background Image
	CIplImage m_imb_handsImage;		 //Hands Only Image
	CIplImage m_imb_dI_colour;		 //Colour Difference Image

	CIplImage m_imb_edgeDetectedImage;
	CIplImage m_imb_fingerDetectedImage;

	CIplImage m_imc_displayImage;

	CIplImage m_img_tempImage;

	CIplImage m_img_bwImage;

	CIplImage m_imc_tempImage;
	CIplImage m_imc_tempImage2;

	CIplImage m_ORIMAGE;

	CColourChannel m_colourChannel;

	FILE *fDataFile;

	char keyString[100];	//Key Press String

	int m_nWidth, m_nHeight;

	CPianoKeyboard *m_Piano;	//Object that takes care of Keyboard orientation,
							//Location and Key detection
	
	CPianoHand	m_Hand[4];	//Object that represents each hand
	CPianoHand	m_tempHand[4];	//Object that represents each hand // change to m_handTemp

	CPianoFinger m_Fingers[20];
	CPianoFinger m_tempFingers[20];

	int m_nNumHands;
	int m_nTempNumHands;
	int m_nTempNumFingers;

	CPointSeries lines[20];
	int m_nNumLines;

	//CONTROL VALUES
	int m_nHandsDetectThreshold;//Background Subtraction Threshold
	bool m_bColourAverage;		//Colour Average Control
	int m_nColourThreshold;	//Colour Average Threshold
	bool m_bEdgeDetect;		//Edge Detection Control
	int m_nEdgeDetectThreshold;//Edge Detection Threshold
	bool m_bDetectHands;		//Hand Detection Control
	bool m_bFingerDetect;		//Finger Detection Control

	////////////////////////////END ARJUN'S VARIABLES//////////////////////////
};

#endif // !defined(AFX_PIANOPLAYING_H__DD73302F_5CB0_4F06_A0A3_4AAE32DB72E4__INCLUDED_)

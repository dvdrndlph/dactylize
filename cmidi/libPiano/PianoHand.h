// PianoHand.h: interface for the CPianoHand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIANOHAND_H__1F59F20B_7951_42CE_A418_A04C4A98C07A__INCLUDED_)
#define AFX_PIANOHAND_H__1F59F20B_7951_42CE_A418_A04C4A98C07A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../PVS/pvi-utility.h"
#include "ImageSize.h"
#include "pianofinger.h"
//#include "../ImageSize.h"

//#define IMAGE_WIDTH 160
//#define IMAGE_HEIGHT 120
//#define IMAGE_WIDTH 320
//#define IMAGE_HEIGHT 240

class CPianoHand  
{
public:
	CPianoHand();
	CPianoHand(int ID, int x1, int y1, int x2, int y2);
	virtual ~CPianoHand();
	void destroy();

	void UpdateWithHand(CPianoHand *currHand);
	void AddFinger(int x, int y);
	void UpdateFinger(int index, int x, int y);
	void UpdateFinger(int index, int x);
	void UpdateFinger(int index);
	void SetBounds(int top, int bottom);
	void Copy(CPianoHand *hand);

	//Hand Search Functions
	void SearchForHand(CIplImage *image);
	float Probability2(CPianoHand *currHand, int *splitType);
	float CheckForHand(CIplImage *image, int x, int y, int *splitType);
	void SegmentBetter(CIplImage *image, CIplImage *segImage);

	//General Purpose
	void computeBlob(CIplImage *iplImage, CIplImage *m_imgBlobs, int rootX, int rootY, int value,
								 int *x1, int *y1, int *x2, int *y2);
	int LineSizeRecursive(IplImage *pImg, int i, int j, int *x, int *y);

	//Finger Updating Functions
	void DetectFingers(CIplImage *image);
	void CorrectFingers();

	//Finger Locating Functions
	void MarkNextProbableFinger(CIplImage *colourImage, CIplImage *dest, CIplImage *mask);
	void CreateFingerBlob(CIplImage *image);
	CvPoint FindFingerTip(CIplImage *trace, CIplImage *mask);
	void FingerBlob(CIplImage *iplImage, int rootX, int rootY, int *tipX, int *tipY);
	void MarkAllProbableFingers(CIplImage *colourImage, CIplImage *dest, CIplImage *mask);
	CvPoint FindLongestLine(CIplImage *orig, CIplImage *output);

	//Finger Rect Matching
	int SelectFinger(CvRect *rect);


	int m_nHandID;				//ID number for this hand
	int m_nPersonID;			//ID of person who this hand belongs to
	int m_nLeftOrRight;		// 0 for Left, 1 for Right

	CvPoint m_center;		//Centre of Hand
	CvRect m_boundingBox;	//Bounding Box of Hand //change to m_rectBounding

	int widthOverTime;		//Average Width of past 10 frames or so

	int velocityX, velocityY;	//Velocity of Hand (based on Previous position)

	CvRect m_prevBoundingBox;
	CvPoint m_prevCenter;

	int m_leftOrRight;		// 0 for Left, 1 for Right

	CPianoFinger m_fingers[5];
	int m_nNumFingers;
	
	CPianoHand *otherHand;	//Pointer to the other hand belonging to this person

	CIplImage m_image;	//Image Of Hand
	CIplImage m_handsImage;
	CIplImage m_traceImage;
	CIplImage m_edgeImage;
	CIplImage m_fingerImage;
	CIplImage m_fingImg[5];

	bool ready;
	bool onScreen;

	int topBound, bottomBound;

	bool multiPurposeFlag;

};

#endif // !defined(AFX_PIANOHAND_H__1F59F20B_7951_42CE_A418_A04C4A98C07A__INCLUDED_)

// PianoHand.h: interface for the CPianoHand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIANOHAND_H__1F59F20B_7951_42CE_A418_A04C4A98C07A__INCLUDED_)
#define AFX_PIANOHAND_H__1F59F20B_7951_42CE_A418_A04C4A98C07A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\pvi-utility.h"
#include "pianofinger.h"

#define IMAGE_WIDTH 160
#define IMAGE_HEIGHT 120
//#define IMAGE_WIDTH 320
//#define IMAGE_HEIGHT 240

#define UNCERTAIN 0.5
#define CERTAIN 1.0

class CPianoHand  
{
public:
	CPianoHand();
	CPianoHand(int ID, int x1, int y1, int x2, int y2);
	virtual ~CPianoHand();

	float Probability(CPianoHand *currHand);

	void UpdateWithHand(CPianoHand *currHand);
	void UpdateLikelyFingers();

	void AddFinger(int x, int y);
	void UpdateFinger(int index, int x, int y);
	void UpdateFinger(int index);

	void SearchForHand(CIplImage *image);
	float Probability2(CPianoHand *currHand, int *splitType);
	float CheckForHand(CIplImage *image, int x, int y, int *splitType);
	void computeBlob(CIplImage *iplImage, CIplImage *m_imgBlobs, int rootX, int rootY, int value,
								 int *x1, int *y1, int *x2, int *y2);

	void SegmentBetter(CIplImage *image, CIplImage *segImage);
	void UpdateFingers(CIplImage *image);
	void MarkProbableFingers(CIplImage *colourImage, CIplImage *dest, CIplImage *mask);
	void RemoveSinglePixels(IplImage *image);
	int LineSizeRecursive2(IplImage *pImg, int i, int j, int *x, int *y, int gap, int dir, int *straight);
	void UpdateFingersWithHand(CPianoHand *hand);



	int m_nHandID;				//ID number for this hand
	int m_nPersonID;			//ID of person who this hand belongs to
	int m_nLeftOrRight;		// 0 for Left, 1 for Right

	CvPoint m_center;		//Centre of Hand
	CvRect m_boundingBox;	//Bounding Box of Hand //change to m_rectBounding

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

};

#endif // !defined(AFX_PIANOHAND_H__1F59F20B_7951_42CE_A418_A04C4A98C07A__INCLUDED_)

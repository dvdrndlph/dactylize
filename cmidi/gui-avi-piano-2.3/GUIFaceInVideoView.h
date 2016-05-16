// GUIFaceInVideoView.h : interface of the CGUIFaceInVideoView class
//
/////////////////////////////////////////////////////////////////////////////

//#include "GUIFaceInVideoApp.h"
#include "GUIFaceInVideoDoc.h"
#include "MainFrm.h"
#include "../PianoLib/PianoPlaying.h"	
#include "../PianoLib/ImageSize.h"
#include "../PianoLib/PianoMidi.h"

#if !defined(AFX_GUIFACEINVIDEOVIEW_H__B10EE94C_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_)
#define AFX_GUIFACEINVIDEOVIEW_H__B10EE94C_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VIDEO_STREAM_CURRENT_FRAME				(0)
#define VIDEO_STREAM_LAST_FRAME					(1)
#define VIDEO_STREAM_SECOND_LAST_FRAME			(2)
#define VIDEO_STREAM_LAST_MOTION_FRAME			(3)
#define VIDEO_STREAM_AVERAGE_MOTION_FRAME		(4)

//ARJUN
#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 120
//#define DISPLAY_WIDTH 320
//#define DISPLAY_HEIGHT 240

class CGUIFaceInVideoView : public CView
{
protected: // create from serialization only
	CGUIFaceInVideoView();
	DECLARE_DYNCREATE(CGUIFaceInVideoView)

// Attributes
public:
	CGUIFaceInVideoDoc* GetDocument();
//	static CGUIFaceInVideoDoc* GetDocument();

// Operations
public:

	//CONTROL VALUES
	int m_nHandsDetectThreshold;	//Background Subtraction Threshold
	
	bool m_bColourAverage;		//Colour Average Control
	int m_nColourThreshold;	//Colour Average Threshold
	
	bool m_bEdgeDetect;		//Edge Detection Control
	int m_nEdgeDetectThreshold;//Edge Detection Threshold

	bool m_bDetectHands;		//Hand Detection Control
	bool m_bFingerDetect;		//Finger Detection Control

	int m_nOctave;	//Octave Selector for MIDI matching to Visual coordinates
	int m_nKeyOffset; //Key Offset Selector

	bool m_bMidiCheck;

	//CONTROL VALUE UPDATE FUNCTION
	void UpdateControlValues(CPianoPlaying *pp);

	bool keyDown[200];	//MIDI Keys
	
	//FOR MIDI Event Printing Purposes
	char MIDIString[10][200];
	int numStrings;



	CPianoPlaying *m_pianoPlaying;	// From PVS
	bool m_bReadyToUse;



	/*int m_handsDetectThreshold;

	bool m_colourAverage;
	int m_colourAverageThreshold;

	bool m_edgeDetect;
	int m_edgeDetectThreshold;

	bool m_lineDetect;

	bool m_boundingBox;
	int m_boundingBoxThreshold;

	bool m_detectHands;*/

	int runOnce;

	int interval;

	CPianoMidi *m_Midi;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGUIFaceInVideoView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	// virtual void OnInitialUpdate(); // Dmitry
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGUIFaceInVideoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int m_nVideoStream;
	bool m_bTraceMode, m_bShowPINNwell, m_bShowPINN, m_bRecognizeAlways ;
	bool m_bStretch, m_bFlip, m_bMirror;
	bool m_bUseClient;
	void OnDraw_LogWindow(CDC* pDC, char *strMessage, RECT *textRect, int nLine);
	BOOL OnEraseBkgnd(CDC* pDC); 

// Generated message map functions
protected:
	//{{AFX_MSG(CGUIFaceInVideoView)
	afx_msg void OnConfigSelectClient();
	afx_msg void OnUpdateConfigSelectClient(CCmdUI* pCmdUI);
	afx_msg void OnVideoCurrent();
	afx_msg void OnUpdateVideoCurrent(CCmdUI* pCmdUI);
	afx_msg void OnVideoLast();
	afx_msg void OnUpdateVideoLast(CCmdUI* pCmdUI);
	afx_msg void OnVideoLastMotion();
	afx_msg void OnUpdateVideoLastMotion(CCmdUI* pCmdUI);
	afx_msg void OnVideoSecondLast();
	afx_msg void OnUpdateVideoSecondLast(CCmdUI* pCmdUI);
	afx_msg void OnVideoAverageMotion();
	afx_msg void OnUpdateVideoAverageMotion(CCmdUI* pCmdUI);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in GUIFaceInVideoView.cpp
inline CGUIFaceInVideoDoc* CGUIFaceInVideoView::GetDocument()
   { return (CGUIFaceInVideoDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GUIFACEINVIDEOVIEW_H__B10EE94C_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_)

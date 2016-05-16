// GUIFaceInVideoDoc.h : interface of the CGUIFaceInVideoDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GUIFACEINVIDEODOC_H__B10EE94A_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_)
#define AFX_GUIFACEINVIDEODOC_H__B10EE94A_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "VideoLib.h"
#include "../PVS/PVSystem.h"
//#include "pvi-utility.h"
//#include "FaceRecognizer/FaceRecognizer.h"

//#define FACE_SIZE_48 48
typedef struct GuiButtons
{
	int slider1;
	int slider2;
	int slider3;
	int slider4;
	int check1;
	int check2;
	int check3;
	int check_start;
	int check_nonstop;
	int check_colour;
	int check_motion;
	int check_intensity;
} GuiButtons;

typedef struct InputFiles
{
	TCHAR strPath[MAX_PATH];
	TCHAR strFile1[MAX_PATH];
	TCHAR strFile2[MAX_PATH];
} InputFiles;


class CGUIFaceInVideoDoc : public CDocument
{
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGUIFaceInVideoDoc)
	public:
	//virtual void OnCloseDocument(); // Dmitry
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

protected: // create from serialization only
	CGUIFaceInVideoDoc();
	DECLARE_DYNCREATE(CGUIFaceInVideoDoc)

public:
	virtual ~CGUIFaceInVideoDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Attributes
protected:
	CVideoCameraEntry m_videoCameraEntry;
public:
	CPVS_WithCapture m_PVSystem;

//	CFaceRecognizer m_frIds;

	FILE *m_fLog, *m_fScenario, *m_fScenarioTest, 
		*m_fFeaturesR, *m_fFeaturesB;;
	char m_strFaceList[200], m_strImagePath[200], m_strImageExt[6], m_strImagePrefix[30] ; 

	unsigned char *m_arruFace;

public:
	bool m_bVideoFileOpen;
	bool m_bVideoFilePaused;
	GuiButtons m_gui;
	InputFiles m_input;

	bool m_recordCheck;

	bool m_bWorkWithFiles;

	CvPoint m_pointLEye, m_pointREye;
	CIplImage m_imgIn;

#define MAX_STACK 20

	int nStack[MAX_STACK];
	char sStackY[MAX_STACK][100], sStackS[MAX_STACK][100];
	int nLastFrameIDs;
	int nLastFrameID[20];

// Implementation
public:
	CPVS_WithCapture *getPVS() { return &m_PVSystem; }
	int openFaceList();
	void closeFaceList();



// Generated message map functions
protected:
	//{{AFX_MSG(CGUIFaceInVideoDoc)
	afx_msg void OnConfigSelectCamera();
	afx_msg void OnConfigSelectVideofile();
	afx_msg void OnVideofilePlay();
	afx_msg void OnUpdateVideofilePlay(CCmdUI* pCmdUI);
	afx_msg void OnVideofileRewindFrame();
	afx_msg void OnUpdateVideofileRewindFrame(CCmdUI* pCmdUI);
	afx_msg void OnVideofileRewindStart();
	afx_msg void OnUpdateVideofileRewindStart(CCmdUI* pCmdUI);
	afx_msg void OnVideofileForwardFrame();
	afx_msg void OnUpdateVideofileForwardFrame(CCmdUI* pCmdUI);
	afx_msg void OnVideofileForwardEnd();
	afx_msg void OnUpdateVideofileForwardEnd(CCmdUI* pCmdUI);
	afx_msg void OnVideofileStop();
	afx_msg void OnUpdateVideofileStop(CCmdUI* pCmdUI);
	afx_msg void OnConfigCameraProperties();
	afx_msg void OnConfigFile1();
	afx_msg void OnConfigFile2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GUIFACEINVIDEODOC_H__B10EE94A_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_)

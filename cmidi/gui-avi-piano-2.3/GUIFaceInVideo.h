// GUIFaceInVideo.h : main header file for the GUIFACEINVIDEO application
//

#if !defined(AFX_GUIFACEINVIDEO_H__B10EE944_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_)
#define AFX_GUIFACEINVIDEO_H__B10EE944_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoApp:
// See GUIFaceInVideo.cpp for the implementation of this class
//

class CGUIFaceInVideoApp : public CWinApp
{
public:
	CGUIFaceInVideoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGUIFaceInVideoApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CGUIFaceInVideoApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GUIFACEINVIDEO_H__B10EE944_A566_11D6_B7C0_00C0F04CEABC__INCLUDED_)

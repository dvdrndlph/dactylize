#if !defined(AFX_GUI_H__A33860C1_129F_11D7_B7C0_00C0F04CEABC__INCLUDED_)
#define AFX_GUI_H__A33860C1_129F_11D7_B7C0_00C0F04CEABC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Gui.h : header file
//
#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
// Gui dialog

class Gui : public CDialogBar
{
// Construction
public:
	Gui(/*CWnd* pParent = NULL*/);   // standard constructor

public:
	void initialize();
	void updateControlValues();
// Dialog Data
	//{{AFX_DATA(Gui)
	enum { IDD = IDD_DIALOGBAR1 };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Gui)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Gui)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheck2();
	afx_msg void OnCheck3();
	afx_msg void OnCheck1();
	afx_msg void OnCheckStart();
	afx_msg void OnCheckNonstop();
	afx_msg void OnCheckMotion();
	afx_msg void OnCheckIntensity();
	afx_msg void OnCheckColour();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GUI_H__A33860C1_129F_11D7_B7C0_00C0F04CEABC__INCLUDED_)

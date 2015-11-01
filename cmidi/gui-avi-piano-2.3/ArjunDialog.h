#if !defined(AFX_ARJUNDIALOG_H__744DEFD7_C135_4F9B_B0B0_13802EAA1243__INCLUDED_)
#define AFX_ARJUNDIALOG_H__744DEFD7_C135_4F9B_B0B0_13802EAA1243__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ArjunDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ArjunDialog dialog

class ArjunDialog : public CDialogBar
{
// Construction
public:
	ArjunDialog(CWnd* pParent = NULL);   // standard constructor
	void initialize(void);
// Dialog Data
	//{{AFX_DATA(ArjunDialog)
	enum { IDD = IDD_ARJUNDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ArjunDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ArjunDialog)
	afx_msg void OnEdgeCheck();
	afx_msg void OnAverageColourCheck();
	afx_msg void OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBgcap();
	afx_msg void OnFingerDetect();
	afx_msg void OnRecord();
	afx_msg void OnHanddetect();
	afx_msg void OnReleasedcaptureSlider4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSlider5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMidicheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ARJUNDIALOG_H__744DEFD7_C135_4F9B_B0B0_13802EAA1243__INCLUDED_)

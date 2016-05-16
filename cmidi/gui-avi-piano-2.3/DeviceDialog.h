#if !defined(AFX_DEVICEDIALOG_H__5C3A575C_9AD0_11D5_ADCE_00010289BF94__INCLUDED_)
#define AFX_DEVICEDIALOG_H__5C3A575C_9AD0_11D5_ADCE_00010289BF94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeviceDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDeviceDialog dialog

#include "../VideoLib/VideoLib.h"

class CDeviceDialog : public CDialog
{
// Construction
public:
	CDeviceDialog(CWnd* pParent = NULL);   // standard constructor
	~CDeviceDialog();

	CVideoCameraEntry *getSelectedEntry() { return m_pSelectedEntry; }
	int getWidth() { return m_nWidth; }
	int getHeight() { return m_nHeight; }
	int getBpp() { return m_nBpp; }

// Dialog Data
	//{{AFX_DATA(CDeviceDialog)
	enum { IDD = IDD_DEVICE_DIALOG };
	CListBox	m_deviceListBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CameraDeviceList m_deviceList;
	CVideoCameraEntry *m_pSelectedEntry;

	int m_nWidth;
	int m_nHeight;
	int m_nBpp;

	// Generated message map functions
	//{{AFX_MSG(CDeviceDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICEDIALOG_H__5C3A575C_9AD0_11D5_ADCE_00010289BF94__INCLUDED_)

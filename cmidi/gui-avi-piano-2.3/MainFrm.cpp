// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "GUIFaceInVideo.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Create the setting bar
	m_wndSettingBar.Create(this, IDD_DIALOG, CBRS_RIGHT, IDD_DIALOG);
	m_wndSettingBar.initialize();

	m_wndArjunBar.Create(this, IDD_ARJUNDIALOG, CBRS_BOTTOM, IDD_ARJUNDIALOG);
	m_wndArjunBar.initialize();
		
	//m_wndArjunBar.updateControlValues();
	//m_wndArjunBar.initialize();

	//CWnd* dlg = GetDlgItem(IDD_DIALOG);
//	((CButton*)dlg->GetDlgItem(IDC_RADIO1))->SetCheck(0);

/*
	((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER1))->SetRange(0,91,FALSE);
	((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER1))->SetPos(42);
	//((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER1))->SetTicFreq(92/4);
	((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER2))->SetRange(0,91,FALSE);
	((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER2))->SetPos(50);
	((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER3))->SetRange(0,139,FALSE);
	((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER3))->SetPos(45);
	((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER4))->SetRange(0,139,FALSE);
	((CSliderCtrl*)dlg->GetDlgItem(IDC_SLIDER4))->SetPos(48);
*/

/*	int CURSOR_SENSITIVITY_TICKS = 10;
	CSliderCtrl *pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	if(pSlider)
	{
	    pSlider->SetRange(0, 10, FALSE);
		pSlider->SetPos(5);
	}
*/
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);



	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


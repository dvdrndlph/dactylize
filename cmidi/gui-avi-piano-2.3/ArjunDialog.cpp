// ArjunDialog.cpp : implementation file
//

#include "stdafx.h"
#include "GUIFaceInVideo.h"
#include "ArjunDialog.h"
#include "MainFrm.h"

#include "GUIFaceInVideoView.h"
#include "GUIFaceInVideoDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ArjunDialog dialog


ArjunDialog::ArjunDialog(CWnd* pParent /*=NULL*/)
	: CDialogBar()
{
	//{{AFX_DATA_INIT(ArjunDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ArjunDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(ArjunDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ArjunDialog, CDialogBar)
	//{{AFX_MSG_MAP(ArjunDialog)
	ON_BN_CLICKED(IDC_EDGE_CHECK, OnEdgeCheck)
	ON_BN_CLICKED(IDC_AVERAGE_COLOUR_CHECK, OnAverageColourCheck)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnReleasedcaptureSlider1)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER2, OnReleasedcaptureSlider2)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER3, OnReleasedcaptureSlider3)
	ON_BN_CLICKED(IDC_BGCAP, OnBgcap)
	ON_BN_CLICKED(IDC_FINGERDETECT, OnFingerDetect)
	ON_BN_CLICKED(IDC_RECORD, OnRecord)
	ON_BN_CLICKED(IDC_HANDDETECT, OnHanddetect)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER4, OnReleasedcaptureSlider4)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER5, OnReleasedcaptureSlider5)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MIDICHECK, OnMidicheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ArjunDialog message handlers

void ArjunDialog::OnEdgeCheck() 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	if (pView->m_bEdgeDetect == true)
		pView->m_bEdgeDetect = false;
	else
		pView->m_bEdgeDetect = true;

}

void ArjunDialog::OnAverageColourCheck() 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	if (pView->m_bColourAverage == true)
		pView->m_bColourAverage = false;
	else
		pView->m_bColourAverage = true;
}

void ArjunDialog::OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	CSliderCtrl *pSlider1 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	pView->m_nHandsDetectThreshold = pSlider1->GetPos();
	*pResult = 0;
}

void ArjunDialog::OnReleasedcaptureSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	CSliderCtrl *pSlider2 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
	pView->m_nColourThreshold = pSlider2->GetPos();

	*pResult = 0;
}

void ArjunDialog::OnReleasedcaptureSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	CSliderCtrl *pSlider3 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
	pView->m_nEdgeDetectThreshold = pSlider3->GetPos();

	*pResult = 0;
}



void ArjunDialog::OnBgcap() 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;	

	//Allow New Backgorund to be Captured
	pView->runOnce = 0;
}

void ArjunDialog::OnFingerDetect() 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	if (pView->m_bFingerDetect == true)
		pView->m_bFingerDetect = false;
	else
		pView->m_bFingerDetect = true;		
}

void ArjunDialog::initialize(void)
{
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER1))->SetRange(0,100,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER1))->SetPos(50);

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER2))->SetRange(0,100,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER2))->SetPos(30);
	
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER3))->SetRange(0,255,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER3))->SetPos(3);
	
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER4))->SetRange(-3,3,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER4))->SetPos(0);

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER5))->SetRange(0,7,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER5))->SetPos(0);
}
void ArjunDialog::OnRecord() 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	if(!pDoc) 
		return;

	if (pDoc->m_recordCheck == true)
		pDoc->m_recordCheck = false;
	else
		pDoc->m_recordCheck = true;		
	
}

void ArjunDialog::OnHanddetect() 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;
	
	pView->m_bDetectHands = true;
}

void ArjunDialog::OnReleasedcaptureSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	CSliderCtrl *pSlider4 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER4);
	pView->m_nOctave = pSlider4->GetPos() * -1;	//*-1 because the image is flipped, for it to be easier for user
		
	*pResult = 0;
}

void ArjunDialog::OnReleasedcaptureSlider5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	CSliderCtrl *pSlider5 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER5);
	pView->m_nKeyOffset = (7-pSlider5->GetPos()); //7-  because the image is flipped, so key offest goes backwards
		
	*pResult = 0;
}

void ArjunDialog::OnTimer(UINT nIDEvent) 
{
	
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;

	CListBox* pListBox = (CListBox*)GetDlgItem(IDD_KEYLISTBOX);
	if(pListBox)
	{
		pListBox->ResetContent();
		for(int i = 0; i < pView->numStrings; i++)
		{
			// Store the entry into the list
			pListBox->InsertString(i, pView->MIDIString[i]);
		}			
	}
	
	CDialogBar::OnTimer(nIDEvent);
}

void ArjunDialog::OnMidicheck() 
{
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) 
		return;
	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)pMainFrame->GetActiveView();
	if(!pView) 
		return;
	pView->m_bMidiCheck=true;

	SetTimer(1, 50, 0);	
	
}

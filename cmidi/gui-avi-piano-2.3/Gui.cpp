// Gui.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrm.h"
#include "GUIFaceInVideo.h"
#include "GUIFaceInVideoDoc.h"
#include "GUIFaceInVideoView.h"

#include "Gui.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Gui dialog


Gui::Gui(/*CWnd* pParent /*=NULL*/)
	: CDialogBar(/*Gui::IDD, pParent*/)
{
	//{{AFX_DATA_INIT(Gui)
	//}}AFX_DATA_INIT
}

void Gui::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Gui)
	//}}AFX_DATA_MAP
}


void Gui::initialize() {
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER1))->SetRange(0,50,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER1))->SetPos(10);
	((CStatic*)GetDlgItem(IDC_MINU_VALUE))->SetWindowText("10");

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER2))->SetRange(0,100,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER2))->SetPos(10);
	((CStatic*)GetDlgItem(IDC_MAXU_VALUE))->SetWindowText("10");

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER3))->SetRange(0,5,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER3))->SetPos(0);
	((CStatic*)GetDlgItem(IDC_MINV_VALUE))->SetWindowText("0");

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER4))->SetRange(0,10,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER4))->SetPos(0);
	((CStatic*)GetDlgItem(IDC_MAXV_VALUE))->SetWindowText("0");

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER6))->SetRange(0,1000,FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER6))->SetPos(0);
	((CStatic*)GetDlgItem(IDC_MINV_VALUE))->SetWindowText("0");

	updateControlValues();
}


void Gui::updateControlValues() {
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(!pMainFrame) return;
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	if(!pDoc) return;

	CSliderCtrl *pSlider1 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	CSliderCtrl *pSlider2 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
	CSliderCtrl *pSlider3 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
	CSliderCtrl *pSlider4 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER4);
	CSliderCtrl *pSlider6 = (CSliderCtrl*)GetDlgItem(IDC_SLIDER6);

	char posAsText[10];

	pDoc->m_gui.slider1 = pSlider1->GetPos();
	sprintf(posAsText, "%d", pSlider1->GetPos());
	((CStatic*)GetDlgItem(IDC_MINU_VALUE))->SetWindowText(posAsText);
	pDoc->m_gui.slider2 = pSlider2->GetPos();
	sprintf(posAsText, "%d", pSlider2->GetPos());
	((CStatic*)GetDlgItem(IDC_MAXU_VALUE))->SetWindowText(posAsText);
	pDoc->m_gui.slider3 = pSlider3->GetPos();
	sprintf(posAsText, "%d", pSlider3->GetPos());
	((CStatic*)GetDlgItem(IDC_MINV_VALUE))->SetWindowText(posAsText);
	pDoc->m_gui.slider4 = pSlider4->GetPos();
	sprintf(posAsText, "%d", pSlider4->GetPos());
	((CStatic*)GetDlgItem(IDC_MAXV_VALUE))->SetWindowText(posAsText);


//	pDoc->m_PVSystem. = pSlider6->GetPos();	
//	pSlider6->GetPos() / 1000;
//	cvSetCaptureProperty (pDoc->m_PVSystem.m_cvCapture, CV_CAP_PROP_POS_FRAMES, pSlider6->GetPos());

}


BEGIN_MESSAGE_MAP(Gui, CDialogBar)
	//{{AFX_MSG_MAP(Gui)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK2, OnCheck2)
	ON_BN_CLICKED(IDC_CHECK3, OnCheck3)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_CHECK_START, OnCheckStart)
	ON_BN_CLICKED(IDC_CHECK_NONSTOP, OnCheckNonstop)
	ON_BN_CLICKED(IDC_CHECK_MOTION, OnCheckMotion)
	ON_BN_CLICKED(IDC_CHECK_INTENSITY, OnCheckIntensity)
	ON_BN_CLICKED(IDC_CHECK_COLOUR, OnCheckColour)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Gui message handlers

void Gui::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{

/* Chris can you update this function to get values from, lets say, four sliders, which 
you may use to set min and max for Uf, Uf in skin CColourChannel.m_pointUV 
	
Please also create a similar function to get values for pDoc->m_gui.radio1 and radio2, which you would 
use to Learn Skin (YES/NO) and Learn Mode (FROMPIXEL/FROM_REGION)
	*/

//	char output[20];
//	int  nnn = pSlider->GetPos();
//	sprintf(output, "%d\n",nnn);
//	OutputDebugString(output);
//	MessageBox(output);


//	CGUIFaceInVideoDoc* pDoc = CGUIFaceInVideoView::GetDocument();
//	CGUIFaceInVideoDoc* pDoc = GetDocument();

	updateControlValues();


	CDialogBar::OnHScroll(nSBCode, nPos, pScrollBar);
}


void Gui::OnCheckStart() 
{
	// TODO: Add your control notification handler code here
	CButton *pCheck = (CButton*)GetDlgItem(IDC_CHECK_START);
	if(!pCheck) return;

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	pDoc->m_gui.check_start = (pCheck->GetState() & 0x0003);
}


void Gui::OnCheckNonstop() 
{
	// TODO: Add your control notification handler code here
	CButton *pCheck = (CButton*)GetDlgItem(IDC_CHECK_NONSTOP);
	if(!pCheck) return;

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	pDoc->m_gui.check_nonstop = (pCheck->GetState() & 0x0003);
}

void Gui::OnCheck1() 
{
	// TODO: Add your control notification handler code here
	CButton *pCheck1 = (CButton*)GetDlgItem(IDC_CHECK1);
	if(!pCheck1) return;

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	pDoc->m_gui.check1 = (pCheck1->GetState() & 0x0003);
}

void Gui::OnCheck2() 
{
	// TODO: Add your control notification handler code here
	CButton *pCheck2 = (CButton*)GetDlgItem(IDC_CHECK2);
	if(!pCheck2) return;

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	pDoc->m_gui.check2 = (pCheck2->GetState() & 0x0003);
}

void Gui::OnCheck3() 
{
	// TODO: Add your control notification handler code here
	CButton *pCheck3 = (CButton*)GetDlgItem(IDC_CHECK3);
	if(!pCheck3) return;

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	pDoc->m_gui.check3 = (pCheck3->GetState() & 0x0003);
}


void Gui::OnCheckMotion() 
{
	// TODO: Add your control notification handler code here
	CButton *pCheck3 = (CButton*)GetDlgItem(IDC_CHECK_MOTION);
	if(!pCheck3) return;

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	pDoc->m_gui.check_motion = (pCheck3->GetState() & 0x0003);	
}

void Gui::OnCheckIntensity() 
{
	CButton *pCheck3 = (CButton*)GetDlgItem(IDC_CHECK_INTENSITY);
	if(!pCheck3) return;

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	pDoc->m_gui.check_intensity = (pCheck3->GetState() & 0x0003);
}

void Gui::OnCheckColour() 
{
	CButton *pCheck3 = (CButton*)GetDlgItem(IDC_CHECK_COLOUR);
	if(!pCheck3) return;

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CGUIFaceInVideoDoc* pDoc = (CGUIFaceInVideoDoc*)pMainFrame->GetActiveDocument();
	pDoc->m_gui.check_colour = (pCheck3->GetState() & 0x0003);	
}

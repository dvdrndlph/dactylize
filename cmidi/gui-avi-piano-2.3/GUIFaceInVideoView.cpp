// GUIFaceInVideoView.cpp : implementation of the CGUIFaceInVideoView class
// 
// Descpription: PVS project user-end function access example
//
// Author: Dmitry Goorodnichy (Copyright NRC-CNRC 2001-2004)
// 
// Legal warning:
// Any reproduction of these code outside of NRC-CNRC premises is strictly prohibited.

#include "stdafx.h"
#include "GUIFaceInVideo.h"

#include "GUIFaceInVideoDoc.h"
#include "GUIFaceInVideoView.h"

#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoView

IMPLEMENT_DYNCREATE(CGUIFaceInVideoView, CView)

BEGIN_MESSAGE_MAP(CGUIFaceInVideoView, CView)
	//{{AFX_MSG_MAP(CGUIFaceInVideoView)
	ON_COMMAND(ID_CONFIG_SELECT_CLIENT, OnConfigSelectClient)
	ON_UPDATE_COMMAND_UI(ID_CONFIG_SELECT_CLIENT, OnUpdateConfigSelectClient)
	ON_COMMAND(ID_VIDEO_CURRENT, OnVideoCurrent)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_CURRENT, OnUpdateVideoCurrent)
	ON_COMMAND(ID_VIDEO_LAST, OnVideoLast)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_LAST, OnUpdateVideoLast)
	ON_COMMAND(ID_VIDEO_LAST_MOTION, OnVideoLastMotion)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_LAST_MOTION, OnUpdateVideoLastMotion)
	ON_COMMAND(ID_VIDEO_SECOND_LAST, OnVideoSecondLast)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_SECOND_LAST, OnUpdateVideoSecondLast)
	ON_COMMAND(ID_VIDEO_AVERAGE_MOTION, OnVideoAverageMotion)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_AVERAGE_MOTION, OnUpdateVideoAverageMotion)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoView construction/destruction

CGUIFaceInVideoView::~CGUIFaceInVideoView()
{
}

BOOL CGUIFaceInVideoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// Dmitry
/* void CGUIFaceInVideoView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

  	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)GetActiveView();
	CGUIFaceInVideoDoc *pDoc = (CGUIFaceInVideoDoc*)pView->GetDocument();
	CPerceptualVisionSystem *pPVS = pDoc->getPVS();

	pPVS->initializeFromRegistry(m_hWnd, &m_videoCameraEntry);
	
	// Update the GUI settings
	CMainFrame* pMainFrame = (CMainFrame*)GetParentFrame();
	if(pMainFrame)
	{
		pMainFrame->setGuiControlMode( m_PVSystem.m_control.getControlMode() );
		pMainFrame->setGuiCursorSensitivity( m_PVSystem.m_control.getCursorSensitivity() );
		pMainFrame->updateSettingBar();
	}

}
*/


/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoView diagnostics

#ifdef _DEBUG
void CGUIFaceInVideoView::AssertValid() const
{
	CView::AssertValid();
}

void CGUIFaceInVideoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGUIFaceInVideoDoc* CGUIFaceInVideoView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGUIFaceInVideoDoc)));
	return (CGUIFaceInVideoDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoView message handlers



void CGUIFaceInVideoView::OnConfigSelectClient() 
{
	m_bUseClient = m_bUseClient ? 0 : 1;
//	m_nVideoStream = VIDEO_STREAM_LAST_FRAME;
}
void CGUIFaceInVideoView::OnUpdateConfigSelectClient(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bUseClient ? 1 : 0);
}



void CGUIFaceInVideoView::OnVideoCurrent() 
{
	m_nVideoStream = VIDEO_STREAM_CURRENT_FRAME;
}

void CGUIFaceInVideoView::OnUpdateVideoCurrent(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_nVideoStream == VIDEO_STREAM_CURRENT_FRAME ? 1 : 0);
}



void CGUIFaceInVideoView::OnVideoLast() 
{
	m_bTraceMode = m_bTraceMode ? 0 : 1;
//	m_nVideoStream = VIDEO_STREAM_LAST_FRAME;
}
void CGUIFaceInVideoView::OnUpdateVideoLast(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bTraceMode ? 1 : 0);
}



void CGUIFaceInVideoView::OnVideoSecondLast() 
{
	m_bShowPINN = m_bShowPINN ? 0 : 1;
//	m_nVideoStream = VIDEO_STREAM_SECOND_LAST_FRAME;
}
void CGUIFaceInVideoView::OnUpdateVideoSecondLast(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowPINN ? 1 : 0);
}


void CGUIFaceInVideoView::OnVideoLastMotion() 
{
	m_bShowPINNwell = m_bShowPINNwell ? 0 : 1;
	//	m_nVideoStream = VIDEO_STREAM_LAST_MOTION_FRAME;
}
void CGUIFaceInVideoView::OnUpdateVideoLastMotion(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bShowPINNwell ? 1 : 0);
}



void CGUIFaceInVideoView::OnVideoAverageMotion() 
{
	m_bRecognizeAlways = m_bRecognizeAlways ? 0 : 1;
//	m_nVideoStream = VIDEO_STREAM_AVERAGE_MOTION_FRAME;
}

void CGUIFaceInVideoView::OnUpdateVideoAverageMotion(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_bRecognizeAlways ? 1 : 0);
}





void CGUIFaceInVideoView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	int nC = point.x;
	// show 3 octaves
	m_pianoPlaying->m_Piano->m_nOrigC_X2 = int(float(point.x - m_pianoPlaying->m_Piano->m_nOrigC_X1) / 21) + m_pianoPlaying->m_Piano->m_nOrigC_X1;
//	m_pianoPlaying.m_Piano.m_nOrigC_X2 = int(float(point.x - m_pianoPlaying.m_Piano.m_nOrigC_X1) / 7) + m_pianoPlaying.m_Piano.m_nOrigC_X1;
	m_pianoPlaying->m_Piano->m_nC_X2 = m_pianoPlaying->m_Piano->m_nOrigC_X2;
	//m_pianoPlaying.m_Piano.m_nKeyboardY2 = point.y;
	//m_pianoPlaying.m_Piano.m_nC_X1 = fabs(m_pianoPlaying.m_Piano.m_nC_X2 - m_pianoPlaying.m_Piano.m_nC_X1) / 7;
	
	CView::OnLButtonUp(nFlags, point);
}


void CGUIFaceInVideoView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	int nC = point.x;
	m_pianoPlaying->m_Piano->m_nOrigC_X1 = point.x;
	m_pianoPlaying->m_Piano->m_nC_X1 = point.x;
	//m_pianoPlaying.m_Piano.m_nKeyboardY1 = point.y;
		
	CView::OnLButtonDown(nFlags, point);
}


BOOL CGUIFaceInVideoView::OnEraseBkgnd(CDC* pDC) 
{
#define BACKGROUND_COLOUR	(RGB(255, 255, 255))	//		(RGB(0,0,100))

	CRect r;
	pDC->GetClipBox(&r);
	pDC->FillSolidRect(&r, BACKGROUND_COLOUR);
	return TRUE;
}


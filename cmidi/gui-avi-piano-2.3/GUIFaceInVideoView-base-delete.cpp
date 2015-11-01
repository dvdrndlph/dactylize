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

/////////////////////////////////////////////////////////////////////////////
//---- TCP-IP server ----
#define PORT_NUM 20200
#include <winsock.h>

#define MAX_PACKET_SIZE 100000

#define NUM_CONS 1	
// int num_cons=5;
int num_bytes_rec;
char host_name1[1024],host_name2[1024],host_ip[1024];
int port_num,packet_len,max_packet_len,tx_cache_size,rx_cache_size;
char end,is_more;
unsigned char packet[MAX_PACKET_SIZE];
char last_open[10];
char idstring[512];



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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



BOOL CGUIFaceInVideoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}


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
	
	CView::OnLButtonUp(nFlags, point);
}


BOOL CGUIFaceInVideoView::OnEraseBkgnd(CDC* pDC) 
{
#define BACKGROUND_COLOUR	(RGB(255, 255, 255))	//		(RGB(0,0,100))

	CRect r;
	pDC->GetClipBox(&r);
	pDC->FillSolidRect(&r, BACKGROUND_COLOUR);
	return TRUE;
}



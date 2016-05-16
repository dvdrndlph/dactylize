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


#define IMAGE_160 160
#define IMAGE_120 120


RECT *set_window(int x, int y, int size, int dxOffset, int dyOffset) 
{
	static RECT clientRect;
	int Xsize = IMAGE_160, Ysize = IMAGE_120; 

	clientRect.left = (Xsize+1)*x + 2*x +(size+1)*dxOffset;
	clientRect.top = (Ysize+1)*y + 2*y + (size+1)*dyOffset;;
	clientRect.right = clientRect.left+size;
	clientRect.bottom = clientRect.top+size;	
	return &clientRect;
}
RECT *set_window(int x, int y, int xExtra, int yExtra) // additional windows
{
	static RECT clientRect;
	int Xsize = IMAGE_160, Ysize = IMAGE_120; //320; // pImage->getWidth()*2;

	clientRect.left = (Xsize)*x + 2*x;
	clientRect.top = (Ysize+1)*y + 2*y;
	clientRect.right = (Xsize)*(x+1+xExtra) + 2*x;
	clientRect.bottom = (Ysize)*(y+1+yExtra) + 2*y;	
	return &clientRect;
}
RECT *set_window(int x, int y) 
{	return set_window(x, y, 0, 0); 
}

RECT *set_window(int x, int y, int size) 
{ 	return set_window(x, y, size, 0, 0); 
}


RECT *set_faceRect(int n) // , RECT *clientRect)
{
	static RECT faceRect;
	int FACES_PER_LINE = 25;// 10; // 20
	int SIDE = 28; // 36; // 48; 
	int Y0 = 120*2+45;

	int x = n % FACES_PER_LINE;
	int y = n / FACES_PER_LINE;
	int Xsize = SIDE, Ysize = SIDE; // 24, CANONIC_FACE_SIZE


	faceRect.left = (Xsize)*x + 2*x;
	faceRect.top = (Ysize)*y + 2*y      + Y0 + 3;
	faceRect.right = (Xsize)*(x+1) + 2*x;
	faceRect.bottom = (Ysize)*(y+1) + 2*y + Y0 + 3;	

	return &faceRect;
}



/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoView construction/destruction

CGUIFaceInVideoView::CGUIFaceInVideoView()
{
	m_nVideoStream = VIDEO_STREAM_CURRENT_FRAME;
	m_bUseClient = false;
	m_bTraceMode = false;
	m_bShowPINN = true;
	m_bShowPINNwell = false;
	m_bRecognizeAlways = false;

/*	m_bStretch = true; // false;
	m_bFlip = false;
	m_bMirror = false;*/

	
	//--initial set up network--

//	OnDraw_LogWindow(pDC, "Starting up TCP server", set_window(0,0), 0);
//	OnDraw_LogWindow(pDC, idstring, set_window(0,0), 1);

	char str[400];
	sprintf(str, "Starting up TCP server. \n This computer's IP=<%s> port:%d", host_ip, PORT_NUM);
//	MessageBox(str,	MB_OK); 

}

CGUIFaceInVideoView::~CGUIFaceInVideoView()
{
}

BOOL CGUIFaceInVideoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoView drawing


void CGUIFaceInVideoView::OnDraw(CDC* pDC)
{ 
	CGUIFaceInVideoDoc* pDoc = GetDocument();    

//	CGUIFaceInVideoView* pView = (CGUIFaceInVideoView*)GetActiveView();
//	CGUIFaceInVideoDoc *pDoc = (CGUIFaceInVideoDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc == NULL)
		return;

	CFaceRecognizer *frIds = & pDoc->m_frIds;

// in 24x24 image
// char sWhatValuesToUse[5]; // "011" - don't use I, use dIx, dIy  
// char sHowToCrop[5]; // "22001" - cut from left and right 2 pixels , from top/bottom 0 pixels and scale by 1.
	char 
//		sWhatValuesToUse[]="111", 
		sWhatValuesToUse[]="111", 
		sHowToCrop[]="00001";
	// just remeber: The m_imgY will not be diplayed properly if m_nWidth is not muliple of 4 !

	sHowToCrop[3]='0';

	frIds->initialize(10, sWhatValuesToUse, sHowToCrop);


	CIplImage *pImgIn;
	CvPoint *pPointL, *pPointR;
	int nNameTag, nBestMatch; 
	
	char strMessage[256], str[200], strNameTag[40];
	
/*
	if (frIds->m_nStoredFaces > frIds->m_pinn.m_N / 2) // MAX_FACES - 1 )
	{
		OnDraw_LogWindow(pDC, "Memory is full!", set_window(0,0), 0);
		return;
	}
*/	

// Update all sliders data:

	frIds->m_pinn.m_nNoise = pDoc->m_gui.slider1*frIds->m_pinn.m_N/100; 
	frIds->m_pinn.m_D = pDoc->m_gui.slider2/100.0; 
//Doc->m_memory.m_pinn.m_dFade=1;// 1-pDoc->m_gui.slider3/1000.0;

// ************************************************************************************************
// Show MemoryContents
// ************************************************************************************************


	if (m_nVideoStream == VIDEO_STREAM_CURRENT_FRAME)  // Draw all stored faces
		for(int n=0; n< MIN (frIds->m_nStoredFaces, FR_MAX_FACES); n++) // frIds->m_nStoredFaces
			frIds->m_arrImgStoredFaces[n].draw(pDC->m_hDC, set_faceRect(n));

	if (m_bShowPINN )  // Draw the contents of PINN memory
	{
//		if (m_bShowPINNwell )    
		{

/*			RECT rect;
			rect.left = IMAGE_160*2+2;
			rect.top = 0; 
			rect.right = rect.left + frIds->m_pinn.m_N;
			rect.bottom = rect.top + frIds->m_pinn.m_N;	*/
			frIds->getPINNasImage()->
				draw(pDC->m_hDC, set_window(3,0, 1, 1), false);
		}
//		else
//			frIds->getPINNasImage()->draw(pDC->m_hDC, set_window(4,0, 264), true); //, false);
	}

		
// Clear memory and reset all checks to unchecked
	
	if (pDoc->m_gui.check3)  
	{
		CMainFrame *pMainFrm = (CMainFrame*)GetParentFrame();
		CButton* check = (CButton*)((pMainFrm->GetDlgItem(IDD_DIALOGBAR1))->GetDlgItem(IDC_CHECK2));
		check->SetCheck(BST_UNCHECKED); // back to video mode
		check = (CButton*)((pMainFrm->GetDlgItem(IDD_DIALOGBAR1))->GetDlgItem(IDC_CHECK1));
		check->SetCheck(BST_UNCHECKED);// back to memorization mode
		check = (CButton*)((pMainFrm->GetDlgItem(IDD_DIALOGBAR1))->GetDlgItem(IDC_CHECK3));
		check->SetCheck(BST_UNCHECKED);			
		
		pDoc->m_gui.check1 = false; 
		pDoc->m_gui.check2 = false; 
		pDoc->m_gui.check3 = false; 
		
//		fclose(pDoc->m_fScenario);
//		fclose(pDoc->m_fScenarioTest);

		pDoc->closeFaceList();
		pDoc->openFaceList();
			
		pMainFrm->UpdateData(FALSE);
		
		frIds->destroy();
		OnEraseBkgnd(pDC);
		
		//		pDoc->m_bWorkWithFiles = false;
		OnDraw_LogWindow(pDC, "Memory cleared          ", set_window(1,0), 0);
		
		return;
	}
	

/***************************************************************************************/
/*** Use data from video camera / AVI **************************************************/
/***************************************************************************************/


				
	CPerceptualVisionSystem *pPVS = pDoc->getPVS();
		
	OnDraw_LogWindow(pDC, pPVS->m_strFrameRate, set_window(0,2,1,0), 0);
	
	sprintf(strMessage, "%i: %i (%i) msec        ", pPVS->m_nFrame, pPVS->m_tBetweenFrames, pPVS->m_tBetweenFramesNow);		
	OnDraw_LogWindow(pDC, strMessage, set_window(0,2), 1);
		
	bool bRes = pPVS->initializeNextFrame();
	if (bRes == false)
		return;		// if want to show it but not process it, change it.
	
	pPVS->m_imgIn.draw(pDC->m_hDC, set_window(0,0), false, false);

// Detect up to 1 person and 
//	load the rescaled to 28x28 face of this person into m_face 

	int nFacesDetected = pPVS->m_fd.detectFacesIn(&pPVS->m_imgBW);

	if (nFacesDetected<=0)
		return;
	
	sprintf(strMessage, "%i faced detected", nFacesDetected);		
	OnDraw_LogWindow(pDC, strMessage, set_window(0,2), -2);

	if (nFacesDetected>1)
		OnDraw_LogWindow(pDC, strMessage, set_window(0,1,1,0), 5);
	else
		OnDraw_LogWindow(pDC, "                            ", set_window(0,1,1,0), 5);

	
	pPVS->m_chanColour.updateImages(&pPVS->m_imgIn);// NB: Colour is useless, unless you recompute skin model (like Bradski)

	pPVS->m_chanMotion.updateImages(&pPVS->m_imgIn);
	pPVS->m_chanMotion.compute_dI();
//	pPVS->m_chanMotion.compute_ddI();
	pPVS->m_chanMotion.compute_FG();

	int nFaceRotation;

	for(int nFace=0; nFace<nFacesDetected; nFace++)
//	for(int nFace=0; nFace<1; nFace++)
	{
		CvPoint ptL, ptR; 
		int nEye;

		CFace::fdRect2Eyes(pPVS->m_fd.getLocationOfFace(nFace), &ptL, &ptR, &nEye);

		CvPoint pt00={ptL.x-nEye/2, ptL.y-nEye/2},	pt11={ptR.x+nEye/2, ptR.y+nEye};
		CvRect rect={pt00.x, pt00.y, pt11.x-pt00.x, pt11.y-pt00.y}; 

		cvRectangle(pPVS->m_imgOut.getIplImage(), pt00, pt11, 0xffffaa, 2 );


// Detect rotation - do it later
		nFaceRotation = g_findDominantOrientation(&pPVS->m_imgBW, rect, 5); 	


		ptL.y -= 3.14/180.0*nEye*nFaceRotation;
		ptR.y += 3.14/180.0*nEye*nFaceRotation;
		CFace::draw_Glasses(&pPVS->m_imgOut, ptL, ptR, 0xff00aa);

		sprintf(strMessage, "%3i degrees rotated",  nFaceRotation);		
		OnDraw_LogWindow(pDC, strMessage, set_window(1,2), 5);
////////////////////////////////

		PVI_BLOB blobC1, blobC2, blobM;

		bool resC1 = isThereAnything(&pPVS->m_chanColour.m_imbSkinYCrCb, &rect, &blobC1);
//		bool resC1 = pPVS->m_chanMotion.isThereAnything(&pPVS->m_chanColour.m_imbSkinYCrCb, &rect, &blobC1);
		bool resC2 = isThereAnything(&pPVS->m_chanColour.m_imbSkinUCS, &rect, &blobC2);
		bool resM = isThereAnything(&pPVS->m_chanMotion.m_imbFG, &rect, &blobM);
//		bool resM = pPVS->m_chanMotion.isThereSomething(&pPVS->m_chanMotion.m_imbFG, &rect, &blobM);
						
/*		if ( ! resC1 )
			// ( (blobC1.N > rect0->width) && (blobC1.N > rect0->width) ))
		{
			sprintf(strMessage, "No skin detected" );		
			OnDraw_LogWindow(pDC, strMessage, set_window(1,3), 0);
			return;
		}
		else
		{
			sprintf(strMessage, "Skin detected    " );		
			OnDraw_LogWindow(pDC, strMessage, set_window(1,3), 0);
		}
*/		
		
		if ( !resM )
		{
			sprintf(strMessage, "No motion detected" );		
			OnDraw_LogWindow(pDC, strMessage, set_window(3,3), 0);
			return;
		}
		else
		{
//			sprintf(strMessage, "Motion detected    " );		
//			OnDraw_LogWindow(pDC, strMessage, set_window(3,3), 0);
			break;
		}
	}
	
	// A uzh kakoj iz nih m_face[f] use decide sam	
	int nFaceSelected = nFace-1;

	// Load into a 0th face Selected Face / MUST PASS pPVS->m_imgBW image not Colour image !!
	pPVS->m_face[0].loadFace(&pPVS->m_imgBW, pPVS->m_fd.getLocationOfFace(nFaceSelected), nFaceRotation); 
		
			

	frIds->loadVectorsFromFace(& pPVS->m_face[0]);
//	Maybe I should just : frIds->loadVectorsFromFace(&pPVS->m_imgBW, pPVS->m_fd.getLocationOfFace(nFaceSelected));
	


	cvRectangle(pPVS->m_face[0].m_imgFaceDetected.getIplImage(), 
		cvPoint( 2+a2i( frIds->m_sItoCrop[0] ),2+a2i( frIds->m_sItoCrop[1] ) ), 
		cvPoint( 26-a2i( frIds->m_sItoCrop[2] ), 26-a2i( frIds->m_sItoCrop[3] ) ), 
		0xffffaa, 2 );

	pPVS->m_face[0].m_imgFaceDetected.draw(pDC->m_hDC, 
		set_window(0,1, 2*pPVS->m_face[0].m_imgFaceDetected.getWidth(),0,0)); 

	if (!m_bTraceMode)
	{

/*	frIds->m_imgNewFace.draw(pDC->m_hDC, set_window(4,1, 24*3));
	frIds->m_imgQueryFace.draw(pDC->m_hDC, set_window(4,1, 24*3, 1,0));
*/
	frIds->getSasImage(frIds->m_S, 0)->draw(pDC->m_hDC, set_window(1,1, 60));
	frIds->getSasImage(frIds->m_S, 1)->draw(pDC->m_hDC, set_window(2,1, 60));
	frIds->getSasImage(frIds->m_S, 2)->draw(pDC->m_hDC, set_window(3,1, 60));

	frIds->getYasImage(frIds->m_Y, 0)->draw(pDC->m_hDC, set_window(1,1, 60, 1,0));
	frIds->getYasImage(frIds->m_Y, 1)->draw(pDC->m_hDC, set_window(2,1, 60, 1,0));
	frIds->getYasImage(frIds->m_Y, 2)->draw(pDC->m_hDC, set_window(3,1, 60, 1,0));
	}

		
	pPVS->m_imgOut.draw(pDC->m_hDC, set_window(1,0), false, false);
	pPVS->m_chanColour.m_imbSkinYCrCb.draw(pDC->m_hDC, set_window(0,3)); 	
	pPVS->m_chanColour.m_imbSkinUCS.draw(pDC->m_hDC, set_window(1,3)); 
	pPVS->m_chanMotion.m_imbFG.draw(pDC->m_hDC, set_window(2,3)); 
	pPVS->m_chanMotion.m_imb_dI.draw(pDC->m_hDC, set_window(3,3)); 
//	pPVS->m_chanMotion.m_imb_ddI.draw(pDC->m_hDC, set_window(4,3)); 


	nNameTag = pDoc->m_gui.slider4;//-1;

// ************************************************************************************************

	if (!pDoc->m_gui.check_start)  // It has to be checked to start the process
		return;
	
//NB:  frIds->m_pinn.m_dFade, frIds->m_pinn.m_D zadautsq by sliders; !!!
		frIds->m_pinn.m_D = 1.0;


#if 1 // later

// ************************************************************************************************
	if (pDoc->m_gui.check1 == false)  // memorize
// ************************************************************************************************
	{
		bool bStoreOnDisk = m_bTraceMode;

		// use  every Nth frame
		if (pDoc->m_PVSystem.m_nFrame % (pDoc->m_gui.slider2+1) != 0)
		{
			sprintf(strMessage, "... skipped (%i/%i)   ", 
				pDoc->m_PVSystem.m_nFrame % (pDoc->m_gui.slider2+1), pDoc->m_gui.slider2+1);
			OnDraw_LogWindow(pDC, strMessage, set_window(0,1), 5);
			return;
		}


//	frIds->LoadID(nNameTag);
		frIds->setIDto1(nNameTag);

		int res = frIds->m_pinn.putInMemory(frIds->m_Y);
//** 
//		nn->MemorizeV_as(nNameTag);

		frIds->m_arrImgStoredFaces[frIds->m_nStoredFaces++ % FR_MAX_FACES].copy(&pPVS->m_face[0].m_imgFaceDetected);


//		if (m_bShowPINNwell )    
		{
			frIds->m_pinn.analyzeMemory();

			sprintf(strMessage,  "Brain model: N=%i D=%4.2f,  C: %4.2f %4.2f %4.2f", 
			 frIds->m_pinn.m_N,	frIds->m_pinn.m_D, 
			 frIds->m_pinn.m_dAveCii, frIds->m_pinn.m_dAveCij2, frIds->m_pinn.m_dAveAbsCij2);
		}
/*		else
			sprintf(strMessage,  "Brain model: N=%i", 
			 frIds->m_pinn.m_N,	frIds->m_pinn.m_D, 
			 frIds->m_pinn.m_dAveCii, frIds->m_pinn.m_dAveCij2, frIds->m_pinn.m_dAveAbsCij2);
*/	
		OnDraw_LogWindow(pDC, strMessage, set_window(2,0,2,0), 0);

		sprintf(strMessage,"Memorize: ID=%i, E:%.1f    ", 
			nNameTag, frIds->m_pinn.m_E); // %s,  strNameTag,
		OnDraw_LogWindow(pDC, strMessage, set_window(2,0), 1);

		if (res==0)
			sprintf(strMessage, " similar face already memorized");
		else
			sprintf(strMessage, " face memorized (#%i) !        ", frIds->m_pinn.m_M); // frIds->m_nStoredFaces-1);

		OnDraw_LogWindow(pDC,strMessage, set_window(2,0), 2);
//		for(int i=2;i<=20;i++)
//			OnDraw_LogWindow(pDC, "                                   ", set_window(1,0,0,1), i+2);				


		if ( (! pDoc->m_gui.check2) && (! pDoc->m_gui.check_nonstop )) // i.e. from video
		{
			// Uncheck checkbox (switch to recognition mode)
			CMainFrame *pMainFrm = (CMainFrame*)GetParentFrame();
			CButton* check = (CButton*)((pMainFrm->GetDlgItem(IDD_DIALOGBAR1))->GetDlgItem(IDC_CHECK1));
			check->SetCheck(BST_CHECKED);
			pDoc->m_gui.check1 = true; 
			pMainFrm->UpdateData(FALSE);
		}

	}
// ************************************************************************************************
	else  // recognize
// ************************************************************************************************
	{
		//		frIds->m_imgQueryFace.draw(pDC->m_hDC, set_window(0,1));
		//		OnDraw_LogWindow(pDC, "Stimulus", set_window(0,1), 0);
		
// move this line to memorization! Nope... it does not work this way...
		frIds->m_pinn.setMemoryToDormantState();

// Influence of past history

//		for(int k=0; k<pDoc->nLastFrameIDs; k++)
//			frIds->setIDto1(pDoc->nLastFrameID[k]);  // this is too much influence/
		
		//** we should 
		//** frIds->setYofID(pDoc->nLastFrameID[k], 0.15);


		frIds->m_pinn.retrieveFromMemory(frIds->m_Y, &frIds->m_Y);
//		frIds->m_pinn.retrieveFromMemory();
		
			
		nNameTag = 0;
		
		char word[5][100], strY[100]="", strS[100]="    ";
		int nPosY=0;
		strcpy(word[4],"");

		pDoc->nLastFrameIDs=0;

		for(int i=0; i<frIds->m_nIDs; i++)
		{
			int nY = frIds->getYofID(i);
//			sprintf (word[1], "%+1i", (i+1)*nY);

			if (nY > 0)
			{
				sprintf (word[1], "1 ", (i+1)*nY);

				sprintf (word[2], "*%2.0f", (frIds->getSofID(i))*10); //
				nPosY++;
				pDoc->nLastFrameID[ pDoc->nLastFrameIDs++ ] = i; 
				sprintf (word[3], "%i ", i);

				strcat(word[4], word[3]);
			}
			else
			{
				sprintf (word[1], "_ ", (i+1)*nY);
//				sprintf (word[2], " ");
				sprintf (word[2], " %2.0f", ((-1)*frIds->getSofID(i))*10);
			}
			strcat(strY, word[1]);
			strcat(strS, word[2]);
		}
		OnDraw_LogWindow(pDC, word[4], set_window(0,0), 0);
		sprintf(strMessage, "Neural response: %s    ",word[4]);
		OnDraw_LogWindow(pDC, strMessage, set_window(1,0), 0);
			
		sprintf(strMessage, "  It=%i, E=%3.0f   ",
			frIds->m_pinn.m_nIter, frIds->m_pinn.m_E);
		OnDraw_LogWindow(pDC, strMessage, set_window(1,0), 1);
//		OnDraw_LogWindow(pDC, strY, set_window(1,0), 1);
//		OnDraw_LogWindow(pDC, strS, set_window(3,3,1,0), 1);

		
		int nStack= pDoc->m_gui.slider1+1;  

/*		for(i=0;i<nStack;i++)
		{
			strcpy(pDoc->sStackY[i],""); 
			strcpy(pDoc->sStackS[i],""); 
		}
*/		strcpy(pDoc->sStackY[nStack],strY); 
		strcpy(pDoc->sStackS[nStack],strS); 

// nNameTag !!!!!!!!!!! decode and display!

		int nResConsistent=0;
//		int nTagOverTime = nNameTag;
//			char sTags[30];
//			char sTagsNum[11];
//			int nIDs=0;

		for(i=1;i<=nStack;i++)
		{
//			if (pDoc->nStack[i-1]==pDoc->nStack[i]) // nBestMatch)  
//				nResConsistent ++;

			pDoc->nStack[i-1]=pDoc->nStack[i];
			strcpy(pDoc->sStackY[i-1],pDoc->sStackY[i]); 
			strcpy(pDoc->sStackS[i-1],pDoc->sStackS[i]); 

			sprintf(strMessage, "%s  ", pDoc->sStackY[i-1]); // pDoc->nStack[i-1]);
			OnDraw_LogWindow(pDC, strMessage, set_window(1,0), i+1);

			if (m_bShowPINNwell )    
			{
				sprintf(strMessage, "%s ", pDoc->sStackS[i]); // pDoc->nStack[i-1]);
				OnDraw_LogWindow(pDC, strMessage, set_window(2,0), i+1);
			}

		}
			

		for(i;i<=20;i++)
		{
			OnDraw_LogWindow(pDC, "                                      ",  set_window(1,0), i+1);				
			OnDraw_LogWindow(pDC, "                                      ",  set_window(2,0), i+1);
		}
		
		
//		if (m_bTraceMode)
		{
			frIds->getSasImage(frIds->m_pinn.m_S, 0)->draw(pDC->m_hDC, set_window(1,1, 24*3, 0,1));
			frIds->getSasImage(frIds->m_pinn.m_S, 1)->draw(pDC->m_hDC, set_window(2,1, 24*3, 0,1));
			frIds->getSasImage(frIds->m_pinn.m_S, 2)->draw(pDC->m_hDC, set_window(3,1, 24*3, 0,1));
			
			frIds->getYasImage(frIds->m_Y, 0)->draw(pDC->m_hDC, set_window(1,1, 24*3, 1,1));
			frIds->getYasImage(frIds->m_Y, 1)->draw(pDC->m_hDC, set_window(2,1, 24*3, 1,1));
			frIds->getYasImage(frIds->m_Y, 2)->draw(pDC->m_hDC, set_window(3,1, 24*3, 1,1));
		}	
		
	}  // end of recognition

	
#endif


// ************************************************************************************************
// end of memorization/recognition
// ************************************************************************************************

// ************************************************************************************************

	pDoc->m_bWorkWithFiles = false;
		
	if (m_bTraceMode == true && pDoc->m_gui.check2)  // if scenario, wait to continue			
//		AfxMessageBox("Next", MB_OK);
		if (MessageBox("Proceed to next face?",NULL,MB_OKCANCEL) != IDOK)
			exit(1);	
//	strcpy(pDoc->m_input.strFile1,"");  
}
	
void CGUIFaceInVideoView::OnDraw_LogWindow(CDC* pDC, char *strMessage, RECT *textRect, int nLine)
{
	textRect->top = textRect->top + 18*nLine;
	
	pDC->SetTextColor(RGB(100,0,100));
	pDC->SetBkMode(OPAQUE);
	pDC->DrawText(strMessage, strlen(strMessage), textRect, DT_LEFT);
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



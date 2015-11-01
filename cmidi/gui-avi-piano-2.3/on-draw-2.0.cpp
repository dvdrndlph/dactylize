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

#define USING_MIDI 1

#define NUM_CONS 1	
// int num_cons=5;
int num_bytes_rec;
char host_name1[1024],host_name2[1024],host_ip[1024];
int port_num,packet_len,max_packet_len,tx_cache_size,rx_cache_size;
char end,is_more;
unsigned char packet[MAX_PACKET_SIZE];
char last_open[10];
char idstring[512];



/////////////////////////////////////////////////////////////////////////////

#define IMAGE_160 160
#define IMAGE_120 120

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define DRIVER_INFO NULL
#define TIME_PROC ((long (*)(void *)) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0)


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



CGUIFaceInVideoView::CGUIFaceInVideoView()
{
	m_nVideoStream = VIDEO_STREAM_CURRENT_FRAME;
	m_bUseClient = false;
	m_bTraceMode = false;
	m_bShowPINN = true;
	m_bShowPINNwell = false;
	m_bRecognizeAlways = false;

	int q;
	for (q=0; q < 200; q++)
		keyDown[q] = false;

/*	m_bStretch = true; // false;
	m_bFlip = false;
	m_bMirror = false;*/

	
	//--initial set up network--

//	OnDraw_LogWindow(pDC, "Starting up TCP server", set_window(0,0), 0);
//	OnDraw_LogWindow(pDC, idstring, set_window(0,0), 1);

	char str[400];
	sprintf(str, "Starting up TCP server. \n This computer's IP=<%s> port:%d", host_ip, PORT_NUM);
//	MessageBox(str,	MB_OK); 


	/////ARJUN JUNE 2005/////

	m_nHandsDetectThreshold = 50;
	m_bColourAverage = false;
	m_nColourThreshold = 50;
	m_bEdgeDetect = false;
	m_nEdgeDetectThreshold = 3;
	m_bFingerDetect = false;
	runOnce = 0;
	

	interval=0;

	if (USING_MIDI == 1)
		m_Midi = new CPianoMidi();

	m_nOctave = 0;
	m_nKeyOffset = 0;
	m_bMidiCheck=false;

	/////END ARJUN//////

}

//UPDATES CONTROL VALUES
void CGUIFaceInVideoView::UpdateControlValues(CPianoPlaying *pp)
{
	//Copy Control Values
	pp->m_nHandsDetectThreshold = m_nHandsDetectThreshold;
	pp->m_bColourAverage = m_bColourAverage;		
	pp->m_nColourThreshold = m_nColourThreshold;
	pp->m_bEdgeDetect = m_bEdgeDetect;
	pp->m_nEdgeDetectThreshold = m_nEdgeDetectThreshold;
	pp->m_bDetectHands = m_bDetectHands;
	pp->m_bFingerDetect = m_bFingerDetect;
}


/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoView drawing
////////////////////////////////////////////////////////////////

void CGUIFaceInVideoView::OnDraw(CDC* pDC)
{ 
	

	CGUIFaceInVideoDoc* pDoc = GetDocument();    

	ASSERT_VALID(pDoc);
	if (pDoc == NULL)
		return;

	CIplImage *pImgIn;
	CvPoint *pPointL, *pPointR;
	int nNameTag, nBestMatch; 
	char strMessage[200];
	

	CPerceptualVisionSystem *pPVS = pDoc->getPVS();
	
	bool bRes = pPVS->initializeNextFrame();
	if (bRes == false)
 		return;		// if want to show it but not process it, change it.

	sprintf(strMessage, "%i msec", pPVS->m_tBetweenFrames);
	OnDraw_LogWindow(pDC, strMessage, set_window(0,0), 0);


	pDoc->m_gui.check_start = pDoc->m_gui.check_start;

	if (pDoc->m_gui.check_start == true)
	{
		int z;
		for (z=0; z < pPVS->m_pianoPlaying.m_nNumHands; z++)
			pPVS->m_pianoPlaying.m_Hand[z].destroy();

		pPVS->m_pianoPlaying.m_nNumHands = 0;

	}


	////////////AVI RECORDING//////////////////
	if (pDoc->m_recordCheck == true)  // For saving video file made of video frames
	{
		pPVS->m_imgIn.draw(pDC->m_hDC, set_window(0,3), false, false);
		OnDraw_LogWindow(pDC, "Start recording..", set_window(0,3), 1);

		pPVS->m_bRecording = true;
		if (pPVS->m_bRecordingStarted == false) 
		{

				// open a dialog box.
			TCHAR szFilter[] = _T ( "Movie Files (*.avi)" );
			CFileDialog *dlg = new CFileDialog( FALSE, _T("avi"), _T("*.avi"), OFN_ENABLESIZING | OFN_OVERWRITEPROMPT, szFilter, (CWnd*)this);
			char  strAviFullName[200], strAviLocalName[200];


			if(dlg->DoModal() == IDOK)
				strcpy(strAviFullName, dlg->GetPathName());

			pPVS->m_pCvVideoWriter = NULL;
			pPVS->m_pCvVideoWriter = cvCreateVideoWriter(strAviFullName, 
			// Uncompressed
			CV_FOURCC('D','I','B',' '), 10.0, cvSize(160,120) );

			pPVS->m_bRecordingStarted = true;// Open AVI file
		}

		//Add frame to AVI
		if (pPVS->m_pCvVideoWriter) 
			cvWriteFrame( pPVS->m_pCvVideoWriter, pPVS->m_imgIn.getIplImage() );

		return; 
	}
	else
	{
		pPVS->m_bRecording = false;
		if (pPVS->m_bRecordingStarted == true) 
		{
			pPVS->m_bRecordingStarted = false; // Close AVI
			if (pPVS->m_pCvVideoWriter)
				cvReleaseVideoWriter( &pPVS->m_pCvVideoWriter );
		}
	}
	////////////////END AVI RECORDING////////////////
	


///////////////////////
//	return;   //////// debuging.....
///////////////////////


/***************************************************************************************/
/*** Grab data from MIDI **************************************************/
/***************************************************************************************/

	int q, keyValue, octave;
	bool keyChange[200]; // bKeyChanged[nTotalPianoKeys]	//To indicate Changes only
								//For printing Purposes
	if (m_bMidiCheck==false)
	{
		m_Midi->clearAllEvents();
	}
	//DO MIDI INFORMATION
	if (USING_MIDI == 1 && m_bMidiCheck==true)
	{
		//Get MIDI Events
		m_Midi->getAllEvents();
		//Update User Selected Offsets
		pPVS->m_pianoPlaying.m_Piano.UpdateOctave(m_nKeyOffset, m_nOctave);

	
		
		//No Changes to Start
		for (q=0; q < 200; q++)
			keyChange[q] = false;
		
		//Go Through All MIDI events
		for (q=0; q < m_Midi->numNotes; q++)
		{
			//If Key Has Been Pressed, set it as True
			if (keyDown[int(m_Midi->noteArray[q])] == true)
			{
				keyDown[int(m_Midi->noteArray[q])] = false;
				keyChange[int(m_Midi->noteArray[q])] = true;	//Indicate Change
			}
			//Vice Versa
			else
			{
				keyDown[int(m_Midi->noteArray[q])] = true;
				keyChange[int(m_Midi->noteArray[q])] = true;	//Indicate Change
			}
		}

	}//END MIDI


/***************************************************************************************/
/*** Grab data from video camera / AVI **************************************************/
/***************************************************************************************/


	sprintf(strMessage, "%i: %i (%i) msec        ", pPVS->m_nFrame, pPVS->m_tBetweenFrames, pPVS->m_tBetweenFramesNow);		
//	OnDraw_LogWindow(pDC, strMessage, set_window(0,2), 1);
		

	//Motion Channel Calculations
	pPVS->m_chanMotion.updateImages(&pPVS->m_imgIn);
	pPVS->m_chanMotion.compute_dI();
	pPVS->m_chanMotion.compute_FG(3, 0);


	//////////////////////////PIANO PLAYING SECTION////////////////////////////
	
	//Update Images
	pPVS->m_pianoPlaying.updateImages(&pPVS->m_imgIn);

	//Do Backgorund Initialization && Piano Key (width) Calibration
	if (runOnce == 0)
	{
		pPVS->m_pianoPlaying.setBackgroundImage(&pPVS->m_imgIn);
		runOnce++;
	}

	//Image Adjustment, based on Piano Orientation and Positioning
	pPVS->m_pianoPlaying.AdjustImages();

	//Update Control Values from GUI
	UpdateControlValues(&(pPVS->m_pianoPlaying));

	//Main Piano Playing Function
	pPVS->m_pianoPlaying.createHandsImage();
	m_bDetectHands = false;	//Reset Hand Detecting Flag

/*
// no Filter2D

	CIplImage img1; // , img2; 
	img1.initializeAs(&pPVS->m_imgBW);
//	img2.initializeAs(&pPVS->m_imgBW);

	IplImage *ipl;

	ipl = cvCreateImage( cvGetSize(pPVS->m_imgBW.getIplImage()), IPL_DEPTH_16S , 1 );

	cvAnd(pPVS->m_imgBW.getIplImage(), pPVS->m_pianoPlaying.m_imb_handsImage.getIplImage(), img1.getIplImage());
	cvSobel(img1.getIplImage(), ipl, 1,0,3);

	cvConvert (&ipl, img1.getIplImage());
	img1.draw(pDC->m_hDC, set_window(0,3));
	img1.destroy();

	cvReleaseImage(&ipl);
//	img2.destroy();

*/


#if 1  //<< Clustering


		CIplImage imc, imcClusters;
		imc.initializeAs(&pPVS->m_imgIn);
		imcClusters.initializeAs(&pPVS->m_imgBW);

        cvZero( imc.getIplImage() );

		pImgIn = & pPVS->m_pianoPlaying.m_imb_handsImage; // m_chanColour.m_imbSkinYCrCb; // ;
		CIplImage *pImgOut = & imcClusters;
        cvZero( pImgOut->getIplImage());

//		pImgIn->draw(pDC->m_hDC, set_window(0,3));

		int nClustersDetected, nMaxClusters;

// a)
		PVI_BLOB *blobs = new PVI_BLOB[5];
		nMaxClusters = 4;
		nClustersDetected = detectClusters_wKmeans( pImgIn, pImgOut, nMaxClusters, &nClustersDetected, &blobs);		
		delete blobs;



		colourizeClusters(*pImgOut, imc, nClustersDetected);
		imc.draw(pDC->m_hDC, set_window(0,1));
		sprintf(strMessage, "%i hands", nClustersDetected);
		OnDraw_LogWindow(pDC,strMessage, set_window(0,1), 0);
		
/*


//		nClustersDetected = clusterize_wKmeans( pImgIn, pImgOut, 2, 1, &imc); 
//		imc.draw(pDC->m_hDC, set_window(1,iActiveCamera+1));

// b)
		int nXclose=4, nYclose=4,  nCosine=0;
		nClustersDetected = detectClusters_wProximity( pImgIn, pImgOut, nXclose, nYclose,  nCosine, &imc);
		imc.draw(pDC->m_hDC, set_window(4,0));
		sprintf(strMessage, "%i", nClustersDetected);
		OnDraw_LogWindow(pDC,strMessage, set_window(4,0), 0);

		imc.destroy();
		imcClusters.destroy();

//		if (nClustersDetected > pPVS->m_pianoPlaying.m_nNumHands)
//			m_bDetectHands = true;

  */

#endif //>> Clustering



/***************************************************************************************/
/*** Use MIDI data do detect hand/finger **************************************************/
/***************************************************************************************/

		CvFont cvFont; 
		cvInitFont( &cvFont, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0.0, 2 );
                        				
		CvPoint ptBtmLeft;



// To do :  midi for hand blob detection

	//DO MIDI INFORMATION
	if (USING_MIDI == 1 && m_bMidiCheck==true)
	{

		char keyPressString[200];	//Printable String
		int i=0;					//Print Counter

		numStrings=0;
	
		//FOR each possible MIDI Key
		for (q=0; q < 200; q++)
		{
			//Calculate Key and Octave Values
			keyValue = (q) % 12;
			octave = (q - keyValue)/12;

			//IF Key is ON
			if (keyDown[q] == true)
			{
				//Get Bounding Box of Key
				CvRect myRect = pPVS->m_pianoPlaying.m_Piano.ReturnKeyBounds(keyValue, octave);

				//Select Hand that this Key intersects with
				int selectedHand = pPVS->m_pianoPlaying.SelectHand(&myRect);

				//Highlight Selected Key
				pPVS->m_pianoPlaying.DrawBox(pPVS->m_pianoPlaying.m_imc_displayImage.getIplImage(), myRect.x, myRect.y, myRect.x + myRect.width, myRect.y + myRect.height, 3); 



				//Put Information to String
				if (selectedHand != -1)
				{
					int selectedFinger = pPVS->m_pianoPlaying.m_Hand[selectedHand].SelectFinger(&myRect);

					if (selectedFinger == -1)
						sprintf(keyPressString, "MIDI: %d -> Octave: %d, Key: %d -> Hand: %d", q, octave, keyValue, selectedHand);
					else
					{
						sprintf(keyPressString, "MIDI: %d -> Octave: %d, Key: %d -> Hand: %d, Finger: %d", q, octave, keyValue, selectedHand, selectedFinger+1);
						sprintf(strMessage, "%i", selectedFinger);
						ptBtmLeft = cvPoint(myRect.x, myRect.y-5);
						cvPutText( pPVS->m_pianoPlaying.m_imc_displayImage.getIplImage(), strMessage, ptBtmLeft, &cvFont, CV_RGB(0, 200, 200));
					}
				}
				else
					sprintf(keyPressString, "MIDI: %d -> Octave: %d, Key: %d", q, octave, keyValue);
					
				//IF Key Was JUST Pressed
				if (keyChange[q] == true)
				{
					//Write Information to File
					fprintf(pPVS->m_pianoPlaying.fDataFile, "%s\n", keyPressString);
				}

				//Add String To List
				if (numStrings < 200)
				{
					sprintf(MIDIString[numStrings], "%s", keyPressString); 
					numStrings++;
				}

			}//END IF
			//IF Key is OFF
			else
			{
				//Put Information to String
				sprintf(keyPressString, "Key Value: %d  -> Octave: %d, Key: %d - RELEASED", q, octave, keyValue);
				
				//IF Key Was JUST Released
				if (keyChange[q] == true)
					//Write Information to File
					fprintf(pPVS->m_pianoPlaying.fDataFile, "%s\n", keyPressString);

			}//END ELSE
		}//END FOR Each Key
	}//END MIDI
	



/***************************************************************************************/
/*** Draw all    **************************************************/
/***************************************************************************************/


	//DRAW IMAGES
	pPVS->m_pianoPlaying.m_imc_displayImage.draw(pDC->m_hDC, set_window(1, 0));
	pPVS->m_pianoPlaying.m_imc_backgroundImage.draw(pDC->m_hDC, set_window(2,0));
//	pPVS->m_pianoPlaying.m_imb_handsImage.draw(pDC->m_hDC, set_window(3,0));
//	pPVS->m_pianoPlaying.m_imb_edgeDetectedImage.draw(pDC->m_hDC, set_window(4,0));

	

/*	pPVS->m_pianoPlaying.m_imb_fingerDetectedImage.
		draw(pDC->m_hDC, set_window(1,1));	
	pPVS->m_pianoPlaying.m_imc_displayImage. 
		draw(pDC->m_hDC, set_window(2,1));
	pPVS->m_pianoPlaying.m_imc_backgroundImage. 
		draw(pDC->m_hDC, set_window(3,1));
*/
	pPVS->m_pianoPlaying.m_Piano.boxes.draw(pDC->m_hDC, set_window(1, 1));
	pPVS->m_pianoPlaying.m_Piano.polarizedImage.draw(pDC->m_hDC, set_window(2, 1));
	//pPVS->m_pianoPlaying.m_ORIMAGE.draw(pDC->m_hDC, set_window(3, 1));
	//pPVS->m_pianoPlaying.m_Hand[0].m_edgeImage.draw(pDC->m_hDC, set_window(3, 1));

	
	int f, h;
	if (pPVS->m_pianoPlaying.m_nNumHands > 0)
	{
		h=0;
		if (pPVS->m_pianoPlaying.m_Hand[h].ready == true)
		{
	//		pPVS->m_pianoPlaying.m_Hand[h].m_fingImg[f].draw(pDC->m_hDC, set_window(f, h+2));
			
			for (f=0; f < 5; f++)
			{
			
				pPVS->m_pianoPlaying.m_Hand[h].m_fingImg[f].draw(pDC->m_hDC, set_window(f, h+2));
				//cvSet( (pPVS->m_imgOut.getIplImage()), CV_RGB(0, 100*h, f*50), 
				//		(pPVS->m_pianoPlaying.m_Hand[h].m_fingImg[f].getIplImage()));
			}
		}
	}


	
	char clearString[150];
	int i;
	for (i=0; i < 150; i++)
		clearString[i] = ' ';
	//////////////////////////END ARJUN////////////////////////////

//
//	pPVS->m_imgIn.draw(pDC->m_hDC, set_window(0,0), false, false);
	pPVS->m_imgOut.draw(pDC->m_hDC, set_window(0,0), false, false);



	nNameTag = pDoc->m_gui.slider4;//-1;

// ************************************************************************************************

	if (!pDoc->m_gui.check_start)  // It has to be checked to start the video processing
		return;
	

// ************************************************************************************************

	pDoc->m_bWorkWithFiles = false;
		
	if (m_bTraceMode == true && pDoc->m_gui.check2)  // if scenario, wait to continue			
		if (MessageBox("Proceed to next face?",NULL,MB_OKCANCEL) != IDOK)
			exit(1);	




}
	
void CGUIFaceInVideoView::OnDraw_LogWindow(CDC* pDC, char *strMessage, RECT *textRect, int nLine)
{
	textRect->top = textRect->top + 18*nLine;
	
	pDC->SetTextColor(RGB(100,0,100));
	pDC->SetBkMode(OPAQUE);
	pDC->DrawText(strMessage, strlen(strMessage), textRect, DT_LEFT);
}


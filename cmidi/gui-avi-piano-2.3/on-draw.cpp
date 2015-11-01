// GUIFaceInVideoView.cpp : implementation of the CGUIFaceInVideoView class
// 
// Descpription: PVS project user-end function access example
//
// Author: Dmitry Gorodnichy (Copyright NRC-CNRC 2001-2004)
// 
// Legal warning:
// Any reproduction of these code outside of NRC-CNRC premises is strictly prohibited.

#include "stdafx.h"
#include "GUIFaceInVideo.h"

#include "GUIFaceInVideoDoc.h"
#include "GUIFaceInVideoView.h"

#include "MainFrm.h"
#include "../PVS/window_rect.h"

/////////////////////////////////////////////////////////////////////////////
//---- TCP-IP server ----
#define PORT_NUM 20200
#include <winsock.h>

#define MAX_PACKET_SIZE 100000

#define USING_MIDI 0

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

	

	/////ARJUN JUNE 2005/////

	m_nHandsDetectThreshold = 50;
	m_bColourAverage = false;
	m_nColourThreshold = 50;
	m_bEdgeDetect = false;
	m_nEdgeDetectThreshold = 3;
	m_bFingerDetect = false;
	runOnce = 10;
	

	interval=0;

	if (USING_MIDI == 1)
		m_Midi = new CPianoMidi();

	m_nOctave = 0;
	m_nKeyOffset = 0;
	m_bMidiCheck=false;

	m_pianoPlaying = new CPianoPlaying();
	m_bReadyToUse = false;

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
	bool bStretch=true, bMirror=false, bFlip=false; // default

	bFlip=true; // to show piano as for pianist
	bMirror=true; // to show piano as for pianist

//	OnDraw_LogWindow1(pDC, "STARTING...", pvu::setRECT(0,0), 0);

	CGUIFaceInVideoDoc* pDoc = GetDocument();    


	ASSERT_VALID(pDoc);
	if (pDoc == NULL)
		return;




/***************************************************************************************/
/*** Grab data from video camera / AVI **************************************************/
/***************************************************************************************/


	CIplImage *pImgIn;
	CvPoint *pPointL, *pPointR;
	int nNameTag, nBestMatch; 
	char strMessage[200];
	

	CPVS_WithCapture *pPVS = pDoc->getPVS();
	bool bRes;

		

// Get frame from Capture
#if 1 //New style (with original 360 image saved)
		if( ! pPVS->captureCAM_AVI(false) ) // was (true)
		{
//			OnDraw_LogWindow(pDC, pPVS->m_strLog, pvu::setRECT(0,iActiveCamera+1,1,0), 2);
			return;
		}
		pPVS->initNextFrame(); 
//		pPVS->m_imcOriginal.draw(pDC->m_hDC, pvu::setRECT(1, 0, 1, 1), true);
#else // old style
		if( ! pPVS->initializeNextFrame( true, true) ) // old style
		{
			return;
		}
#endif	

	if (bRes = false)
 		return;		// if want to show it but not process it, change it.

		drawStr(pDC, pPVS->m_strFrameRate, pvu::setRECT(0,5,2,0), 0);		
		sprintf(strMessage, "%i msec", pPVS->m_tBetweenFrames);
		drawStr(pDC, strMessage, pvu::setRECT(0,5), 1);

//////////////////////////// testing......
	

		pPVS->m_imcIn.draw(pDC->m_hDC, pvu::setRECT(0, 0));

		
		pPVS->m_chanColour.updateImages(pPVS->m_imcIn.p); 	

/*	for(int jj = 0; jj < pPVS->m_chanColour.m_nHeight; jj++)
		for(int ii = 0;ii < pPVS->m_chanColour.m_nWidth; ii++)
		{
			pPVS->m_chanColour.m_imcHSV.setPixel(ii, jj, cvScalar(ii*10%255, jj*10%255,0));
		}
*/
		double dT1 = this->m_nHandsDetectThreshold*2.5, dT2 = this->m_nEdgeDetectThreshold;

		sprintf(strMessage, "T1=%i / T2=%i     ", (int)dT1, (int)dT2);
		drawStr(pDC, strMessage, pvu::setRECT(4,2), 1);

 //!!! ipl->plane(0).Iy() and Iy should be sufficient to rectify piano !
		CIpl *ipl;
		for (int p=0; p<3; p++)
		{
			for (int s=0; s<3; s++)
			{
				if (s==0) 			ipl= & pPVS->m_chanColour.m_imcHSV;
				else if (s==1) 	ipl = &pPVS->m_chanColour.m_imcYCrCb;
				else if (s==2) 	ipl = &pPVS->m_chanColour.m_imcUCS;
				else	ipl = &pPVS->m_imcIn;

			(ipl->plane(p)).draw(pDC->m_hDC, pvu::setRECT(s, p+2),pvu::i2s(p+100*s));
			ipl->plane(0).Iy().draw(pDC->m_hDC, pvu::setRECT(3+s, p+2),"Ix");
//			(ipl->plane(0).laplacianed()<dT1).draw(pDC->m_hDC, pvu::setRECT(3+s, p+3),"Laplace<T");

//			((ipl->plane(p)%dT1)<dT2).draw(pDC->m_hDC, pvu::setRECT(3, p+2),pvu::i2s(p));
			}
		}




//== to detect black keys
		double dT =dT1;

//		CIpl ipl1; ipl1.destroy(); this leaks the memory !!!


		pPVS->m_chanColour.m_imbSkinHSV = ((pPVS->m_chanColour.m_imcUCS.plane(0)).laplacianed() < dT ); //.close();
		pPVS->m_chanColour.m_imbSkinUCS = (pPVS->m_chanColour.m_imcUCS.plane(1) < dT ); // .close();
		pPVS->m_chanColour.m_imbSkinYCrCb = (pPVS->m_chanColour.m_imcUCS.plane(2) < dT ); // .close();

		pPVS->m_chanColour.m_imbSkinHSV = pPVS->m_chanColour.m_imbSkinHSV && pPVS->m_chanColour.m_imbSkinUCS && pPVS->m_chanColour.m_imbSkinYCrCb;

//		pPVS->m_chanColour.m_imbSkinHSV.draw(pDC->m_hDC, pvu::setRECT(5, 0+1),"MY RESULT");

		pPVS->m_chanColour.m_imcHSV.draw(pDC->m_hDC, pvu::setRECT(0, 0+1),"HSV MY");
		pPVS->m_chanColour.m_imcUCS.draw(pDC->m_hDC, pvu::setRECT(1, 0+1), "UCS");
		pPVS->m_chanColour.m_imcYCrCb.draw(pDC->m_hDC, pvu::setRECT(2, 0+1), "YCrCb");
//	  cvCvtColor( pPVS->m_imcIn.p, pPVS->m_chanColour.m_imcHSV.p, CV_BGR2YCrCb);
//		pPVS->m_chanColour.m_imcYCrCb.draw(pDC->m_hDC, pvu::setRECT(5, 0+1));
	  cvCvtColor( pPVS->m_imcIn.p, pPVS->m_imcIn.p, CV_BGR2HSV);
		pPVS->m_imcIn.draw(pDC->m_hDC, pvu::setRECT(3, 0+1), "HSC(OpenCV)");

		return;

		pPVS->m_chanMotion.updateImages(pPVS->m_imcIn.p); 		
	CMotionChannel *pMotion = & pPVS->m_chanMotion; // m_chanMotion

	
#if 1 //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Workable: imbFG = (imfgI % imfgEI) > imfgDI; //(imfgDI*1.1);  OR imbFG = (!imbFG) && ( (imfgI % imfgEI) > imfgDI );
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Way 1: Use FG

//		CvRect rectNewZoom=cvRect(blobMotion.x, ...)

//		pvu::drawRectangle(pPVS->m_imcOri160.getIplImage(), pPVS->m_rZoomSuggested.rect(), pvu::int2colour(0), 2 );
//		pvu::drawRectangle(pPVS->m_imcOri160.getIplImage(), pPVS->m_rZoomNew.rect(), pvu::int2colour(1), 2 );
//		pvu::drawRectangle(pPVS->m_imcOri160.getIplImage(), pPVS->m_rZoom.rect(), pvu::int2colour(2), 2 );
//		pPVS->m_imcOri160.draw(pDC->m_hDC, pvu::setRECT(2, 0, 2));
//		pPVS->m_imcIn.draw(pDC->m_hDC, pvu::setRECT(1, 0, 2));


int 	iActiveCamera=0;
	
	pMotion->imb_dI.draw(pDC->m_hDC, pvu::setRECT(4, iActiveCamera));
	drawStr(pDC, "dI", pvu::setRECT(4,iActiveCamera), 0);
	
	pMotion->m_imbBG.draw(pDC->m_hDC, pvu::setRECT(5, iActiveCamera));
	drawStr(pDC, "My MHI", pvu::setRECT(5,iActiveCamera), 0);	
//	pMotion->m_imbFG.draw(pDC->m_hDC, pvu::setRECT(6, iActiveCamera));
//	drawStr(pDC, "FG:MHI>T", pvu::setRECT(6,iActiveCamera), 0);// Would be nice	(~pMotion->imfgEI).draw(pDC->m_hDC, pvu::setRECT(1, iActiveCamera), "BG");

	
	(~pMotion->imfgEI).draw(pDC->m_hDC, pvu::setRECT(1, iActiveCamera));
	drawStr(pDC, "BG", pvu::setRECT(1,iActiveCamera), 0);

// DI (and EI?) are changed too quickly to a moving object... <-- FIX IT. Should be where mask = MHI>0, 
	(~(pMotion->imfgDI*10)).draw(pDC->m_hDC, pvu::setRECT(6, iActiveCamera));
	drawStr(pDC, "DI", pvu::setRECT(0,iActiveCamera+1), 0); 

	pMotion->imbFG.draw(pDC->m_hDC, pvu::setRECT(2, iActiveCamera));
	drawStr(pDC, "FG", pvu::setRECT(2,iActiveCamera), 0);
//	pMotion->imbFG = (pMotion->imfgI % pMotion->imfgEI) >	pMotion->imfgDI*(dAAA));
	

	
//	pMotion->imgMask = (pMotion->bufimg[pMotion->m_nCurrent] % pMotion->bufimg[pMotion->m_nLast] );
	pMotion->imgMask  = pMotion->bufimg[pMotion->m_nCurrent].detectCornersByAdaptiveThreshold(5);
	pMotion->imgMask.draw(pDC->m_hDC, pvu::setRECT(0, iActiveCamera+1));
	drawStr(pDC, "%", pvu::setRECT(0,iActiveCamera+1), 0);



#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Step 3b. Test arjun's finger detection - doesn't show good results
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
	CIplImage tempMask;
	tempMask.initialize(160, 120, 8);
	cvErode(pPVS->m_chanColour.m_imbSkinYCrCb.getIplImage(), tempMask.getIplImage(), NULL, 1);
	cvDilate(pPVS->m_chanColour.m_imbSkinYCrCb.getIplImage(), tempMask.getIplImage(), NULL, 2);
	tempMask.draw(pDC->m_hDC, pvu::setRECT(7,iActiveCamera+3));// backprojected

	//	tempMask.copy(&pPVS->m_chanColour.m_imbSkinYCrCb);

	for (i=0; i < 5; i++)
	{
		//Initialize Individual Finger Masks
		m_hand.m_fingImg[i].initialize(160, 120, 8);
	}

		//Next Probable Finger is marked by two edges
		m_hand.MarkNextProbableFinger(&pPVS->m_imcIn, &g_img160x120toDraw, &tempMask);
	g_img160x120toDraw.draw(pDC->m_hDC, pvu::setRECT(0, iActiveCamera+4));	
	
		m_hand.CreateFingerBlob(&g_img160x120toDraw);
	g_img160x120toDraw.draw(pDC->m_hDC, pvu::setRECT(1, iActiveCamera+4));		

		//Make Sure Finger Falls within Hand Mask
		cvAnd(g_img160x120.p, g_img160x120toDraw.getIplImage(),g_img160x120toDraw.getIplImage(), NULL);
	g_img160x120toDraw.draw(pDC->m_hDC, pvu::setRECT(2, iActiveCamera+4));		

		//Add to Resulting Image
		cvOr(g_img160x120.p, g_img160x120toDraw.getIplImage(),g_img160x120.p, NULL);
	g_img160x120.draw(pDC->m_hDC, pvu::setRECT(3, iActiveCamera+4));		

	tempMask.destroy();
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	pPVS->m_bPlayBackwards = pDoc->m_gui.check_intensity;

	if (pDoc->m_gui.check_start == true)
	{
		int z;
		for (z=0; z < this->m_pianoPlaying->m_nNumHands; z++)
			this->m_pianoPlaying->m_Hand[z].destroy();

		this->m_pianoPlaying->m_nNumHands = 0;

	}


	////////////AVI RECORDING//////////////////
	if (pDoc->m_recordCheck == true)  // For saving video file made of video frames
	{
		pPVS->m_imcIn.draw(pDC->m_hDC, pvu::setRECT(0,3), bStretch, bMirror, bFlip);
		OnDraw_LogWindow(pDC, "Start recording..", pvu::setRECT(0,3), 1);

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
			cvWriteFrame( pPVS->m_pCvVideoWriter, pPVS->m_imcIn.p );

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



	//////////////////////////PIANO PLAYING SECTION////////////////////////////

	



	//Update Images
	this->m_pianoPlaying->updateImages(&pPVS->m_imcOriginal160);
//	m_bReadyToUse = true;

	//Do Backgorund Initialization && Piano Key (width) Calibration
	if (this->runOnce == 0)
	{
		this->m_pianoPlaying->setBackgroundImage(&pPVS->m_imcOriginal160,  pDoc->m_gui.slider4); 
		m_bReadyToUse = true;
 	}
	runOnce++;
	//Update Control Values from GUI
	UpdateControlValues((this->m_pianoPlaying));

	//Image Adjustment, based on Piano Orientation and Positioning
	if (m_bReadyToUse == true)
	{
		this->m_pianoPlaying->AdjustImages();

		//Main Piano Playing Function
		this->m_pianoPlaying->createHandsImage();
		m_bDetectHands = false;	//Reset Hand Detecting Flag
	}
/*
// no Filter2D

	CIplImage img1; // , img2; 
	img1.initializeAs(&pPVS->m_imgBW);
//	img2.initializeAs(&pPVS->m_imgBW);

	IplImage *ipl;

	ipl = cvCreateImage( cvGetSize(pPVS->m_imgBW.getIplImage()), IPL_DEPTH_16S , 1 );

	cvAnd(pPVS->m_imgBW.getIplImage(), this->m_pianoPlaying.m_imb_handsImage.getIplImage(), img1.getIplImage());
	cvSobel(img1.getIplImage(), ipl, 1,0,3);

	cvConvert (&ipl, img1.getIplImage());
	img1.draw(pDC->m_hDC, pvu::setRECT(0,3), bStretch, bMirror, bFlip);
	img1.destroy();

	cvReleaseImage(&ipl);
//	img2.destroy();

*/


#if 0  //<< Clustering, using image subtracted from background image 
		CIplImage imc, imcClusters;
		imc.initializeAs(&pPVS->m_imcIn);
		imcClusters.initializeAs(&pPVS->m_imgBW);

        cvZero( imc.getIplImage() );

		pImgIn = & this->m_pianoPlaying->m_imb_handsImage; // m_chanColour.m_imbSkinYCrCb; // ;
		CIplImage *pImgOut = & imcClusters;
        cvZero( pImgOut->getIplImage());

//		pImgIn->draw(pDC->m_hDC, pvu::setRECT(0,3));
		int nClustersDetected, nMaxClusters;

		PVI_BLOB *blobs = new PVI_BLOB[5];
		nMaxClusters = 4;
		nClustersDetected = pvu::detectClusters_wKmeans( pImgIn, pImgOut, nMaxClusters, &nClustersDetected, &blobs);		
		delete blobs;

		pvu::colourizeClusters(*pImgOut, imc, nClustersDetected);
		imc.draw(pDC->m_hDC, pvu::setRECT(0,1), bStretch, bMirror, bFlip);
		sprintf(strMessage, "%i hands", nClustersDetected);
		OnDraw_LogWindow(pDC,strMessage, pvu::setRECT(0,1), 0);		
#endif //>> Clustering

#if 0 // 1. use motion to initialize colour tracking. 2 detect hands by colour -  INSTEAD OF BG subtraction

		pPVS->m_chanMotion.updateImages(&pPVS->m_imcOriginal);
		pPVS->m_chanMotion.compute_dI();

		pPVS->m_chanColour.updateImages(&pPVS->m_imcIn);		


		// once in a while recompute skin model
// pPVS->m_fd.m_nInFrame % ...

				CPvsTracker *ft = & pPVS->m_tracker[0];
				
				if ( ft->m_nTrainingColoursCount < 10 ) // 
//				if ( ft->m_nTrainingColoursCount < 5 && (pPVS->m_nFrame % (pPVS->m_fd.m_nFramesToSkip + 1) == 0) ) // 
				{   
					ft->m_bAccumulate = true; 
					ft->m_nTrainingColoursCount++; 
					ft->learnColour (&pPVS->m_chanColour, &pPVS->m_user.m_fd_rectFaceDetected, NULL, PVS_OBJECT_SKIN, PVS_COLOUR_HCr);
				} 
				else
				{
					ft->detectColour(&pPVS->m_chanColour); 
//			pPVS->m_user.setNewPosition(&pPVS->m_tracker[0].m_rectLast, pPVS->m_imcIn.getWidth(), pPVS->m_imcIn.getHeight() );
					ft->m_imgSkin.draw(pDC->m_hDC, set_window(1+o,0));// backprojected
				}


#endif //>> Colour Motion

// The program can't be killed! - Find out why! -> move MIDI part elsewhere

/***************************************************************************************/
/*** Grab data from MIDI **************************************************/
/***************************************************************************************/



	int q, keyValue, octave;
	bool keyChange[200]; // bKeyChanged[nTotalPianoKeys]	//To indicate Changes only
								//For printing Purposes
	if (m_bMidiCheck==false && USING_MIDI == 1)
	{
		m_Midi->clearAllEvents();
	}
	//DO MIDI INFORMATION
	if (USING_MIDI == 1 && m_bMidiCheck==true)
	{
		//Get MIDI Events
		m_Midi->getAllEvents();
		//Update User Selected Offsets
		this->m_pianoPlaying->m_Piano->UpdateOctave(m_nKeyOffset, m_nOctave);

	
		
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
/*** Use MIDI data do detect hand/finger **************************************************/
/***************************************************************************************/



		CvFont cvFont; 
		cvInitFont( &cvFont, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
                        				
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
				CvRect myRect = this->m_pianoPlaying->m_Piano->ReturnKeyBounds(keyValue, octave);

				//Select Hand that this Key intersects with
				int selectedHand = this->m_pianoPlaying->SelectHand(&myRect);

				//Highlight Selected Key
				this->m_pianoPlaying->DrawBox(this->m_pianoPlaying->m_imc_displayImage.getIplImage(), myRect.x, myRect.y, myRect.x + myRect.width, myRect.y + myRect.height, 3); 



				//Put Information to String
				if (selectedHand != -1) 
				{
					int selectedFinger = this->m_pianoPlaying->m_Hand[selectedHand].SelectFinger(&myRect);

					if (selectedFinger == -1)
						sprintf(keyPressString, "MIDI: %d -> Octave: %d, Key: %d -> Hand: %d", q, octave, keyValue, selectedHand);
					else
					{
						sprintf(keyPressString, "MIDI: %d -> Octave: %d, Key: %d -> Hand: %d, Finger: %d", q, octave, keyValue, selectedHand, selectedFinger+1);
						sprintf(strMessage, "%i", selectedFinger+1);
						CIplImage tempImage;
						tempImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 24);
						tempImage.copyRotated(&m_pianoPlaying->m_imc_displayImage, 180);

//						ptBtmLeft = cvPoint(IMAGE_WIDTH - myRect.x, myRect.y-10); ///// 
						ptBtmLeft = cvPoint(IMAGE_WIDTH - myRect.x, myRect.y+10); ///// 

						//cvPutText( this->m_pianoPlaying.m_imc_displayImage.getIplImage(), strMessage, ptBtmLeft, &cvFont, CV_RGB(0, 200, 200));
						cvPutText( tempImage.getIplImage(), strMessage, ptBtmLeft, &cvFont, CV_RGB(0, 200, 200));

//						pvu::drawText(tempImage.getIplImage(), strMessage, ptBtmLeft);
						m_pianoPlaying->m_imc_displayImage.copyRotated(&tempImage, 180);
					}
				}
				else
					sprintf(keyPressString, "MIDI: %d -> Octave: %d, Key: %d", q, octave, keyValue);
					
				//IF Key Was JUST Pressed
				if (keyChange[q] == true)
				{
					//Write Information to File
					fprintf(this->m_pianoPlaying->fDataFile, "%s\n", keyPressString);
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
					fprintf(this->m_pianoPlaying->fDataFile, "%s\n", keyPressString);

			}//END ELSE
		}//END FOR Each Key
	}//END MIDI
	




/***************************************************************************************/
/*** Draw all    **************************************************/
/***************************************************************************************/


	//DRAW IMAGES
	if (m_bReadyToUse == true)
	{
	this->m_pianoPlaying->m_imc_displayImage.draw(pDC->m_hDC, pvu::setRECT(0, 0), bStretch, false, false);
	this->m_pianoPlaying->m_imc_displayImage.draw(pDC->m_hDC, pvu::setRECT(1, 1), bStretch, bMirror, bFlip);
	this->m_pianoPlaying->m_imc_backgroundImage.draw(pDC->m_hDC, pvu::setRECT(2,0), bStretch, bMirror, bFlip);
	this->m_pianoPlaying->m_imb_handsImage.draw(pDC->m_hDC, pvu::setRECT(3,0), bStretch, bMirror, bFlip);
	this->m_pianoPlaying->m_imb_edgeDetectedImage.draw(pDC->m_hDC, pvu::setRECT(4,0), bStretch, bMirror, bFlip);

	

/*	this->m_pianoPlaying.m_imb_fingerDetectedImage.
		draw(pDC->m_hDC, pvu::setRECT(1,1));	
	this->m_pianoPlaying.m_imc_displayImage. 
		draw(pDC->m_hDC, pvu::setRECT(2,1));
	this->m_pianoPlaying.m_imc_backgroundImage. 
		draw(pDC->m_hDC, pvu::setRECT(3,1));
*/
//	this->m_pianoPlaying->m_Piano->boxes.draw(pDC->m_hDC, pvu::setRECT(3, 1), bStretch, bMirror, bFlip);
//	this->m_pianoPlaying.m_Piano.polarizedImage.draw(pDC->m_hDC, pvu::setRECT(2, 1), bStretch, bMirror, bFlip);
	//this->m_pianoPlaying.m_ORIMAGE.draw(pDC->m_hDC, pvu::setRECT(3, 1), bStretch, bMirror, bFlip);

	//****CHANGED JAN 8, 2006****//
	//Create Outline of Hand based on BG Subtracted Image
	CIplImage tempEdge;
	tempEdge.initialize(160, 120, 8);
	cvCanny(this->m_pianoPlaying->m_imb_handsImage.getIplImage(), tempEdge.getIplImage(), 0, 255, 3);
	
	int f, h;
	if (this->m_pianoPlaying->m_nNumHands > 0)
	{
		//****CHANGED JAN 8, 2006****//
		//Combine Hand Outline, and Finger Detection
		cvOr(tempEdge.getIplImage(), this->m_pianoPlaying->m_Hand[0].m_edgeImage.getIplImage(), this->m_pianoPlaying->m_Hand[0].m_edgeImage.getIplImage(), NULL);
		this->m_pianoPlaying->m_Hand[0].m_edgeImage.draw(pDC->m_hDC, pvu::setRECT(3, 1), bStretch, bMirror, bFlip);
		

		h=0;
		if (this->m_pianoPlaying->m_Hand[h].ready == true)
		{
	//		this->m_pianoPlaying.m_Hand[h].m_fingImg[f].draw(pDC->m_hDC, pvu::setRECT(f, h+2), bStretch, bMirror, bFlip);
			for (f=0; f < 5; f++)
			{
			
				this->m_pianoPlaying->m_Hand[h].m_fingImg[f].draw(pDC->m_hDC, pvu::setRECT(f, 2), bStretch, bMirror, bFlip);
				//cvSet( (pPVS->m_imcOut.getIplImage()), CV_RGB(0, 100*h, f*50), 
				//		(this->m_pianoPlaying.m_Hand[h].m_fingImg[f].getIplImage()));
			}
		}
	}


	
	char clearString[150];
	int i;
	for (i=0; i < 150; i++)
		clearString[i] = ' ';
	}
	//////////////////////////END ARJUN////////////////////////////

//
	//pPVS->m_imcOut.draw(pDC->m_hDC, pvu::setRECT(0,0), bStretch, bMirror, bFlip);
	pPVS->m_imcOut.draw(pDC->m_hDC, pvu::setRECT(1,0), bStretch, false, false);
	pPVS->m_imcIn.draw(pDC->m_hDC, pvu::setRECT(2, 1), bStretch, true, true);
}

	
void CGUIFaceInVideoView::OnDraw_LogWindow(CDC* pDC, char *strMessage, RECT *textRect, int nLine)
{
	textRect->top = textRect->top + 18*nLine;
	
	pDC->SetTextColor(RGB(100,0,100));
	pDC->SetBkMode(OPAQUE);
	pDC->DrawText(strMessage, strlen(strMessage), textRect, DT_LEFT);
}

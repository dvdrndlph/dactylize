// PianoPlaying.cpp: implementation of the CPianoPlaying class.
//
//////////////////////////////////////////////////////////////////////

#include "PianoPlaying.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPianoPlaying::CPianoPlaying()
{
	m_nWidth = IMAGE_WIDTH;
	m_nHeight = IMAGE_HEIGHT;
	fDataFile = fopen("data.txt", "w+");

	m_nNumHands = 0;
	m_nTempNumHands = 0;
	m_nTempNumFingers = 0;
}

CPianoPlaying::~CPianoPlaying()
{
	fclose(fDataFile);
}


bool CPianoPlaying::updateImages(CIplImage *pImage)
{	
	m_imc_displayImage.initialize(m_nWidth, m_nHeight, 24);
	m_imc_displayImage.copy(pImage); 
		
	m_imb_dI_colour.initialize(m_nWidth, m_nHeight, 8);
	m_imb_handsImage.initialize(m_nWidth, m_nHeight, 8);
	m_imb_edgeDetectedImage.initialize(m_nWidth, m_nHeight, 8);
	m_imb_fingerDetectedImage.initialize(m_nWidth, m_nHeight, 8);
	m_img_tempImage.initialize(m_nWidth, m_nHeight, 8);

	m_img_bwImage.initialize(m_nWidth, m_nHeight, 8);

	m_imb_handsEdgeImage.initialize(m_nWidth, m_nHeight, 8);

	return true;
}

void CPianoPlaying::setBackgroundImage(CIplImage *pImage)
{
	//Initialize and Set Background Image
	m_imc_backgroundImage.initialize(m_nWidth, m_nHeight, 24);
	m_imc_backgroundImage.copy(pImage);

	m_imc_tempImage.initialize(m_nWidth, m_nHeight, 24);
	m_imc_tempImage2.initialize(m_nWidth, m_nHeight, 24);
		
	//Initialize Piano Keyboard Object with Current Image
	m_Piano = *(new CPianoKeyboard(pImage));

	//Use Determined Piano characteristics to rotate image
	//And select region of interest
	CIplImage tempImage;
	tempImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 24);
	tempImage.copyRotated(&m_imc_backgroundImage, m_Piano.m_dDegreeRotation);
	cvCopy(tempImage.getIplImage(), m_imc_backgroundImage.getIplImage());

	//Black Out Everything Except for Keyboard
	MaskRegion(m_imc_backgroundImage.getIplImage(), 0, m_Piano.m_nKeyboardY1, IMAGE_WIDTH, m_Piano.m_nKeyboardY2);
	
	//Draw Box Around Detected "C"  (actually its an "E" right now)
	DrawBox(m_imc_backgroundImage.getIplImage(), m_Piano.m_nC_X1, m_Piano.m_nKeyboardY1, m_Piano.m_nC_X2, m_Piano.m_nKeyboardY2, 3);

//	cvCvtColor(m_imc_backgroundImage.getIplImage(), m_imc_tempImage.getIplImage(), CV_BGR2YCrCb);
//	cvCopy(m_imc_tempImage.getIplImage(), m_imc_backgroundImage.getIplImage(), NULL);
}

void CPianoPlaying::createHandsImage(int threshold, bool colourAverage, 
									int colourThreshold, bool edgeDetect, 
									int edgeDetectThreshold, bool lineDetect,
									bool boundingBox, int boundingBoxThreshold)
{
	int avgX, avgY;
	unsigned char avgR, avgG, avgB;	

	int m_nDiffThres, m_nDiffMode, m_nDiffNonlinearBox;
	m_nDiffThres = threshold;

	//Detect Changes Based on Colour
	detectChange_Colour(m_imc_displayImage.getIplImage(), m_imc_backgroundImage.getIplImage(),
			m_imb_handsImage.getIplImage(), m_imb_dI_colour.getIplImage(),
			m_nDiffThres);

	ClearImage(m_imb_edgeDetectedImage.getIplImage());

	m_img_bwImage.copyBW(&m_imc_displayImage);

//	cvCvtColor(m_imc_displayImage.getIplImage(), m_imc_tempImage.getIplImage(), CV_BGR2YCrCb);
//	cvCopy(m_imc_tempImage.getIplImage(), m_imc_displayImage.getIplImage(), NULL);

	//Colour Averaging and Compensation
	if (colourAverage == true)
	{
		//Boost Skin Colour Recognition

		int totalPixels = 0;

		//Calculate Average Colour of Difference (hopefully Skin Colour)
		CalculateAverageColour(m_imc_displayImage.getIplImage(), m_imb_dI_colour.getIplImage(), 
								&avgR, &avgG, &avgB, &avgX, &avgY);
	
		/*CalculateAverageColour(m_imc_tempImage.getIplImage(), m_imb_dI_colour.getIplImage(), 
								&avgR, &avgG, &avgB, &avgX, &avgY);*/
		//Locate Skin Colour in Image
		//Outputs results in dI_colour
		AddAverageColour(m_imc_displayImage.getIplImage(), m_imb_handsImage.getIplImage(), 
							avgR, avgG, avgB, colourThreshold);

		//Locate Skin Colour in Background Image
		//Outputs results in dI
		AddAverageColour(m_imc_backgroundImage.getIplImage(), m_img_tempImage.getIplImage(),
							avgR, avgG, avgB, colourThreshold);
	
		//Remove Common Elements from Colour Difference Map
		cvSub(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), m_imb_handsImage.getIplImage());
		
		cvSmooth(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), CV_MEDIAN, 3, 3, 0);
		cvCopy(m_img_tempImage.getIplImage(), m_imb_handsImage.getIplImage(), NULL);

	}

	if (edgeDetect == true)
	{
		//DetectEdges
		//DetectEdges(m_imb_handsImage.getIplImage(), m_imb_edgeDetectedImage.getIplImage(), edgeDetectThreshold);		
		//cvCanny(m_imb_handsImage.getIplImage(), m_imb_edgeDetectedImage.getIplImage(), 0, 1, 3);
		
		LocateHands(&m_imb_handsImage, m_tempHand, &m_nTempNumHands);

		int x;
		int i, j;

		for (x=0; x < m_nTempNumHands; x++)
		{
			for (i=m_tempHand[x].m_boundingBox.x - 10; i < (m_tempHand[x].m_boundingBox.x + m_tempHand[x].m_boundingBox.width + 10); i++)
			{
				for (j=m_tempHand[x].m_boundingBox.y - 10; j < (m_tempHand[x].m_boundingBox.y + m_tempHand[x].m_boundingBox.height + 10); j++)
					m_imb_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] = 255;
			}
		}
		
		for (i=0; i < IMAGE_WIDTH; i++)
		{
			for (j=0; j < IMAGE_HEIGHT; j++)
			{
				if ((unsigned char)m_imb_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] == 0)
					m_img_bwImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] = 0;
			}
		}
		
				cvCanny(m_img_bwImage.getIplImage(), m_imb_edgeDetectedImage.getIplImage(), 0, 255, 3);		
	}

	if (lineDetect == true)
	{
		//Edge of Hand
		cvErode(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage());
		cvCanny(m_img_tempImage.getIplImage(), m_imb_handsEdgeImage.getIplImage(), 0, 1, 3);

		//Use TempImage so that traced pixels can be removed
		LocateIntensityIncreases(&m_imc_displayImage, &m_imb_edgeDetectedImage, &m_imb_handsImage);
		RemoveSinglePixels(m_imb_edgeDetectedImage.getIplImage());
		
	//	cvSmooth(m_imb_edgeDetectedImage.getIplImage(), m_img_tempImage.getIplImage(), CV_BLUR, 3, 3, 0);
	//	cvCopy(m_img_tempImage.getIplImage(), m_imb_edgeDetectedImage.getIplImage(), NULL);

	/*	int i, j;

		for (i=0; i < IMAGE_WIDTH; i++)
		{
			for (j=0; j < IMAGE_HEIGHT; j++)
			{
				if ((unsigned char)m_imb_edgeDetectedImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] > 80)
					m_imb_edgeDetectedImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] = 255;
			}
		}

	//	FillVerticalGaps(&m_imb_edgeDetectedImage,5);
	/*	cvSmooth(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), CV_MEDIAN, 3, 3, 0);
		cvCopy(m_img_tempImage.getIplImage(), m_imb_handsImage.getIplImage(), NULL);
		//ClearHorizontalGaps(&m_imb_edgeDetectedImage, 3);
	/*	cvCanny(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), 0, 1, 3);					

		int i, j;
		for (i=0; i < IMAGE_WIDTH; i++)
		{
			for (j=0; j < IMAGE_HEIGHT; j++)
			{
				if ((unsigned char)m_img_tempImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
					m_imb_edgeDetectedImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] = 255;
			}
		}
*/
		//Backup Intensity Change Image
//		cvCopy(m_imb_edgeDetectedImage.getIplImage(), m_img_tempImage.getIplImage(), NULL);

		ClearImage(m_img_tempImage.getIplImage());

		//Locate Finger tips by tracing lines
		CvPoint tempFingers[20];
		int tempNum;
		TraceFingerLines(m_imb_edgeDetectedImage.getIplImage(), tempFingers, &tempNum);

		//Restore Intensity Change Image to EdgeDetectedImage
		cvCopy(m_img_tempImage.getIplImage(), m_imb_edgeDetectedImage.getIplImage(), NULL);

		FillSinglePixels(m_imb_handsImage.getIplImage());

		cvErode(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), NULL, 1);
		//cvDilate(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage());
		cvCopy(m_img_tempImage.getIplImage(), m_imb_handsImage.getIplImage(), NULL);

		//cvCanny(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), 0, 1, 3);

		int i, j;

		for (i=0; i < IMAGE_WIDTH; i++)
		{
			for (j=0; j < IMAGE_HEIGHT; j++)
			{
			//	i = 60;
			//	j = 50;
				
				int fingX1, fingY1, fingX2, fingY2, tipX, tipXhigh;
				if ((unsigned char)m_imb_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
				{
					//Find Finger Bounds
					FindFingerTipsFromBlobs2(&m_imb_handsImage, &m_img_tempImage, i, j, 100,
								&fingX1, &fingY1, &fingX2, &fingY2, &tipX, &tipXhigh);
		
					int r, s;
					int sum = 0;
					int sum2 = 0;
					
					for (r=-2; r < 2; r++)
					{
						for (s=-2; s < 2; s++)
							sum += (unsigned char)m_imb_handsEdgeImage.getIplImage()->imageData[((fingY2+r)*IMAGE_WIDTH)+tipX+s];
					}

					for (r=-2; r < 2; r++)
					{
						for (s=-2; s < 2; s++)
							sum2 += (unsigned char)m_imb_handsEdgeImage.getIplImage()->imageData[((fingY1+r)*IMAGE_WIDTH)+tipXhigh+s];
					}

					//Make sure it is on the outline of the hand
					if (sum > 0 && sum2 > 0)
					{
						//DrawBox(m_imc_displayImage.getIplImage(), tipX-2, fingY1-2, tipXhigh+2, fingY2+2, 3);
						//DrawBox(m_imb_handsEdgeImage.getIplImage(), tipX-2, fingY1-2, tipXhigh+2, fingY2+2, 1);
						if ((abs(fingY1-fingY2) * abs(fingX1-fingX2)) > 4)
							DrawBox(m_imc_displayImage.getIplImage(), tipX-2, fingY2-2, tipX+2, fingY2+2, 3);
					}

//					computeBlob(&m_imb_handsImage, &m_imb_handsImage, i, j, 100, &fingX1, &fingY1, &fingX2, &fingY2);

				//	if (abs(fingX1-fingX2) > 2 && abs(fingY1-fingY2) > 5)
						//DrawBox(m_imc_displayImage.getIplImage(), tipX-2, fingY2-2, tipX+2, fingY2+2, 3);
				}
			}
		}
	
		cvCopy(m_imb_handsEdgeImage.getIplImage(), m_imb_edgeDetectedImage.getIplImage(), NULL);

		/*int i, j;

		for (i=0; i < IMAGE_WIDTH; i++)
		{
			for (j=0; j < IMAGE_HEIGHT; j++)
			{
				if ((unsigned char)m_img_tempImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
				{
//					m_imb_edgeDetectedImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] = 255;

//					m_imc_displayImage.getIplImage()->imageData[j*3*IMAGE_WIDTH+i*3+0]=255;
//					m_imc_displayImage.getIplImage()->imageData[j*3*IMAGE_WIDTH+i*3+1]=255;
//					m_imc_displayImage.getIplImage()->imageData[j*3*IMAGE_WIDTH+i*3+2]=255;
				}
			}
		}*/
/*		m_nTempNumHands = 0;
		m_nTempNumFingers = 0;

		cvSmooth(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), CV_MEDIAN, 3, 3, 0);
		cvCopy(m_img_tempImage.getIplImage(), m_imb_handsImage.getIplImage(), NULL);

		LocateHands(&m_img_tempImage, m_tempHand, &m_nTempNumHands);

		//Outline of Hand
		cvCanny(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), 0, 1, 3);			


		int i;
		int q;
		for (q=0; q < m_nTempNumHands; q++)
		{
			for (i=0; i < tempNum; i++)
			{
				if ((tempFingers[i].x > m_tempHand[q].m_boundingBox.x) && (tempFingers[i].x < (m_tempHand[q].m_boundingBox.x + m_tempHand[q].m_boundingBox.width)))
				{
					if ((tempFingers[i].y > m_tempHand[q].m_boundingBox.y) && (tempFingers[i].y < (m_tempHand[q].m_boundingBox.y + m_tempHand[q].m_boundingBox.height)))
					{
						//Check if it lies within 4x4 region of some pixel on the outline
						int r, s;
						int sum = 0;
						for (r=-1; r < 2; r++)
						{
							for (s=-1; s < 2; s++)
								sum = (unsigned char)m_img_tempImage.getIplImage()->imageData[((tempFingers[i].y+r)*IMAGE_WIDTH)+tempFingers[i].x+s];
						}
						//sum=1;
						//ADD Finger to Appropriate Temp Hand
						if (sum > 0)
						//if ((tempFingers[i].y - m_tempHand[q].m_boundingBox.y) < (m_tempHand[q].m_boundingBox.height / 3))
						{
							m_tempHand[q].AddFinger(tempFingers[i].x, tempFingers[i].y);
							m_tempFingers[m_nTempNumFingers] = *(new CPianoFinger(tempFingers[i].x, tempFingers[i].y));
							m_nTempNumFingers++;
						}
					}
				}
			}
		}


		/*
		FillVerticalGaps(&m_imb_edgeDetectedImage, 5);
		ClearHorizontalGaps(&m_imb_edgeDetectedImage, 3);

		//Get Outline of Hand
		ClearHorizontalGaps(&m_imb_handsImage, 3);
		cvSmooth(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), CV_MEDIAN, 3, 3, 0);
		cvCopy(m_img_tempImage.getIplImage(), m_imb_handsImage.getIplImage(), NULL);
				
		//Locate Hands
		m_nTempNumHands = 0;
		m_nTempNumFingers = 0;
		LocateHands(&m_img_tempImage, m_tempHand, &m_nTempNumHands);

		//Outline of Hand
		cvCanny(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), 0, 1, 3);			
*/
		//Get All Finger Tips using Compute Blob to get the bottom of fingers
/*		int i, j;
		int fingX1, fingY1, fingX2, fingY2, tipX;
		float boxRatio;

		//Loop Through Each Pixel
		for (i=1; i < IMAGE_WIDTH; i++)
		{
			for (j=1; j < IMAGE_HEIGHT; j++)
			{
				//Only Check Lit Pixels on Intensity Change Image
				if ((unsigned char)m_imb_edgeDetectedImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
				{
					//Find Finger Bounds
					FindFingerTipsFromBlobs(&m_imb_edgeDetectedImage, &m_img_tempImage, i, j, 254,
								&fingX1, &fingY1, &fingX2, &fingY2, &tipX);

					//DrawBox(m_imc_displayImage.getIplImage(), tipX-2, fingY2-2, tipX+2, fingY2+2, 3);

					//Calculate Box Ratio
					boxRatio = abs(float(fingX2-fingX1)/float(fingY2-fingY1));

					//Mark Finger Tips
					//IF within region of any lit pixel on edge map
					int r, s;
					int sum = 0;
					for (r=-2; r < 2; r++)
					{
						for (s=-2; s < 2; s++)
							sum += (unsigned char)m_img_tempImage.getIplImage()->imageData[((fingY2+r)*IMAGE_WIDTH)+tipX+s];
					}
					
					//Add Fingers to Appropriate Temp Hand
					int q;
					for (q=0; q < m_nTempNumHands; q++)
					{
						boxRatio=0; //test
						if (boxRatio < 1)
						{	
							//IF Finger is within bounds of Temp Hand
							if ((tipX > m_tempHand[q].m_boundingBox.x) && (tipX < (m_tempHand[q].m_boundingBox.x + m_tempHand[q].m_boundingBox.width)))
							{
								if ((fingY2 > m_tempHand[q].m_boundingBox.y) && (fingY2 < (m_tempHand[q].m_boundingBox.y + m_tempHand[q].m_boundingBox.height)))
								{
									//ADD Finger to Appropriate Temp Hand
									m_tempHand[q].AddFinger(tipX, fingY2);
									m_tempFingers[m_nTempNumFingers] = *(new CPianoFinger(tipX, fingY2));
									m_nTempNumFingers++;
								}
								
							}
						}
					}
				}
			}
		}
		/*
		//Show Temp Image (Outline) through EdgeDetectedImage
		cvCopy(m_img_tempImage.getIplImage(), m_imb_edgeDetectedImage.getIplImage(), NULL);*/
	}
	if (boundingBox == true)
	{
		int z, y;

		//IF No Hands have been Created, Create them from Temporary Hands
		if (m_nNumHands == 0)
		{
			m_Hand[0] = m_tempHand[0];
			m_Hand[1] = m_tempHand[1];
			m_nNumHands++;
			m_nNumHands++;
		}
		//ELSE, already been created
		else
		{
			//Write Hand Detect Information
/*			fprintf(fDataFile, "TEMP HAND 1: %d, %d, %d, %d\n", m_tempHand[0].m_center.x, 
			m_tempHand[0].m_center.y, m_tempHand[0].m_boundingBox.width, m_tempHand[0].m_boundingBox.height);
			fprintf(fDataFile, "TEMP HAND 2: %d, %d, %d, %d\n", m_tempHand[1].m_center.x, 
			m_tempHand[1].m_center.y, m_tempHand[1].m_boundingBox.width, m_tempHand[1].m_boundingBox.height);
			fprintf(fDataFile, "REAL HAND 1: %d, %d, %d, %d\n", m_Hand[0].m_center.x, 
			m_Hand[0].m_center.y, m_Hand[0].m_boundingBox.width, m_Hand[0].m_boundingBox.height);
			fprintf(fDataFile, "REAL HAND 2: %d, %d, %d, %d\n", m_Hand[1].m_center.x, 
			m_Hand[1].m_center.y, m_Hand[1].m_boundingBox.width, m_Hand[1].m_boundingBox.height);
*/
			//FOR all Temporary Hands
			for (z = 0; z < m_nTempNumHands; z++)
			{
				float highMatch = 0;
				float currMatch;
				int matchNum;
				
				//Check against each Existing Hand
				//And Get Highest Probability Match
				for (y=0; y < m_nNumHands; y++)
				{			//		currMatch[z][y] = m_tempHand[z].Probability(&(m_Hand[y]));
					currMatch = m_tempHand[z].Probability(&(m_Hand[y]));
					fprintf(fDataFile, "PROBABILITY: %f\n", currMatch);
				
					if (highMatch < currMatch)
					{
						highMatch = currMatch;
						matchNum = y;
					}
				}
		
				//Update Matched Hand with Temporary Hand
				m_Hand[matchNum].UpdateWithHand(&(m_tempHand[z]));
				fprintf(fDataFile, "Match Number: %d %d - %d\n", z, matchNum, m_nTempNumHands);

				//Identify Hand by Drawing Box Around it and marking the Center
				DrawLine(m_imc_displayImage.getIplImage(), m_Hand[matchNum].m_center.x - 2, m_Hand[matchNum].m_center.y, m_Hand[matchNum].m_center.x + 2, m_Hand[matchNum].m_center.y, 3);
				DrawLine(m_imc_displayImage.getIplImage(), m_Hand[matchNum].m_center.x, m_Hand[matchNum].m_center.y - 2, m_Hand[matchNum].m_center.x, m_Hand[matchNum].m_center.y + 2, 3);
				DrawBox(m_imc_displayImage.getIplImage(), m_Hand[matchNum].m_boundingBox.x, m_Hand[matchNum].m_boundingBox.y, m_Hand[matchNum].m_boundingBox.x + m_Hand[matchNum].m_boundingBox.width, m_Hand[matchNum].m_boundingBox.y + m_Hand[matchNum].m_boundingBox.height, 3);

				fprintf(fDataFile, "Section Hand %d : ", matchNum);
				//Mark Each Finger of Each Hand, on the Screen
				for (y=0; y < m_Hand[matchNum].m_nNumFingers; y++)
				{
					//fprintf(fDataFile, "%f, ", m_Hand[matchNum].m_fingers[y].certainty);
					fprintf(fDataFile, "%f, ", m_Hand[matchNum].m_fingers[y].sectionX);
					DrawBox(m_imc_displayImage.getIplImage(), m_Hand[matchNum].m_fingers[y].x-2, m_Hand[matchNum].m_fingers[y].y-2, m_Hand[matchNum].m_fingers[y].x+2, m_Hand[matchNum].m_fingers[y].y+2, 3);
					DrawBox(m_imb_edgeDetectedImage.getIplImage(), m_Hand[matchNum].m_fingers[y].x-2, m_Hand[matchNum].m_fingers[y].y-2, m_Hand[matchNum].m_fingers[y].x+2, m_Hand[matchNum].m_fingers[y].y+2, 1);
				}
				fprintf(fDataFile, "\n");
/*
				for (y=0; y < m_tempHand[z].m_nNumFingers; y++)
				{
					DrawBox(m_imc_displayImage.getIplImage(), m_tempHand[z].m_fingers[y].x-2, m_tempHand[z].m_fingers[y].y-2, m_tempHand[z].m_fingers[y].x+2, m_tempHand[z].m_fingers[y].y+2, 3);
					DrawBox(m_imb_edgeDetectedImage.getIplImage(), m_tempHand[z].m_fingers[y].x-2, m_tempHand[z].m_fingers[y].y-2, m_tempHand[z].m_fingers[y].x+2, m_tempHand[z].m_fingers[y].y+2, 1);
				}
*/
			}
		}
		
		/*	
		for (z = 0; z < m_nTempNumHands; z++)
		{
			DrawLine(m_imc_displayImage.getIplImage(), m_tempHand[z].m_center.x - 2, m_tempHand[z].m_center.y, m_tempHand[z].m_center.x + 2, m_tempHand[z].m_center.y, 3);
			DrawLine(m_imc_displayImage.getIplImage(), m_tempHand[z].m_center.x, m_tempHand[z].m_center.y - 2, m_tempHand[z].m_center.x, m_tempHand[z].m_center.y + 2, 3);
			DrawBox(m_imc_displayImage.getIplImage(), m_tempHand[z].m_boundingBox.x, m_tempHand[z].m_boundingBox.y, m_tempHand[z].m_boundingBox.x + m_tempHand[z].m_boundingBox.width, m_tempHand[z].m_boundingBox.y + m_tempHand[z].m_boundingBox.height, 3);

			for (y=0; y < m_tempHand[z].m_nNumFingers; y++)
			{
				fprintf(fDataFile, "Fings: %d: %d, %d\n", m_tempHand[z].m_nNumFingers, m_tempHand[z].m_fingers[y].x, m_tempHand[z].m_fingers[y].y);
				DrawBox(m_imc_displayImage.getIplImage(), m_tempHand[z].m_fingers[y].x-2, m_tempHand[z].m_fingers[y].y-2, m_tempHand[z].m_fingers[y].x+2, m_tempHand[z].m_fingers[y].y+2, 3);
			}

		}
*/
		//for (z=0; z < m_nTempNumFingers; z++)
		//	DrawBox(m_imc_displayImage.getIplImage(), m_tempFingers[z].x-2, m_tempFingers[z].y-2, m_tempFingers[z].x+2, m_tempFingers[z].y+2, 3);
  }

}

//Detects Changes using Colour Differences
void CPianoPlaying::detectChange_Colour(IplImage *pImg, IplImage *pImg1, 
								IplImage *pImb_dIon, IplImage *pImb_dI,
								int nThresh)
{

// 2005 May:	detectChange(pImg, pImg1, pImb_dIon, pImb_dI, 20)

	unsigned char r, g, b, r1, g1, b1, rDiff, gDiff, bDiff, componentDiff;
	int x, y;
	int height, width;
	bool shadow;
	
	height = pImg->height;
	width = pImg->width;

	//Compares Colour Differences against a Threshold
	//and Records them to dI
	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			r = (unsigned char)pImg->imageData[(y*3*width)+(x*3+2)];
			g = (unsigned char)pImg->imageData[(y*3*width)+(x*3+1)];
			b =	(unsigned char)pImg->imageData[(y*3*width)+(x*3)];

			r1 = (unsigned char)pImg1->imageData[(y*3*width)+(x*3+2)];
			g1 = (unsigned char)pImg1->imageData[(y*3*width)+(x*3+1)];
			b1 = (unsigned char)pImg1->imageData[(y*3*width)+(x*3)];

			rDiff = abs(r - r1);
			gDiff = abs(g - g1);
			bDiff = abs(b - b1);

			//For non-shadow recognition
			componentDiff = abs(rDiff - gDiff) + abs(rDiff - bDiff) + abs(bDiff - gDiff);
			shadow = false;//isShadow(r, g, b, r1, g1, b1, 1);

			//if ((rDiff>nThresh) && (gDiff>nThresh) && (bDiff>nThresh))
			if ((componentDiff > nThresh) && (shadow == false))
					pImb_dI->imageData[(y*width)+(x)] = 255;
			else if ((rDiff>nThresh) && (gDiff>nThresh) && (bDiff>nThresh) && (shadow == false))
					pImb_dI->imageData[(y*width)+(x)] = 255;
			else		
					pImb_dI->imageData[(y*width)+(x)] = 0;
		}
	}

	//Blurs to Remove Noise
	cvSmooth( pImb_dI, pImb_dIon, CV_MEDIAN, 3, 3, 0);
	cvCopy(pImb_dIon, pImb_dI, NULL);
//	cvSmooth( pImb_dI, pImb_dIon, CV_BLUR, 3, 3, 0);
//	cvSmooth( pImb_dIon, pImb_dI, CV_GAUSSIAN, 3, 3, 0);
}

void CPianoPlaying::FindBoundingBox(IplImage *pImg, int origX, int origY,
										int *x1, int *y1, int *x2, int *y2, int threshold)
{
	int height = IMAGE_HEIGHT;
	int width = IMAGE_WIDTH;

	int x, y;
	int numPixels;
	const MAXBREAK = 10;
	int bufferDist = MAXBREAK;


	for (x=origX; x < width; x++)
	{
		numPixels = 0;
		for (y=0; y < height; y++)
		{
			if (pImg->imageData[(y*width) + x] != 0)
				numPixels++;
		}
		
		//Get New X of Bounding Box OR exit loop
		if (numPixels > threshold)
			*x2 = x;
		else 
			bufferDist--;

		if (bufferDist < 0)
			x = width;
	
	}

	bufferDist=MAXBREAK;
	for (x=origX; x > 0; x--)
	{
		numPixels = 0;
		for (y=0; y < height; y++)
		{
			if (pImg->imageData[(y*width) + x] != 0)
				numPixels++;
		}
		
		if (numPixels > threshold)
			*x1 = x;
		else 
			bufferDist--;

		if (bufferDist < 0)
			x = 0;
	}

	bufferDist=MAXBREAK;
	for (y=origY; y < height; y++)
	{
		numPixels = 0;
		for (x=0; x < width; x++)
		{
			if (pImg->imageData[(y*width) + x] != 0)
				numPixels++;
		}
		
		if (numPixels > threshold)
			*y2 = y;
		else 
			bufferDist--;

		if (bufferDist < 0)
			y = width;
	}

	bufferDist=MAXBREAK;
	for (y=origY; y > 0; y--)
	{
		numPixels = 0;
		for (x=0; x < width; x++)
		{
			if (pImg->imageData[(y*width) + x] != 0)
				numPixels++;
		}
		
		if (numPixels > threshold)
			*y1 = y;
		else 
			bufferDist--;

		if (bufferDist < 0)
			y = 0;
	}

}

void CPianoPlaying::CalculateAverageColour(IplImage *pImg, IplImage *pImg1, 
											unsigned char *avgR, unsigned char *avgG, 
											unsigned char *avgB, int *avgX, 
											int *avgY)
{
	unsigned char r, g, b;
	float totalR = 0, totalG = 0, totalB = 0;
	float totalPixels = 0;
	int x, y;
	int height, width;

	float totalX = 0, totalY = 0;
	
	height = pImg->height;
	width = pImg->width;
	
	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			if (unsigned char(pImg1->imageData[(y*width)+x]) == 255)
			{
				r = (unsigned char)pImg->imageData[(y*3*width)+(x*3+2)];
				g = (unsigned char)pImg->imageData[(y*3*width)+(x*3+1)];
				b = (unsigned char)pImg->imageData[(y*3*width)+(x*3)];
				totalPixels++;

				totalR += r;
				totalG += g;
				totalB += b;

				totalX += x;
				totalY += y;
			}
		}
	}
	
	*avgR = (unsigned char)((totalR / totalPixels));
	*avgG = (unsigned char)((totalG / totalPixels));
	*avgB = (unsigned char)((totalB / totalPixels));

	*avgX = (int)(totalX/totalPixels);
	*avgY = (int)(totalY/totalPixels);
}

void CPianoPlaying::AddAverageColour(IplImage *pImg, IplImage *pImg1, 
											unsigned char avgR, unsigned char avgG, 
											unsigned char avgB, int threshold)
{
	unsigned char r, g, b;
	unsigned char rDiff, gDiff, bDiff;
	unsigned char componentDiff;
	int x, y;
	int height, width;
	
	height = pImg->height;
	width = pImg->width;

	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			r = pImg->imageData[(y*3*width)+(x*3)+2];
			g = pImg->imageData[(y*3*width)+(x*3)+1];
			b = pImg->imageData[(y*3*width)+(x*3)];
			
			rDiff = abs(r - avgR);
			gDiff = abs(g - avgG);
			bDiff = abs(b - avgB);

			//For Shade Detection
			componentDiff = abs(rDiff - gDiff) + abs(rDiff - bDiff) + abs(gDiff - bDiff);

			if ((unsigned char)m_imb_dI_colour.getIplImage()->imageData[(y*width)+x] == 255)
			{
				if ((rDiff < threshold) && (gDiff < threshold) && (bDiff < threshold))
						pImg1->imageData[(y*width)+x] = 255;
				else
					pImg1->imageData[(y*width)+x] = 0;
			}
		}
	}
}

bool CPianoPlaying::isShadow(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2, float threshold)
{
	float floatR1, floatG1, floatB1;
	float floatR2, floatG2, floatB2;

	floatR1 = r1;
	floatG1 = g1;
	floatB1 = b1;
	
	floatR2 = r2;
	floatG2 = g2;
	floatB2 = b2;

	float ratioR, ratioG, ratioB;

	ratioR = floatR1/floatR2;
	ratioG = floatG1/floatG2;
	ratioB = floatB1/floatB2;

	if ((abs(ratioR - ratioG) < threshold) && (abs(ratioR - ratioB) < threshold))
		return true;
	else
		return false;
}

void CPianoPlaying::DetectEdges(IplImage *pImg, IplImage *pImg1, int threshold)
{
	int x, y;
	int height, width;
	int intensity; 
	int i, j;
	
	height = pImg->height;
	width = pImg->width;

	for (y=1; y<(height-1); y++)
	{
		for (x=1; x<(width-1); x++)
		{
			intensity = 0;
			for (i=0; i < 3; i++)
			{
				for (j=0; j < 3; j++)
				{
					if ((i==2) && (j==2))
						intensity += (8 * pImg->imageData[((y-1+j)*width)+x-1+i]);
					else
						intensity -= pImg->imageData[((y-1+j)*width)+x-1+i];
				}
			}
						
			if (intensity >= threshold)
				pImg1->imageData[(y*width)+x] = 255;
			else
				pImg1->imageData[(y*width)+x] = 0;
		}
	}
}

void CPianoPlaying::DetectHorizontalEdges(IplImage *pImg, IplImage *pImg1, int threshold)
{
	int x, y;
	int height, width;
	int intensity; 
	int i, j;
	
	height = pImg->height;
	width = pImg->width;

	for (y=1; y<(height-1); y++)
	{
		for (x=1; x<(width-1); x++)
		{
			intensity = 0;
			for (i=0; i < 3; i++)
			{
				for (j=0; j < 3; j++)
				{
					if (j==0)
						intensity += pImg->imageData[((y-1+j)*width)+x-1+i];
					else if (j==2)
						intensity -= pImg->imageData[((y-1+j)*width)+x-1+i];
				}
			}
						
			if (intensity >= threshold)
				pImg1->imageData[(y*width)+x] = 255;
			else
				pImg1->imageData[(y*width)+x] = 0;
		}
	}
}

void CPianoPlaying::DetectVerticalEdges(IplImage *pImg, IplImage *pImg1, int threshold)
{
	int x, y;
	int height, width;
	int intensity; 
	int i, j;
	
	height = pImg->height;
	width = pImg->width;

	for (y=1; y<(height-1); y++)
	{
		for (x=1; x<(width-1); x++)
		{
			intensity = 0;
			for (i=0; i < 3; i++)
			{
				for (j=0; j < 3; j++)
				{
					if (i==0)
						intensity += pImg->imageData[((y-1+j)*width)+x-1+i];
					else if (i==2)
						intensity -= pImg->imageData[((y-1+j)*width)+x-1+i];
				}
			}
						
			if (intensity >= threshold)
				pImg1->imageData[(y*width)+x] = 255;
			else
				pImg1->imageData[(y*width)+x] = 0;
		}
	}
}


void CPianoPlaying::createFingerLines(IplImage *pImg, int *r, float *theta, 
									float *density, int totalLines, int threshold, int boundX1,
									int boundY1, int boundX2, int boundY2)
{
	 int x;
	 int height, width;

	 int curDensity;
	 float curTheta;

	 height = pImg->height;
	 width = pImg->width;

	 int data[IMAGE_WIDTH][200];
	 int loopCounter;

	 int y2 = 0;
	 int y = IMAGE_HEIGHT-1;

  	 for (x=0; x < IMAGE_WIDTH; x++)
	 {
		 loopCounter = 0;
		 for (curTheta=0; curTheta < PI; curTheta += (PI/200))
		 {
			data[x][loopCounter] = 0;
			int x2 = x + (y/tan(curTheta));

			int dx = abs(x - x2);
			int dy = abs(y - y2);
			double s = double(0.99/(dx>dy?dx:dy));
			double t = 0.0;
     
			curDensity = 0;
			
			while(t < 1.0)
			{
	 			dx = int((1.0 - t)*x + t*x2);
				dy = int((1.0 - t)*y + t*y2);
	 			 
				if (dx > 0 && dx < IMAGE_WIDTH)
				{
					if (pImg->imageData[(dy*IMAGE_WIDTH) + dx] != 0)
						curDensity++;
				}
				else
					t = 1.0; //Exit Loop
				 
				t += s; 
			}
			data[x][loopCounter] = curDensity;
			loopCounter++;
		 }
	 }
	 int highX, highCounter, highDensity;
	 int i;
	 for (i=0; i < totalLines; i++)
	 {
		highX = 0, highCounter = 0, highDensity = 0;
		for (x=0; x < width; x++)
		{
			loopCounter=0;
			for (curTheta=0; curTheta < PI; curTheta += (PI/200))
			{
				loopCounter++;
				if (data[x][loopCounter] > highDensity)
				{
					highX = x;
					highCounter = loopCounter;
					highDensity = data[x][loopCounter];
				}
			}
		}
		
		//Make so data is not counted again in the sort.
		data[highX][highCounter] = 0;

		//Update Highest Matches
		r[i] = highX;
		theta[i] = (PI/200) * highCounter;
		density[i] = highDensity;
	 }

}

void CPianoPlaying::LocateFingers(IplImage *pImg, int boundX1,
									int boundY1, int boundX2, int boundY2)
{
	int i, j;
	int x, y;
	int newX;
	int val;

	const MAXLENGTH = 1;
	int totalCount = 0;

	int topX2[5];
	int topY2[5];
	int topX[5];
	int topY[5];
	int topVal[5];

	totalCount = 0;
	int q;
	sprintf(keyString, "");

	for (j=IMAGE_HEIGHT-1; j >= 0; j--)
	{
		for (i=0; i < IMAGE_WIDTH; i++)
		{	
			newX = 0;
			y=0;
			val = 0;
	
			if (unsigned char(pImg->imageData[(j*IMAGE_WIDTH)+i]) > 0)
				val = LineSizeRecursive(pImg, i, j, &newX, &y);
			else
				val = 0;


			if (val > 8)
			{
				DrawBox(m_imc_displayImage.getIplImage(), i-3+1, j-3, i+3+1, j+3, 3);
				DrawBox(m_imc_displayImage.getIplImage(), i-3, j-3, i+3, j+3, 3);
				DrawBox(m_imc_displayImage.getIplImage(), i-3, j-3+1, i+3, j+3+1, 3);
				sprintf(keyString, "%s %c - %d, %d|", keyString, m_Piano.SelectKey(i, j), i, j);
			}
		}
	}
}

int CPianoPlaying::LineSizeRecursive(IplImage *pImg, int i, int j, int *x, int *y,
									  int boundX1, int boundY1, int boundX2, int boundY2)
{
	int curTotal, curTotal2, curTotal3;
	int x1=0, x2=0, x3=0;
	int y1=0, y2=0, y3=0;
	//int x1 = 0, y1 = 0;
	int totalLines = 0;
	unsigned char val = 0;
	unsigned tempVal = 0;

	if (j >= 0 && j < IMAGE_HEIGHT && i >= 0 && i < IMAGE_WIDTH)
		val = pImg->imageData[(j*IMAGE_WIDTH)+i];
	else
		val = 0;

	if (val > 0)
	{
			x1 = *x;
			y1 = *y;
			x2 = *x;
			y2 = *y;
			x3 = *x;
			y3 = *y;

			curTotal = LineSizeRecursive(pImg, i-1, j-1, &x1, &y1);

			curTotal2 = LineSizeRecursive(pImg, i, j-1, &x2, &y2);

			curTotal3 = LineSizeRecursive(pImg, i+1, j-1, &x3, &y3);

		if ((curTotal >= curTotal2) && (curTotal >= curTotal3))
		{
			totalLines = curTotal;
			*x = ((x1) - 1);
			*y = ((y1) - 1);
		}
		else if ((curTotal2  >= curTotal) && (curTotal2 >= curTotal3))
		{
			totalLines = curTotal2;
			*x = x2;
			*y = ((y2) - 1);
		}
		else
		{
			totalLines = curTotal3;
			*x = ((x3) + 1);
			*y = ((y3) - 1);
		}
	}
	else
		return 0;
	
	pImg->imageData[(j*IMAGE_WIDTH)+i] = 0;
	return totalLines + 1;
}

void CPianoPlaying::DrawBox(IplImage *pImg, int x1, int y1, int x2, int y2, int bpp)
{
	DrawLine(pImg, x1, y1, x2, y1, bpp);
	DrawLine(pImg, x2, y1, x2, y2, bpp);
	DrawLine(pImg, x2, y2, x1, y2, bpp);
	DrawLine(pImg, x1, y2, x1, y1, bpp);
}

void CPianoPlaying::DrawLine(IplImage *pImg, int x, int y, int x2, int y2, int bpp)
{
	int dx = abs(x - x2);
    int dy = abs(y - y2);
    double s = double(0.99/(dx>dy?dx:dy));
    double t = 0.0;
    while(t < 1.0)
    {
        dx = int((1.0 - t)*x + t*x2);
        dy = int((1.0 - t)*y + t*y2);
        //putPix(dx, dy, col);
		if ((dx >= 0) && (dx < IMAGE_WIDTH) && (dy >= 0) && (dy < IMAGE_HEIGHT))
		{
			int q;
			for (q=0; q<bpp; q++)
				pImg->imageData[(dy*IMAGE_WIDTH*bpp) + (dx*bpp) + q] = 255;
		}
		else
			t = 1.0; //END LOOP
        t += s; 
    }
}

void CPianoPlaying::DrawBlackLine(IplImage *pImg, int x, int y, int x2, int y2, int bpp)
{
	int dx = abs(x - x2);
    int dy = abs(y - y2);
    double s = double(0.99/(dx>dy?dx:dy));
    double t = 0.0;
    while(t < 1.0)
    {
        dx = int((1.0 - t)*x + t*x2);
        dy = int((1.0 - t)*y + t*y2);
        //putPix(dx, dy, col);
		if ((dx >= 0) && (dx < IMAGE_WIDTH) && (dy >= 0) && (dy < IMAGE_HEIGHT))
		{
			int q;
			for (q=0; q<bpp; q++)
				pImg->imageData[(dy*IMAGE_WIDTH*bpp) + (dx*bpp) + q] = 0;
		}
		else
			t = 1.0; //END LOOP
        t += s; 
    }
}

void CPianoPlaying::ClearImage(IplImage *pImg)
{
	int x, y;

	for (x=0; x < IMAGE_WIDTH; x++)
	{
		for (y=0; y < IMAGE_HEIGHT; y++)
			pImg->imageData[(y*IMAGE_WIDTH) + x] = 0;
	}
}

void CPianoPlaying::DrawLineRad(IplImage *pImg, int x, float theta)
{
	int x2 = x + (IMAGE_HEIGHT/tan(theta));
	int y2 = IMAGE_HEIGHT-1;
	int y = 0;

    int dx = abs(x - x2);
    int dy = abs(y - y2);
    double s = double(0.99/(dx>dy?dx:dy));
    double t = 0.0;
    while(t < 1.0)
    {
        dx = int((1.0 - t)*x + t*x2);
        dy = int((1.0 - t)*y + t*y2);
        //putPix(dx, dy, col);
		if ((dx > 0) && (dx < IMAGE_WIDTH))
			pImg->imageData[(dy*IMAGE_WIDTH) + dx] = 255;
		else
			t = 1.0; //END LOOP
        t += s; 
    }
}

void CPianoPlaying::MaskRegion(IplImage *pImg, int x1, int y1, int x2, int y2)
{
	int i, j;

	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if (!(i >= x1 && i <= x2 && j >= y1 && j <= y2))
			{
				pImg->imageData[(j*IMAGE_WIDTH*3)+(i*3)+2] = 0;
				pImg->imageData[(j*IMAGE_WIDTH*3)+(i*3)+1] = 0;
				pImg->imageData[(j*IMAGE_WIDTH*3)+(i*3)] = 0;
			}
		}
	}
}

void CPianoPlaying::ClearRegion(IplImage *pImg, int x1, int y1, int x2, int y2)
{
	int i, j;
	int temp;

	if (x1 > x2)
	{
		temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if (y1 > y2)
	{
		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if (i >= x1 && i <= x2 && j >= y1 && j <= y2)
			{
				pImg->imageData[(j*IMAGE_WIDTH)+(i)] = 0;
			}
		}
	}
}

void CPianoPlaying::AdjustImages(void)
{
	CIplImage tempImage;

	tempImage.copyRotated(&m_imc_displayImage, m_Piano.m_dDegreeRotation);
	cvCopy(tempImage.getIplImage(), m_imc_displayImage.getIplImage());

	MaskRegion(m_imc_displayImage.getIplImage(), 0, m_Piano.m_nKeyboardY1, IMAGE_WIDTH, m_Piano.m_nKeyboardY2);
}

void CPianoPlaying::computeBlob(CIplImage *iplImage, CIplImage *m_imgBlobs, int rootX, int rootY, int value,
								 int *x1, int *y1, int *x2, int *y2) {

	if(!iplImage) return;

	int w = iplImage->getWidth();
	int h = iplImage->getHeight();

	if(rootX < 0 || rootX >= w ||
		rootY < 0 || rootY >= h) return ;

	int x=0, y=0;
	int stackSize=0;
	int *stencilBuffer = new int[w*h];  // buffer used to store visited pixels
	int *stack[2];
	int floodWindowSize = 3;
	stack[0] = new int[w*h];
	stack[1] = new int[w*h];

	//reset bounds variables
	*x1 = *y1 = 500;
	*x2 = *y2 = 0;

	// Initialize the stencil buffer
	for(x=0; x<w*h; x++) {
		stencilBuffer[x] = 0;
	}

	// Initialize the stack
	for(x=0; x<w*h; x++) {
		stack[0][x] = 0;
		stack[1][x] = 0;
	}

	x = rootX;
	y = rootY;
	stencilBuffer[y*w+x] = 1;

	stack[0][stackSize] = x;
	stack[1][stackSize] = y;
	stackSize++;

	while(stackSize>0) {
		stackSize--;
		x = stack[0][stackSize];
		y = stack[1][stackSize];
		stack[0][stackSize] = 0;
		stack[1][stackSize] = 0;

		// update the blob image
		m_imgBlobs->getIplImage()->imageData[y*w+x] = (unsigned char)value;
		iplImage->getIplImage()->imageData[y*w+x] = (unsigned char)value;

		//Bounds
		if (*x1 > x)
			*x1 = x;
		if (*x2 < x)
			*x2 = x;
		if (*y1 > y)
			*y1 = y;
		if (*y2 < y)
			*y2 = y;

		for(int ny=-floodWindowSize/2; ny<=floodWindowSize/2; ny++) {
			for(int nx=-floodWindowSize/2; nx<=floodWindowSize/2; nx++) {
				if(!(nx==0 && ny==0) && x+nx >= 0 && x+nx < w && y+ny >= 0 && y+ny < h &&
					!stencilBuffer[(y+ny)*w+(x+nx)] && ((unsigned char)iplImage->getIplImage()->imageData[(y+ny)*w+(x+nx)] == (unsigned char)255)) {

					// && isSkin(iplImage, x+nx, y+ny)
					stencilBuffer[(y+ny)*w+(x+nx)] = 1;
					if(stackSize < w*h) {
						stack[0][stackSize] = x+nx;
						stack[1][stackSize] = y+ny;
						stackSize++;
					}

				}
			}
		}
	}


	delete [] stencilBuffer;
	delete [] stack[0];
	delete [] stack[1];

}

bool CPianoPlaying::isSkin(CIplImage *pImage,  int x, int y)
{
	unsigned char bgr[3];
	double yy, Cr, Cb;

	bgr[2] = (unsigned char)pImage->getIplImage()->imageData[(y*3*160)+(x*3)+2];
	bgr[1] = (unsigned char)pImage->getIplImage()->imageData[(y*3*160)+(x*3)+1];
	bgr[0] = (unsigned char)pImage->getIplImage()->imageData[(y*3*160)+(x*3)+0];
	

	RGB2YCrCb_jp(bgr[2], bgr[1], bgr[0], &yy, &Cr, &Cb);
	return  isSkinYCrCb((int)yy, (int)Cr, (int)Cb);	
}

void CPianoPlaying::RGB2YCrCb_jp( double R,double G,double B,
					   double * Y,double* Cr,double* Cb )
{
	*Y  = 0.29900*R + 0.58700*G + 0.11400*B ;
	*Cr = 0.50000*R - 0.41869*G - 0.08131*B ;
	*Cb =-0.16874*R - 0.33126*G + 0.50000*B ;
}

bool CPianoPlaying::isSkinYCrCb(int Y, int Cr, int Cb)
{
	double T1, T2, T3, T4;

	if (Y<=128) 
	{
		T1=-2+(256-Y)/16.0;	T2=20-(256-Y)/16.0;	T3=6;		T4=-8;
	}
	else
	{
		T1=6;				T2=12;				T3=2+Y/32.0;	T4=-16+Y/16.0;
	}

	return (Cr >= -2*(Cb+24) && Cr >= -(Cb+17)  && Cr >= -4*(Cb+32) &&
				Cr >= 2.5*(Cb+T1)   &&   Cr >= T3   &&   Cr >= -0.5*(Cb-T4) &&
				Cr <= (220-Cb)/6  && Cr <= -1.34*(Cb-T2) );
}

void CPianoPlaying::ScaleImage(IplImage *source, IplImage *dest, float factor)
{
	int i, j;
	int newWidth, newHeight;

	newWidth = int(factor*160);
	newHeight = int(factor*120);

	for (i=0; i < 160; i++)
	{
		for (j=0; j < 120; j++)
		{
			if ((factor*j) < 120 && (factor*i) < 160)
				dest->imageData[int(j*factor)*160+int(i*factor)] = source->imageData[j*160+i];
		}
	}
}

//Polarizes Images by separating it into Thresholded Black and White
void CPianoPlaying::PolarizeImage(IplImage *image, IplImage *dest, int threshold)
{
	int i, j;
	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if ((unsigned char)image->imageData[j*IMAGE_WIDTH+i] > threshold)
				dest->imageData[j*IMAGE_WIDTH+i] = 0;
			else
				dest->imageData[j*IMAGE_WIDTH+i] = 255;
		}
	}
}

//BestLine - used to calculate line which passes through the most points
void CPianoPlaying::BestLine(int *x, int *y, int totalPoints, float maxError, int *x1, int *y1, int *x2, int *y2, int *elem)
{
	int i, j, z;
	float slope;
	float checkSlope;
	int totalFit;
	
	int highFit=0;
		
	//For Each Starting Point
	for (i=0; i < (totalPoints-1); i++)
	{
		//For Each End Point
		for (j=(i+1); j < totalPoints; j++)
		{
			//Calculate Slope
			if (x[j] != x[i])
				slope = float(y[j] - y[i]) / float(x[j] - x[i]);
			else
				slope = 8000; // Number instead of infinite slope

			if (slope == 0)
				slope = 0.00001;

			totalFit = 0;
			//For Each Point to lie on Slope of Start and End
			for (z=0; z < totalPoints; z++)
			{
				if (x[z] != x[i])
					checkSlope = float(y[z] - y[i]) / float(x[z] - x[i]);
				else 
					checkSlope = 3000; //Number instead of infinite slope

				//Check Percentage Error
				if (fabs((slope - checkSlope)) < maxError)
					totalFit++;
			}

			//Update Highest Match
			if (totalFit > highFit)
			{
				highFit = totalFit;
				*x1 = x[i];
				*y1 = y[i];
				*x2 = x[j];
				*y2 = y[j];
				*elem = i;
			}
		}
	}

	//To Rectify Pixel level errors, and find actual best fitting line
	//by using Average

	//Create New Array with only Best Fitting points
	int newPtsX[20];
	int newPtsY[20];
	int counter=0;

	int avgX=0, avgY=0;

	//Slope
	slope = float(*y2 - *y1)/float(*x2 - *x1);
		
	//For Each Point to lie on Slope of Start and End
	for (z=0; z < totalPoints; z++)
	{
		checkSlope = float(y[z] - *y1) / float(x[z] - *x1);
		
		//Check Percentage Error
		if (fabs((slope - checkSlope)) < maxError && (counter < 20))
		{
			newPtsX[counter] = x[z];
			newPtsY[counter] = y[z];
			avgX += x[z];
			avgY += y[z];
			counter++;
		}
	}

	//Use Average Point locations to find endpoint, to find slope
	*x1 = newPtsX[0];
	*y1 = newPtsY[0];
	*x2 = int(float(avgX)/counter);
	*y2 = int(float(avgY)/counter);
}

//BestSize - Finds Highest Match of Key Size, used to define keyboard location
int CPianoPlaying::BestSize(int *cornerY, int *bottomCornerY, int totalLines)
{
	int lengthList[20];
	int countList[20];
	int i;
	int topLength = 0;

	//Load Array with Lengths
	for (i=0; i < totalLines; i++)
	{
		if (i < 20)
			lengthList[i] = abs(cornerY[i] - bottomCornerY[i]);
	}
	
	//Make Sure we are within Array Bounds
	if (totalLines >= 20)
		totalLines = 20;

	//Find the largest match, to accomdate all keys
	for (i=0; i < totalLines; i++)
	{
		if ((lengthList[i] > topLength) && i < 20)
			topLength = lengthList[i];
	}
	
	//Return Length
	return topLength;
}

void CPianoPlaying::FindFingerTipsFromBlobs(CIplImage *iplImage, CIplImage *m_imgBlobs, int rootX, int rootY, int value,
								 int *x1, int *y1, int *x2, int *y2, int *tipX) {

	if(!iplImage) return;

	int w = iplImage->getWidth();
	int h = iplImage->getHeight();

	if(rootX < 0 || rootX >= w ||
		rootY < 0 || rootY >= h) return ;

	int x=0, y=0;
	int stackSize=0;
	int *stencilBuffer = new int[w*h];  // buffer used to store visited pixels
	int *stack[2];
	int floodWindowSize = 3;
	stack[0] = new int[w*h];
	stack[1] = new int[w*h];

	//reset bounds variables
	*x1 = *y1 = 500;
	*x2 = *y2 = 0;

	// Initialize the stencil buffer
	for(x=0; x<w*h; x++) {
		stencilBuffer[x] = 0;
	}

	// Initialize the stack
	for(x=0; x<w*h; x++) {
		stack[0][x] = 0;
		stack[1][x] = 0;
	}

	x = rootX;
	y = rootY;
	stencilBuffer[y*w+x] = 1;

	stack[0][stackSize] = x;
	stack[1][stackSize] = y;
	stackSize++;

	while(stackSize>0) {
		stackSize--;
		x = stack[0][stackSize];
		y = stack[1][stackSize];
		stack[0][stackSize] = 0;
		stack[1][stackSize] = 0;

		// update the blob image
		m_imgBlobs->getIplImage()->imageData[y*w+x] = (unsigned char)value;
		iplImage->getIplImage()->imageData[y*w+x] = (unsigned char)value;

		//Bounds
		if (*x1 > x)
			*x1 = x;
		if (*x2 < x)
			*x2 = x;
		if (*y1 > y)
			*y1 = y;
		if (*y2 < y)
		{
			*y2 = y;
			*tipX = x;
		}

		for(int ny=-floodWindowSize/2; ny<=floodWindowSize/2; ny++) {
			for(int nx=-floodWindowSize/2; nx<=floodWindowSize/2; nx++) {
				if(!(nx==0 && ny==0) && x+nx >= 0 && x+nx < w && y+ny >= 0 && y+ny < h &&
					!stencilBuffer[(y+ny)*w+(x+nx)] && ((unsigned char)iplImage->getIplImage()->imageData[(y+ny)*w+(x+nx)] == (unsigned char)255)) {

					// && isSkin(iplImage, x+nx, y+ny)
					stencilBuffer[(y+ny)*w+(x+nx)] = 1;
					if(stackSize < w*h) {
						stack[0][stackSize] = x+nx;
						stack[1][stackSize] = y+ny;
						stackSize++;
					}

				}
			}
		}
	}


	delete [] stencilBuffer;
	delete [] stack[0];
	delete [] stack[1];

}

/*void CPianoPlaying::LocateIntensityIncreases(CIplImage *colourImage, CIplImage *dest, CIplImage *mask)
{
	CIplImage intensityImage;
	intensityImage.initialize(m_nWidth, m_nHeight, 8);
	intensityImage.copyBW(colourImage);

	//Mark All Relative Increases in Intensity, Horizontally
	int i, j;

	int prevIntensityTotal;
	int currIntensityTotal;

	bool down = false;
	bool up = false;

	unsigned char recordColour;

	for (j=1; j < (m_Piano.m_nKeyboardY2); j++)
	{
		prevIntensityTotal=0;
		down = false;
		up = false;
		for (i=1; i < IMAGE_WIDTH; i++)
		{

			if ((unsigned char)mask->getIplImage()->imageData[j*IMAGE_WIDTH + i] == 255)
			{
				currIntensityTotal = (unsigned char)intensityImage.getIplImage()->imageData[(j)*IMAGE_WIDTH + i];

				if (currIntensityTotal > prevIntensityTotal)
				{
					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;

//					m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+0] = 0;
//					m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+1] = 0;
//					m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+2] = 0;
					prevIntensityTotal = currIntensityTotal;
					up = true;
				}
				else if (currIntensityTotal < (prevIntensityTotal - 10))
				{
					if (up == true)
					{
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 255;
						
						//CHANGED
						m_imb_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+i]=0;
//						m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+0] = 255;
//						m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+1] = 255;
//						m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+2] = 255;
					}
					else
					{
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
			//			m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+0] = 0;
			//			m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+1] = 0;
			//			m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+2] = 0;
					}
					
					up = false;
				
					prevIntensityTotal = currIntensityTotal;
				}
			}
			else
			{
				dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
			//	m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+0] = 0;
			//	m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+1] = 0;
			//	m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+2] = 0;
//				prevIntensityTotal = 0;
			}
		}
	}

	for (j=1; j < (m_Piano.m_nKeyboardY2); j++)
	{
		prevIntensityTotal=0;
		down = false;
		up = false;
		for (i=IMAGE_WIDTH; i >= 0; i--)
		{

			if ((unsigned char)mask->getIplImage()->imageData[j*IMAGE_WIDTH + i] == 255)
			{
				currIntensityTotal = (unsigned char)intensityImage.getIplImage()->imageData[(j)*IMAGE_WIDTH + i];

				if (currIntensityTotal < prevIntensityTotal)
				{
	//				dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					prevIntensityTotal = currIntensityTotal;
					up = true;
				}
				else if (currIntensityTotal > (prevIntensityTotal + 10))
				{
					if (up == true)
					{
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 255;
						
						//CHANGEd
						m_imb_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+i]=0;
					//	m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+0] = 255;
					//	m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+1] = 255;
					//	m_imc_displayImage.getIplImage()->imageData[(j*3)*IMAGE_WIDTH+(i*3)+2] = 255;
					}
	//				else
	//					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					
					up = false;
				
					prevIntensityTotal = currIntensityTotal;
				}
			}
			else
			{
	//			dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
//				prevIntensityTotal = 0;
			}
		}
	}
	
	//Median Smoothing
	//cvSmooth(dest->getIplImage(), intensityImage.getIplImage(), CV_MEDIAN, 3, 3, 0);
	//cvCopy(intensityImage.getIplImage(), dest->getIplImage(), NULL);
}
*/

/*void CPianoPlaying::LocateIntensityIncreases(CIplImage *colourImage, CIplImage *dest, CIplImage *mask)
{
	CIplImage intensityImage;
	intensityImage.initialize(m_nWidth, m_nHeight, 8);
	intensityImage.copyBW(colourImage);

	//Mark All Relative Increases in Intensity, Horizontally
	int i, j;

	unsigned char currIntensity;
	unsigned char prevIntensity = 0;

	for (j=1; j < (m_Piano.m_nKeyboardY2); j++)
	{
		prevIntensity=0;

		for (i=1; i < IMAGE_WIDTH; i++)
		{
			if ((unsigned char)mask->getIplImage()->imageData[j*IMAGE_WIDTH + i] == 255)
			{	
				currIntensity = (unsigned char)intensityImage.getIplImage()->imageData[j*IMAGE_WIDTH+i];

				if ((currIntensity - prevIntensity) < 0)
				{
					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 255;
					prevIntensity = currIntensity;
				}
				else
				{
					prevIntensity = currIntensity;
					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
				}
			}
			else
				dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
		}
	}
	
	//Median Smoothing
	cvSmooth(dest->getIplImage(), intensityImage.getIplImage(), CV_MEDIAN, 3, 3, 0);
	cvCopy(intensityImage.getIplImage(), dest->getIplImage(), NULL);
}
/**/
void CPianoPlaying::FillVerticalGaps(CIplImage *image, int gapThreshold)
{
	//Get Rid of Vertical Gaps in fingers
	int maxGap=0;
	int i,j;
	bool inProgress = false;

	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if ((unsigned char)image->getIplImage()->imageData[j*IMAGE_WIDTH + i] == 255)
			{
				if (maxGap < gapThreshold)
				{
					while (maxGap > 0)
					{
						image->getIplImage()->imageData[(j-maxGap)*IMAGE_WIDTH+i] = 255;
//						m_imb_handsImage.getIplImage()->imageData[(j-maxGap)*IMAGE_WIDTH+i] = 0;
						maxGap--;
					}
				}
				else
					maxGap=0;
					inProgress = true;
			}
			else
				maxGap++;
			
		}
	}
}


void CPianoPlaying::ClearHorizontalGaps(CIplImage *image, int gapThreshold)
{
	//Get Rid Thin lines connecting fingers, or random lit pixels
	//to make sure ComputeBlob code doesn't think two fingers are one
	//if they are connected
	int vertTotal = 0;
	int vertTotal2 = 0;
	int vertTotal3 = 0;

	int i, j;
	for (i=0; i < IMAGE_WIDTH; i += 3)
	{
		//Use 3 vertical strips
		vertTotal=0;
		vertTotal2=0;
		vertTotal3=0;

		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if ((unsigned char)image->getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
				vertTotal++;
			if ((unsigned char)image->getIplImage()->imageData[j*IMAGE_WIDTH+i+1] == 255)
				vertTotal2++;
			if ((unsigned char)image->getIplImage()->imageData[j*IMAGE_WIDTH+i+2] == 255)
				vertTotal3++;
		}

		if (gapThreshold == 1 && vertTotal < 5)
			ClearRegion(image->getIplImage(), i, 0, i, IMAGE_HEIGHT);
		if (gapThreshold == 2 && vertTotal < 5 && vertTotal2 < 5)
			ClearRegion(image->getIplImage(), i, 0, i+1, IMAGE_HEIGHT);
		if (gapThreshold == 3 && vertTotal < 5 && vertTotal2 < 5 && vertTotal3 < 5)
			ClearRegion(image->getIplImage(), i, 0, i+2, IMAGE_HEIGHT);
	}		
}

void CPianoPlaying::LocateHands(CIplImage *image, CPianoHand *hands, int *numHands)
{
	int i, j;
	int x1, y1, x2, y2;

	CIplImage tempImage;
	tempImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	
	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if ((unsigned char)image->getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
			{
				computeBlob(image, &tempImage, i, j, 100, &x1, &y1, &x2, &y2);

				if ((x2 - x1) > 15 && *numHands < 4)
				{
					hands[*numHands] = *(new CPianoHand(*numHands, x1, y1, x2, y2));
					(*numHands)++;
				}
			}

		}
	}
}

void CPianoPlaying::TraceFingerLines(IplImage *pImg, CvPoint *fingers, int *numFingers)
{
	int i, j;
	int x, y;
	int newX;
	int val;

	const MAXLENGTH = 1;
	int totalCount = 0;

	int topX2[5];
	int topY2[5];
	int topX[5];
	int topY[5];
	int topVal[5];

	int straight;

	CvPoint pointArray[50];
	int numPoints;

	totalCount = 0;
	int q;
	sprintf(keyString, "");

	float slope;

	m_nNumLines=0;
	for (j=IMAGE_HEIGHT-1; j >= 0; j--)
	{
		for (i=0; i < IMAGE_WIDTH; i++)
		{	
			newX = 0;
			y=0;
			val = 0;
			straight = 0;

			numPoints=0;
			
			if (unsigned char(pImg->imageData[(j*IMAGE_WIDTH)+i]) > 0)
				val = LineSizeRecursive2(pImg, i, j, &newX, &y, 0, 0, &straight, pointArray, &numPoints);
			else
				val = 0;

			//if (val > 8 && straight >= int(float(val)/-1))
			if (val > 5 && totalCount < 20)
			{
				//Set Up Fingers
				 fingers[totalCount].x = i;
				 fingers[totalCount].y = j;

				 //Set Up Line Info
				 lines[m_nNumLines].numPoints = numPoints;

				 //Copy Lines to new Array and Display Them
				 for (q=0; q < numPoints; q++)
				 {
/*					 m_imc_displayImage.getIplImage()->imageData[(pointArray[q].y)*3*IMAGE_WIDTH+(pointArray[q].x*3)] = 255;
					 m_imc_displayImage.getIplImage()->imageData[(pointArray[q].y)*3*IMAGE_WIDTH+(pointArray[q].x*3)+1] = 255;
					 m_imc_displayImage.getIplImage()->imageData[(pointArray[q].y)*3*IMAGE_WIDTH+(pointArray[q].x*3)+2] = 255;

					 m_img_tempImage.getIplImage()->imageData[(pointArray[q].y)*IMAGE_WIDTH+(pointArray[q].x)] = 255;
*/
					 lines[m_nNumLines].points[q] = pointArray[q];
				 }

				 lines[m_nNumLines].mark = false;

				 totalCount++;

				 m_nNumLines++;
			}
		}
	}

	CPointSeries newLines[20];
	int newNumLines=0;

	ConnectLines(lines, m_nNumLines, newLines, &newNumLines);
	//lines = newLines;
	//m_nNumLines = newNumLines;

	for (i=0; i < newNumLines; i++)
	{
		for (j=0; j < newLines[i].numPoints; j++)
		{
//			m_imc_displayImage.getIplImage()->imageData[(newLines[i].points[j].y)*3*IMAGE_WIDTH+((newLines[i].points[j].x)*3)] = 255;
//			m_imc_displayImage.getIplImage()->imageData[(newLines[i].points[j].y)*3*IMAGE_WIDTH+((newLines[i].points[j].x)*3)+1] = 255;
//			m_imc_displayImage.getIplImage()->imageData[(newLines[i].points[j].y)*3*IMAGE_WIDTH+((newLines[i].points[j].x)*3)+2] = 255;
						 
			m_img_tempImage.getIplImage()->imageData[(newLines[i].points[j].y)*IMAGE_WIDTH+((newLines[i].points[j].x))] = (255/newNumLines)*i+50;
			
			m_imb_handsImage.getIplImage()->imageData[(newLines[i].points[j].y)*IMAGE_WIDTH+((newLines[i].points[j].x))] = 0;
		}
	}

	//for (i=0; i < m_nNumLines; i++)
	for (i=0; i < newNumLines; i++)
	{
		bool diffDone = false;
		//Trace Up
	//	x= lines[i].points[lines[i].numPoints-1].x;
	//	y= lines[i].points[lines[i].numPoints-1].y;
		x= newLines[i].points[newLines[i].numPoints-1].x;
		y= newLines[i].points[newLines[i].numPoints-1].y;
			
		unsigned char intDiff = 0;
		unsigned char prevInt=0, currInt;
		unsigned char pix1, pix2, pix3;
		unsigned char diff1, diff2, diff3;

		prevInt = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x];

		//Trace Up
		while	(y > 0/* && intDiff < 20*/)
		{
			currInt = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x];

			//Don't Draw Excessive Lines on Colour Image
			if (intDiff > 20)
				diffDone = true;
			if (diffDone == false)
			{
//				m_imc_displayImage.getIplImage()->imageData[y*3*IMAGE_WIDTH+(x*3)] = 255;
//				m_imc_displayImage.getIplImage()->imageData[y*3*IMAGE_WIDTH+(x*3)+1] = 255;
//				m_imc_displayImage.getIplImage()->imageData[y*3*IMAGE_WIDTH+(x*3)+2] = 255;
			}
			
			m_img_tempImage.getIplImage()->imageData[y*IMAGE_WIDTH+(x)] = 155;
            m_imb_handsImage.getIplImage()->imageData[y*IMAGE_WIDTH+(x)] = 0;
						 
			y--;
	
			//Make sure within bounds
			if (x > 0 && x < IMAGE_WIDTH)
			{
				pix1 = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x-1];
				pix2 = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x];
				pix3 = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x+1];
			}
			else
				break;
		
			 diff1 = abs(pix1-currInt);
			 diff2 = abs(pix2-currInt);
			 diff3 = abs(pix3-currInt);

			 if (diff1 <= diff2 && diff1 <= diff3)
				 x--;
			 else if (diff2 <= diff1 && diff2 <= diff3)
				 x=x;
			 else
				 x++;
					 
			 intDiff = abs(prevInt-currInt);
			 prevInt=currInt;
		 }


		 //Trace Down
 		// x= lines[i].points[0].x;
		//y= lines[i].points[0].y;
	
		diffDone = false;
		x= newLines[i].points[0].x;
		y= newLines[i].points[0].y;
		 intDiff=0;
		 prevInt = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x];
		 while	(y < IMAGE_HEIGHT/* && intDiff < 20*/)
		 {
			 currInt = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x];

			 //Don't Draw Excessive Lines on Colour Image
			 if (intDiff > 20)
				 diffDone = true;
			 if (diffDone == false)
			 {
//				m_imc_displayImage.getIplImage()->imageData[y*3*IMAGE_WIDTH+(x*3)] = 255;
//				m_imc_displayImage.getIplImage()->imageData[y*3*IMAGE_WIDTH+(x*3)+1] = 255;
//				m_imc_displayImage.getIplImage()->imageData[y*3*IMAGE_WIDTH+(x*3)+2] = 255;
			 }
					 
			 m_img_tempImage.getIplImage()->imageData[y*IMAGE_WIDTH+(x)] = 155;

			 m_imb_handsImage.getIplImage()->imageData[y*IMAGE_WIDTH+(x)] = 0;
					 
			 y++;

			 //Make sure iwthin bounds
			 if (x > 0 && x < IMAGE_WIDTH)
			 {
				 pix1 = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x-1];
				 pix2 = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x];
			     pix3 = (unsigned char)m_img_bwImage.getIplImage()->imageData[y*IMAGE_WIDTH+x+1];
			 }

			 diff1 = abs(pix1-currInt);
			 diff2 = abs(pix2-currInt);
			 diff3 = abs(pix3-currInt);

			 if (diff1 <= diff2 && diff1 <= diff3)
				 x--;
			 else if (diff2 <= diff1 && diff2 <= diff3)
				 x=x;
			 else
				 x++;
					 
			 intDiff = abs(prevInt-currInt);
			 prevInt=currInt;
		 }
	}

	/*
	CvRect newLines[30];
	int newCount;
	ConnectLines(lines, m_nNumLines, newLines, &newCount);*/

	*numFingers = newNumLines;
}

int CPianoPlaying::LineSizeRecursive2(IplImage *pImg, int i, int j, int *x, int *y, int gap, int dir, int *straight, CvPoint *pointArray, int *numPoints)
{
	int curTotal, curTotal2, curTotal3;
	int curTotal4, curTotal5;
	int x1=0, x2=0, x3=0;
	int y1=0, y2=0, y3=0;
	int straight1, straight2, straight3;
	
	int totalLines = 0;
	unsigned char val = 0;
	unsigned tempVal = 0;

	CvPoint pt1[50], pt2[50], pt3[50];
	int ptCount1=0, ptCount2=0, ptCount3=0;

	int r, s;

	if (j >= 0 && j < IMAGE_HEIGHT && i >= 0 && i < IMAGE_WIDTH)
		val = (unsigned char)pImg->imageData[(j*IMAGE_WIDTH)+i];
	else
		val = 0;

	if ((val == (unsigned char)255)/* || (gap < 1)*/)
	{
		x1 = *x;
		y1 = *y;
		x2 = *x;
		y2 = *y;
		x3 = *x;
		y3 = *y;

		straight1 = *straight - abs(dir - 0);
		straight2 = *straight - abs(dir - 1);
		straight3 = *straight - abs(dir - 2);
		
		if (val == (unsigned char)0)
			gap++;
		else
			gap=0;

		curTotal = LineSizeRecursive2(pImg, i-1, j-1, &x1, &y1, gap, 0, &straight1, pt1, &ptCount1);
		curTotal2 = LineSizeRecursive2(pImg, i, j-1, &x2, &y2, gap, 1, &straight2, pt2, &ptCount2);
		curTotal3 = LineSizeRecursive2(pImg, i+1, j-1, &x3, &y3, gap, 2, &straight3, pt3, &ptCount3);
		
		pointArray[0].x = i; 
		pointArray[0].y = j;

		if ((curTotal >= curTotal2) && (curTotal >= curTotal3))
		{
			totalLines = curTotal;
			*x = ((x1) - 1);
			*y = ((y1) - 1);
			*straight = straight1;
			
			for (r=0; r < ptCount1; r++)
				pointArray[r+1] = pt1[r];

			*numPoints = ptCount1+1;
		}
		else if ((curTotal2  >= curTotal) && (curTotal2 >= curTotal3))
		{
			totalLines = curTotal2;
			*x = x2;
			*y = ((y2) - 1);
			*straight = straight2;

			for (r=0; r < ptCount2; r++)
				pointArray[r+1] = pt2[r];
			*numPoints = ptCount2+1;
		}
		else
		{
			totalLines = curTotal3;
			*x = ((x3) + 1);
			*y = ((y3) - 1);
			*straight = straight3;

			for (r=0; r < ptCount3; r++)
				pointArray[r+1] = pt3[r];
			
			*numPoints = ptCount3+1;
		}
	}
	else
	{
		return 0;
	}
	
	pImg->imageData[(j*IMAGE_WIDTH)+i] = 0;


	return totalLines + 1;

}

void CPianoPlaying::ConnectLines(CPointSeries *lines, int totalCount, CvRect *newLines, int *newCount)
{
	int i, j;
	float slope1, slope2;
	float testSlope;

	for (i=0; i < totalCount; i++)
	{
		for (j=0; j < totalCount; j++)
		{
			testSlope = float(lines[j].points[0].y - lines[i].points[(lines[i].numPoints-1)].y)/float(lines[j].points[0].x - lines[i].points[(lines[i].numPoints-1)].x);
			if (abs(lines[j].points[0].x - lines[i].points[(lines[i].numPoints-1)].x) <= 3)
			{
				if (lines[j].points[0].y < lines[i].points[(lines[i].numPoints-1)].y)
				{
					if (testSlope > 10)
					{
					DrawLine(m_img_tempImage.getIplImage(), 
					lines[i].points[(lines[i].numPoints-1)].x, lines[i].points[(lines[i].numPoints-1)].y, 
					lines[j].points[0].x, lines[j].points[0].y, 1);
					fprintf(fDataFile, "%d:  -  l : %d, %d, h: %d, %d\n", i, lines[i].points[(lines[i].numPoints-1)].x, lines[i].points[(lines[i].numPoints-1)].y, 
					lines[j].points[0].x, lines[j].points[0].y);

					DrawBlackLine(m_imb_handsImage.getIplImage(), 
					lines[i].points[(lines[i].numPoints-1)].x, lines[i].points[(lines[i].numPoints-1)].y, 
					lines[j].points[0].x, lines[j].points[0].y, 1);
					//fprintf(fDataFile, "%d:  -  l : %d, %d, h: %d, %d\n", i, lines[i].points[(lines[i].numPoints-1)].x, lines[i].points[(lines[i].numPoints-1)].y, 
					//lines[j].points[0].x, lines[j].points[0].y);
					}
				}
			}

/*			if (abs((lines[i].x + lines[i].width) - (lines[j].x)) < 5)
			{
				if (abs((lines[i].y + lines[i].height) - lines[j].y) < 5)
				{
					DrawLine(m_imb_edgeDetectedImage.getIplImage(), lines[j].x, lines[j].y, 
						(lines[i].x+lines[i].width), (lines[i].y+lines[i].height), 1);
				}
			}
*/
/*			slope1 = float(lines[i].height)/float(lines[i].width);
			slope2 = float(lines[j].height)/float(lines[j].width);

			testSlope = float((lines[i].y+lines[i].height)-(lines[j].y))/float((lines[i].x+lines[i].width)-(lines[j].x));

			if ((abs((testSlope/slope1) - 1) < 1.25) && i != j)
			{
				DrawLine(m_imb_edgeDetectedImage.getIplImage(), lines[j].x, lines[j].y, 
					(lines[i].x+lines[i].width), (lines[i].y+lines[i].height), 1);
			}

		*/	
		}
	}

}

void CPianoPlaying::LocateIntensityIncreases(CIplImage *colourImage, CIplImage *dest, CIplImage *mask)
{
	CIplImage intensityImage;
	intensityImage.initialize(m_nWidth, m_nHeight, 8);
	intensityImage.copyBW(colourImage);

	//Mark All Relative Increases in Intensity, Horizontally
	int i, j;

	int prevIntensityTotal;
	int currIntensityTotal;

	bool down = false;
	bool up = false;
	bool upAgain = false;

	unsigned char recordColour;

	for (j=1; j < (m_Piano.m_nKeyboardY2); j++)
	{
		prevIntensityTotal=0;
		down = false;
		up = false;
		upAgain=false;
		prevIntensityTotal=255;
		for (i=1; i < IMAGE_WIDTH; i++)
		{
			if ((unsigned char)mask->getIplImage()->imageData[j*IMAGE_WIDTH + i] == 255)
			{
				currIntensityTotal = (unsigned char)intensityImage.getIplImage()->imageData[(j)*IMAGE_WIDTH + i];

				if (currIntensityTotal < prevIntensityTotal)
				{
					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					prevIntensityTotal = currIntensityTotal;
					down=true;
				}
				else if (currIntensityTotal > (prevIntensityTotal + 5))
				{
					if (down == true)
					{
						down = false;
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 255;
						
						//CHANGED
	//					m_imb_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+i]=0;
					}
					else
					{
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					}
					
					down = false;	
				//	up = false;
				
					prevIntensityTotal = currIntensityTotal;
				}
			}
			else
			{
				dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
			}
		}
	}
/*	
	for (i=1; i < IMAGE_WIDTH; i++)
	{
		prevIntensityTotal=0;
		down = false;
		up = false;
		upAgain=false;
		prevIntensityTotal=255;
		
		for (j=1; j < (m_Piano.m_nKeyboardY2); j++)
		{
			if ((unsigned char)mask->getIplImage()->imageData[j*IMAGE_WIDTH + i] == 255)
			{
				currIntensityTotal = (unsigned char)intensityImage.getIplImage()->imageData[(j)*IMAGE_WIDTH + i];

				if (currIntensityTotal < prevIntensityTotal)
				{
				//	dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					prevIntensityTotal = currIntensityTotal;
					down=true;
				}
				else if (currIntensityTotal > (prevIntensityTotal + 10))
				{
					if (down == true)
					{
						down = false;
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 255;
						
						//CHANGED
						m_imb_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+i]=0;
					}
					else
					{
					//	dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					}
					
					down = false;	
				//	up = false;
				
					prevIntensityTotal = currIntensityTotal;
				}
			}
			else
			{
				//dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
			}
		}
	}
*/	
}

void CPianoPlaying::RemoveSinglePixels(IplImage *image)
{
	int i, j;

	int y, x;

	for (i=1; i < IMAGE_WIDTH-1; i++)
	{
		for (j=1; j < IMAGE_HEIGHT-1; j++)
		{
			if (image->imageData[j*IMAGE_WIDTH+i] == 255)
			{
				if ((image->imageData[j*IMAGE_WIDTH+i-1] == 0) && (image->imageData[j*IMAGE_WIDTH+i+1] == 0) &&
					(image->imageData[(j-1)*IMAGE_WIDTH+i-1] == 0) && (image->imageData[(j-1)*IMAGE_WIDTH+i] == 0) && (image->imageData[(j-1)*IMAGE_WIDTH+i+1] == 0) &&
					(image->imageData[(j+1)*IMAGE_WIDTH+i-1] == 0) && (image->imageData[(j+1)*IMAGE_WIDTH+i] == 0) && (image->imageData[(j+1)*IMAGE_WIDTH+i+1] == 0))
						image->imageData[j*IMAGE_WIDTH+i] = 0;
				
			}
		}
	}
}

void CPianoPlaying::FillSinglePixels(IplImage *image)
{
	int i, j;

	int y, x;

	for (i=1; i < IMAGE_WIDTH-1; i++)
	{
		for (j=1; j < IMAGE_HEIGHT-1; j++)
		{
			if (image->imageData[j*IMAGE_WIDTH+i] == 0)
			{
				if ((image->imageData[j*IMAGE_WIDTH+i-1] == 255) && (image->imageData[j*IMAGE_WIDTH+i+1] == 255) &&
					(image->imageData[(j-1)*IMAGE_WIDTH+i-1] == 255) && (image->imageData[(j-1)*IMAGE_WIDTH+i] == 255) && (image->imageData[(j-1)*IMAGE_WIDTH+i+1] == 255) &&
					(image->imageData[(j+1)*IMAGE_WIDTH+i-1] == 255) && (image->imageData[(j+1)*IMAGE_WIDTH+i] == 255) && (image->imageData[(j+1)*IMAGE_WIDTH+i+1] == 255))
						image->imageData[j*IMAGE_WIDTH+i] = 255;
				
			}
		}
	}
}


void CPianoPlaying::ConnectLines(CPointSeries *lines, int totalCount, CPointSeries *newLines, int *newCount)
{
	int i, j;
	float slope1, slope2;
	float testSlope;

	int newLineCount=0;

	*newCount=0;

	for (i=0; i < totalCount; i++)
	{
		if (lines[i].mark == false)
		{
			newLines[newLineCount].numPoints=0;
			newLines[newLineCount].AddPointSeries(&(lines[i]));

			for (j=0; j < totalCount; j++)
			{
				if (lines[j].mark == false && j != i)
				{
					//testSlope = float(lines[j].points[0].y - lines[i].points[(lines[i].numPoints-1)].y)/float(lines[j].points[0].x - lines[i].points[(lines[i].numPoints-1)].x + .001);
					//testSlope = float(lines[j].points[0].y - lines[i].points[0].y)/float(lines[j].points[0].x - lines[i].points[0].x + .001);
					
					testSlope = float(lines[j].points[(lines[j].numPoints-1)].y - lines[i].points[(lines[i].numPoints-1)].y)/float(lines[j].points[(lines[j].numPoints-1)].x - lines[i].points[(lines[i].numPoints-1)].x + .001);

					slope1 = float(lines[j].points[(lines[j].numPoints-1)].y - lines[j].points[0].y)/float(lines[j].points[(lines[j].numPoints-1)].x - lines[j].points[0].x + 0.001);
					slope2 = float(lines[i].points[(lines[i].numPoints-1)].y - lines[i].points[0].y)/float(lines[i].points[(lines[i].numPoints-1)].x - lines[i].points[0].x + 0.001);

					//Make sure that hte lines are sufficiently close
					if (abs(lines[j].points[0].y - lines[i].points[(lines[i].numPoints-1)].y) <= 2)
					{
						if (abs(lines[j].points[0].x - lines[i].points[(lines[i].numPoints-1)].x) <= 3)	
						{
						//	if (testSlope > 10)
						//	{
//							DrawBox(m_imc_displayImage.getIplImage(), lines[j].points[0].x-1, lines[j].points[0].y-1,lines[j].points[0].x+1,lines[j].points[0].y+1, 3);
								newLines[newLineCount].AddLine(lines[i].points[(lines[i].numPoints-1)], lines[j].points[0]);
								newLines[newLineCount].AddPointSeries(&(lines[j]));
								lines[j].mark = true;
								lines[i].mark = true;

								j=totalCount;
						//	}
						}
					}
				/*	else if (fabs((slope1/slope2) - 1) < 0.5 && fabs((testSlope/slope2) - 1) < 0.5)
					{
						DrawBox(m_imc_displayImage.getIplImage(), lines[j].points[0].x-1, lines[j].points[0].y-1,lines[j].points[0].x+1,lines[j].points[0].y+1, 3);
						//newLines[newLineCount].AddLine(lines[i].points[(lines[i].numPoints-1)], lines[j].points[0]);
						newLines[newLineCount].AddLine(lines[j].points[(lines[j].numPoints-1)], lines[i].points[0]);
						newLines[newLineCount].AddPointSeries(&(lines[j]));
						lines[j].mark = true;
						lines[i].mark = true;
						j=totalCount;
					}*/
				}
			}
			newLineCount++;
		}
	}

	*newCount = newLineCount;
}

void CPianoPlaying::FindFingerTipsFromBlobs2(CIplImage *iplImage, CIplImage *m_imgBlobs, int rootX, int rootY, int value,
								 int *x1, int *y1, int *x2, int *y2, int *tipX, int *tipXhigh) 
{

	if(!iplImage) return;

	int w = iplImage->getWidth();
	int h = iplImage->getHeight();

	if(rootX < 0 || rootX >= w ||
		rootY < 0 || rootY >= h) return ;

	int x=0, y=0;
	int stackSize=0;
	int *stencilBuffer = new int[w*h];  // buffer used to store visited pixels
	int *stack[2];
	int floodWindowSize = 3;
	stack[0] = new int[w*h];
	stack[1] = new int[w*h];

	//reset bounds variables
	*x1 = *y1 = 500;
	*x2 = *y2 = 0;

	// Initialize the stencil buffer
	for(x=0; x<w*h; x++) {
		stencilBuffer[x] = 0;
	}

	// Initialize the stack
	for(x=0; x<w*h; x++) {
		stack[0][x] = 0;
		stack[1][x] = 0;
	}

	x = rootX;
	y = rootY;
	stencilBuffer[y*w+x] = 1;

	stack[0][stackSize] = x;
	stack[1][stackSize] = y;
	stackSize++;

	while(stackSize>0) {
		stackSize--;
		x = stack[0][stackSize];
		y = stack[1][stackSize];
		stack[0][stackSize] = 0;
		stack[1][stackSize] = 0;

		// update the blob image
		m_imgBlobs->getIplImage()->imageData[y*w+x] = (unsigned char)value;
		iplImage->getIplImage()->imageData[y*w+x] = (unsigned char)value;

		//Bounds
		if (*x1 > x)
			*x1 = x;
		if (*x2 < x)
			*x2 = x;
		if (*y1 > y)
		{
			*y1 = y;
			*tipXhigh = x;
		}
		if (*y2 < y)
		{
			*y2 = y;
			*tipX = x;
		}

		for(int ny=-floodWindowSize/2; ny<=floodWindowSize/2; ny++) {
			for(int nx=-floodWindowSize/2; nx<=floodWindowSize/2; nx++) {
				if(!(nx==0 && ny==0) && x+nx >= 0 && x+nx < w && y+ny >= 0 && y+ny < h &&
					!stencilBuffer[(y+ny)*w+(x+nx)] && ((unsigned char)iplImage->getIplImage()->imageData[(y+ny)*w+(x+nx)] == (unsigned char)255)) {

					// && isSkin(iplImage, x+nx, y+ny)
					stencilBuffer[(y+ny)*w+(x+nx)] = 1;
					if(stackSize < w*h) {
						stack[0][stackSize] = x+nx;
						stack[1][stackSize] = y+ny;
						stackSize++;
					}

				}
			}
		}
	}


	delete [] stencilBuffer;
	delete [] stack[0];
	delete [] stack[1];

}

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
	m_imc_displayImage.destroy();
	m_imb_dI_colour.destroy();
	m_imb_handsImage.destroy();
	m_imb_edgeDetectedImage.destroy();
	m_imb_fingerDetectedImage.destroy();
	m_img_tempImage.destroy();
	m_img_bwImage.destroy();
	m_ORIMAGE.destroy();

	int i;
	for (i=0; i < m_nNumHands; i++)
		m_Hand[i].destroy();

	fclose(fDataFile);
}


bool CPianoPlaying::updateImages(CIplImage *pImage)
{	
	m_imc_displayImage.initialize(m_nWidth, m_nHeight, 24);
	m_imb_dI_colour.initialize(m_nWidth, m_nHeight, 8);
	m_imb_handsImage.initialize(m_nWidth, m_nHeight, 8);
	m_imb_edgeDetectedImage.initialize(m_nWidth, m_nHeight, 8);
	m_imb_fingerDetectedImage.initialize(m_nWidth, m_nHeight, 8);
	m_img_tempImage.initialize(m_nWidth, m_nHeight, 8);
	m_img_bwImage.initialize(m_nWidth, m_nHeight, 8);
	m_ORIMAGE.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

	m_imc_displayImage.copy(pImage); 
	return true;
}

void CPianoPlaying::setBackgroundImage(CIplImage *pImage, int nPianoHeight)
{
	//Initialize and Set Background Image
	m_imc_backgroundImage.initialize(m_nWidth, m_nHeight, 24);
	m_imc_backgroundImage.copy(pImage);

	m_imc_tempImage.initialize(m_nWidth, m_nHeight, 24);
	m_imc_tempImage2.initialize(m_nWidth, m_nHeight, 24);
		
	//Initialize Piano Keyboard Object with Current Image

	if (USE_CPIANOKEYBOARD == 1)
	{
		m_Piano = new CPianoKeyboard(pImage);
	
		if (nPianoHeight != PVS_DEFAULT)
			m_Piano->m_nKeyboardY1 -= nPianoHeight;
	}
	else
	{
		m_Piano = new CPianoKeyboard();
	}

	//Use Determined Piano characteristics to rotate image
	//And select region of interest
	//CIplImage tempImage;
	//tempImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 24);
	//tempImage.copyRotated(&m_imc_backgroundImage, m_Piano->m_dDegreeRotation);
	//cvCopy(tempImage.getIplImage(), m_imc_backgroundImage.getIplImage());
	//tempImage.destroy();

	//Black Out Everything Except for Keyboard
	MaskRegion(m_imc_backgroundImage.getIplImage(), 0, m_Piano->m_nKeyboardY1, IMAGE_WIDTH, m_Piano->m_nKeyboardY2);
	
	//Draw Box Around Detected "C"  (actually its an "E" right now)
//	DrawBox(m_imc_backgroundImage.getIplImage(), m_Piano->m_nC_X1+1, m_Piano->m_nKeyboardY1, m_Piano->m_nC_X2-1, m_Piano->m_nKeyboardY2, 3);

}

void CPianoPlaying::createHandsImage()
{
	int avgX, avgY;
	unsigned char avgR, avgG, avgB;	

	//Create Black and White Image
	m_img_bwImage.copyBW(&m_imc_displayImage);

	//Detect Changes Based on Colour
	detectChange_Colour(m_imc_displayImage.getIplImage(), m_imc_backgroundImage.getIplImage(),
			m_imb_dI_colour.getIplImage(), m_nHandsDetectThreshold);
	m_imb_handsImage.copy(&m_imb_dI_colour);

	//Draw Middle C
//	DrawBox(m_imc_displayImage.getIplImage(), m_Piano->m_nC_X1, m_Piano->m_nKeyboardY1, m_Piano->m_nC_X2, m_Piano->m_nKeyboardY2, 3);

	//IF Average Colour Checked then
	//Boost Skin Colour Recognition
	if (m_bColourAverage == true)
	{
		int totalPixels = 0;
		
		//Calculate Average Colour of Difference (hopefully Skin Colour)
		CalculateAverageColour(m_imc_displayImage.getIplImage(), m_imb_dI_colour.getIplImage(), 
								&avgR, &avgG, &avgB, &avgX, &avgY);
		//Locate Skin Colour in Image
		//Outputs results in dI_colour
		AddAverageColour(m_imc_displayImage.getIplImage(), m_imb_handsImage.getIplImage(), 
							avgR, avgG, avgB, m_nColourThreshold);

		//Median Smooth Results
		cvSmooth(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), CV_MEDIAN, 3, 3, 0);
		cvCopy(m_img_tempImage.getIplImage(), m_imb_handsImage.getIplImage(), NULL);
	}

	//Show Edge Detected Image
	if (m_bEdgeDetect == true)
	{
		//Clear Edge Detected Image
		ClearImage(m_imb_edgeDetectedImage.getIplImage());
		//Detect Edges
		cvCanny(m_img_bwImage.getIplImage(), m_imb_edgeDetectedImage.getIplImage(), 0, 255, 3);		
		
		//****CHANGED JAN 8, 2006****//
		int i, j;
		for (i=0; i < IMAGE_WIDTH; i++)
		{
			for (j=0; j < IMAGE_HEIGHT; j++)
			{
				if ((unsigned char)m_imb_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] != 255)
					m_imb_edgeDetectedImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] = 0;

			}
		}
	}

	//Show Detected Hands
	if (m_bDetectHands == true || m_nNumHands > 0)
	{
		int i, j;
		m_nTempNumHands = 0;
		m_nTempNumFingers = 0;

		//Create Modifiable copy of Hand Image
		cvCopy(m_imb_handsImage.getIplImage(), m_img_tempImage.getIplImage(), NULL);
		//Locate Hands // Compute number of hands
		LocateHands(&m_imb_handsImage, m_tempHand, &m_nTempNumHands);
		//Clear Temp Hands since Locate Hands is only used for a Hand Count
		for (i=0; i < m_nTempNumHands; i++)
			m_tempHand[i].destroy();
		
		//Restore Hands Image
		cvCopy(m_img_tempImage.getIplImage(), m_imb_handsImage.getIplImage());

		//Stores all pixels that have already been used
		CIplImage usedHandsImage;
		usedHandsImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

		//CHECK IF there are new hands
		if (m_bDetectHands == true)
		{
			//FOR Each Hand
			//Match each existing hand, to another hand
			for (i=0; i < m_nNumHands; i++)
			{
				//Hand Is Ready for Drawing
				m_Hand[i].ready = true;
	
				//Set Bounds
				m_Hand[i].SetBounds(m_Piano->m_nKeyboardY1, m_Piano->m_nKeyboardY2);
				//Search For Updated Hand
				m_Hand[i].SearchForHand(&m_img_tempImage);
				//Segment Hand more Accurately
				m_Hand[i].SegmentBetter(&m_img_bwImage, &m_imb_handsImage);
				//Locate and Update Fingers
				m_Hand[i].DetectFingers(&m_imc_displayImage);

				//Add Hand Image to Used Hands Image
				cvOr(usedHandsImage.getIplImage(), m_Hand[i].m_handsImage.getIplImage(), usedHandsImage.getIplImage(), NULL);
	
				//IF Hand is On Screen
				if (m_Hand[i].onScreen == true)
				{
					//Draw Bounding Box Around Hand
					DrawBox(m_imc_displayImage.getIplImage(), m_Hand[i].m_boundingBox.x, 
					m_Hand[i].m_boundingBox.y, m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width,
					m_Hand[i].m_boundingBox.y+m_Hand[i].m_boundingBox.height, 3);
				
					//Draw Boxes around each Finger
					if (m_bFingerDetect == true)
					{
						for (j=0; j < m_Hand[i].m_nNumFingers; j++)
							DrawBox(m_imc_displayImage.getIplImage(), m_Hand[i].m_fingers[j].x-2, m_Hand[i].m_fingers[j].y-2, 
							m_Hand[i].m_fingers[j].x+2, m_Hand[i].m_fingers[j].y+2, 3);
					}//END IF
				}//END IF m_Hand
			}//END FOR i

			//Remove Already Used Hands
			cvSub(m_imb_handsImage.getIplImage(), usedHandsImage.getIplImage(),
					usedHandsImage.getIplImage(), NULL);
			//Locate Remaining Hands
			LocateHands(&usedHandsImage, m_tempHand, &m_nTempNumHands);
			
			//FOR Each Newly Detected Hand
			for (i=0; i < m_nTempNumHands; i++)
			{
				//IF There aren't already too many hands
				if (m_nNumHands < 4)
				{
					//Add New Hand
					m_Hand[m_nNumHands].Copy(&m_tempHand[i]);
					m_nNumHands++;
					m_tempHand[i].destroy();
				}//END IF < 4
			}//END FOR i
		}
		//NORMAL DRAWING SEQUENCE
		//ELSE same amount, or fewer hands
		else
		{
			//FOR Each Hand
			//Match each existing hand, to another hand
			for (i=0; i < m_nNumHands; i++)
			{
				//Hand Is Ready for Drawing
				m_Hand[i].ready = true;

				//Set Bounds
				m_Hand[i].SetBounds(m_Piano->m_nKeyboardY1, m_Piano->m_nKeyboardY2);
				//Search For Updated Hand
				m_Hand[i].SearchForHand(&m_img_tempImage);
				//Segment Hand more Accurately
				m_Hand[i].SegmentBetter(&m_img_bwImage, &m_imb_handsImage);
				//Locate and Update Fingers
				m_Hand[i].DetectFingers(&m_imc_displayImage);
	
				//IF Hand is On Screen
				if (m_Hand[i].onScreen == true)
				{
					//Draw Bounding Box Around Hand
					DrawBox(m_imc_displayImage.getIplImage(), m_Hand[i].m_boundingBox.x, 
					m_Hand[i].m_boundingBox.y, m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width,
					m_Hand[i].m_boundingBox.y+m_Hand[i].m_boundingBox.height, 3);

					//****CHANGED JAN 8, 2006****//
					//Check if the edges are within range of the slider
					int x, y;
					for (y=0; y < IMAGE_HEIGHT; y++)
					{
						for (x=0; x < IMAGE_WIDTH; x++)
						{
							if (abs(m_nEdgeDetectThreshold - (unsigned char)m_Hand[i].m_edgeImage.getIplImage()->imageData[y*IMAGE_WIDTH+x])  > 30)
									m_Hand[i].m_edgeImage.getIplImage()->imageData[y*IMAGE_WIDTH+x] = (unsigned char)0;
							else
									m_Hand[i].m_edgeImage.getIplImage()->imageData[y*IMAGE_WIDTH+x] = (unsigned char)255;
						}
					}
					


					//Draw Boxes around each Finger
					if (m_bFingerDetect == true)
					{
						m_Hand[i].multiPurposeFlag = true;
						for (j=0; j < m_Hand[i].m_nNumFingers; j++)
							DrawBox(m_imc_displayImage.getIplImage(), m_Hand[i].m_fingers[j].x-2, m_Hand[i].m_fingers[j].y-2, 
							m_Hand[i].m_fingers[j].x+2, m_Hand[i].m_fingers[j].y+2, 3);

						//Overlay Finger Markings
						for (y=0; y < IMAGE_HEIGHT; y++)
						{
							for (x=0; x < IMAGE_WIDTH; x++)
							{
								if ((unsigned char)m_Hand[i].m_edgeImage.getIplImage()->imageData[y*IMAGE_WIDTH+x] == 255)
								{
									m_imc_displayImage.getIplImage()->imageData[(y*3*IMAGE_WIDTH)+(x*3)+2] = (unsigned char)255;
									m_imc_displayImage.getIplImage()->imageData[(y*3*IMAGE_WIDTH)+(x*3)+1] = (unsigned char)255;
									m_imc_displayImage.getIplImage()->imageData[(y*3*IMAGE_WIDTH)+(x*3)+0] = (unsigned char)255;
								}
							}
						}
						//END Overlay Finger Markings
					}//END If Finger Detect
					else
						m_Hand[i].multiPurposeFlag = false;
				}//END IF m_Hand
			}//END FOR i

			ClearImage(m_ORIMAGE.getIplImage());
			for (i=0; i < m_nNumHands; i++)
			{
				for (j=0; j < 5; j++)
					cvOr(m_ORIMAGE.getIplImage(), m_Hand[i].m_fingImg[j].getIplImage(), m_ORIMAGE.getIplImage(), NULL);
			}

		}//END ELSE (m_nTempNumHands > m_nNumHands)

		
		usedHandsImage.destroy();
	}//END IF linedetect


	
	//Show Hand Image within Hand Mask Image
	/*int r, s;
	for (r=0; r < IMAGE_WIDTH; r++)
	{
		for (s=0; s < IMAGE_HEIGHT; s++)
		{
			if ((unsigned char)m_imb_handsImage.getIplImage()->imageData[s*IMAGE_WIDTH+r] == 255)
				m_imb_fingerDetectedImage.getIplImage()->imageData[s*IMAGE_WIDTH+r] = m_img_bwImage.getIplImage()->imageData[s*IMAGE_WIDTH+r];
			else
				m_imb_fingerDetectedImage.getIplImage()->imageData[s*IMAGE_WIDTH+r] = 0;
		}
	}
	m_imb_handsImage.copy(&m_imb_fingerDetectedImage);*/
}

//AdjustImages - Adjusts Images, via Rotation and Masking, to select only the region of 
//interest, in the appropriate rotation
void CPianoPlaying::AdjustImages(void)
{
	CIplImage tempImage;

	//tempImage.copyRotated(&m_imc_displayImage, m_Piano->m_dDegreeRotation);
	//cvCopy(tempImage.getIplImage(), m_imc_displayImage.getIplImage());

	MaskRegion(m_imc_displayImage.getIplImage(), 0, m_Piano->m_nKeyboardY1, IMAGE_WIDTH, m_Piano->m_nKeyboardY2);

	tempImage.destroy();
}


//Detects Changes using Colour Differences
void CPianoPlaying::detectChange_Colour(IplImage *pImg, IplImage *pImg1, 
										IplImage *pImb_dI, int nThresh)
{
	unsigned char r, g, b, r1, g1, b1, rDiff, gDiff, bDiff, componentDiff;
	int x, y;
	int height, width;
	
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
	
/* Dmitry: Test first line ??? Ad-hoc:*/
			if ((componentDiff > nThresh))
				pImb_dI->imageData[(y*width)+(x)] = (unsigned char)255;
			else if ((rDiff>nThresh) && (gDiff>nThresh) && (bDiff>nThresh))
				pImb_dI->imageData[(y*width)+(x)] = (unsigned char)255;
			else		
				pImb_dI->imageData[(y*width)+(x)] = 0;
		}
	}

	//Blurs to Remove Noise
	CIplImage tempImage;
	tempImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	cvSmooth(pImb_dI, tempImage.getIplImage(), CV_MEDIAN, 3, 3, 0);
	cvCopy(tempImage.getIplImage(), pImb_dI, NULL);
	tempImage.destroy();
}

//CalculateAverageColour - Calculates Average RGB Colour within the given Mask
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


//AddAverageColour - Adds Only Pixels of Given Average Colour, within Threshold,
//					 to the Mask Image
void CPianoPlaying::AddAverageColour(IplImage *pImg, IplImage *pImg1, 
											unsigned char avgR, unsigned char avgG, 
											unsigned char avgB, int threshold)
{
	unsigned char r, g, b;
	unsigned char avgIntensity, intensity;
	unsigned char rDiff, gDiff, bDiff;
	unsigned char componentDiff;
	int x, y;
	int height, width;
	
	height = pImg->height;
	width = pImg->width;

	avgIntensity = unsigned char(float(avgR + avgG + avgB)/3);

	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			r = pImg->imageData[(y*3*width)+(x*3)+2];
			g = pImg->imageData[(y*3*width)+(x*3)+1];
			b = pImg->imageData[(y*3*width)+(x*3)];
			intensity = unsigned char(float(r + g + b)/3);
			
			rDiff = abs(r - avgR);
			gDiff = abs(g - avgG);
			bDiff = abs(b - avgB);

			//For Shade Detection
			componentDiff = abs(rDiff - gDiff) + abs(rDiff - bDiff) + abs(gDiff - bDiff);

			if ((unsigned char)m_imb_dI_colour.getIplImage()->imageData[(y*width)+x] == 255)
			{
				if (abs(intensity - avgIntensity) < threshold)
					pImg1->imageData[(y*width)+x] = (unsigned char)255;
				else
					pImg1->imageData[(y*width)+x] = 0;
			}
		}
	}
}

//DetectEdges - Detects Horizontal and Vertical Edges combined
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
				pImg1->imageData[(y*width)+x] = (unsigned char)255;
			else
				pImg1->imageData[(y*width)+x] = 0;
		}
	}
}

//DetectHorizontalEdges - Detects Horizontal Edges Only
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
				pImg1->imageData[(y*width)+x] = (unsigned char)255;
			else
				pImg1->imageData[(y*width)+x] = 0;
		}
	}
}

//DetectVerticalEdges - Detects Vertical Edges Only
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
				pImg1->imageData[(y*width)+x] = (unsigned char)255;
			else
				pImg1->imageData[(y*width)+x] = 0;
		}
	}
}

//LocateHands - Locates New Hands in the Image using Blob Separation
void CPianoPlaying::LocateHands(CIplImage *image, CPianoHand *hands, int *numHands)
{
	int i, j;
	int x1, y1, x2, y2;

	*numHands=0;

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

	tempImage.destroy();
/*
	float totalIntersect;
	float totalArea;

	//If there are less detected hands, than previously existed
	if ((*numHands) < m_nNumHands)
	{
 		for (i=0; i < m_nNumHands; i++)	//Through all existing hands
		{
			for (j=0; j < (*numHands); j++)	//Through discovered hands
			{
				//Get coordinates of intersecting box between old hand box,
				//and new hand box
				if (m_Hand[i].m_boundingBox.x > hands[j].m_boundingBox.x)
					x1 = m_Hand[i].m_boundingBox.x;
				else
					x1 = hands[j].m_boundingBox.x;

				if (m_Hand[i].m_boundingBox.y > hands[j].m_boundingBox.y)
					y1 = m_Hand[i].m_boundingBox.y;
				else
					y1 = hands[j].m_boundingBox.y;

				if ((m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width) < (hands[j].m_boundingBox.x+hands[j].m_boundingBox.width))
					x2 = m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width;
				else
					x2 = hands[j].m_boundingBox.x+hands[j].m_boundingBox.width;

				if ((m_Hand[i].m_boundingBox.y+m_Hand[i].m_boundingBox.height) < (hands[j].m_boundingBox.y+hands[j].m_boundingBox.height))
					y2 = m_Hand[i].m_boundingBox.y+m_Hand[i].m_boundingBox.height;
				else
					y2 = hands[j].m_boundingBox.y+hands[j].m_boundingBox.height;

				//Calculate what percentage of old hand and new hand intersect
				//giving a probability that it is the same hand
				totalArea = m_Hand[i].m_boundingBox.width*m_Hand[i].m_boundingBox.height;
				totalIntersect = fabs((x2-x1)*(y2-y1))/totalArea;

				//IF most of hand intersects && they actually do intersect
				if (totalIntersect > 0.5 && (x1 < x2) && (y1 < y2))
				{
					//Create right part of box, from remainder of box
						
					//If box is partially outside the left hand side of the newbox
					//create rightbox (to split) accordingly
					if (m_Hand[i].m_boundingBox.x <= hands[j].m_boundingBox.x)
					{
						if (abs(hands[j].m_boundingBox.width - m_Hand[i].m_boundingBox.width) > 15)
						{
							hands[*numHands] = *(new CPianoHand(*numHands, 
							hands[j].m_boundingBox.x+m_Hand[i].m_boundingBox.width,
							hands[j].m_boundingBox.y,
							hands[j].m_boundingBox.x+hands[j].m_boundingBox.width,
							hands[j].m_boundingBox.y+hands[j].m_boundingBox.height));
							(*numHands)++;
						}
						
						//Set likely values for hand to split it
						hands[j].m_boundingBox.width = m_Hand[i].m_boundingBox.width;
					}
					//If box is partially outside the right hand side of the newbox
					//create leftbox (to split) accordingly
					else if ((m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width) >= (hands[j].m_boundingBox.x+hands[j].m_boundingBox.width))
					{
						if (abs(hands[j].m_boundingBox.width-m_Hand[i].m_boundingBox.width) > 25)
						{
							hands[*numHands] = *(new CPianoHand(*numHands, 
							hands[j].m_boundingBox.x,
							hands[j].m_boundingBox.y,
							hands[j].m_boundingBox.x+hands[j].m_boundingBox.width-m_Hand[i].m_boundingBox.width,
							hands[j].m_boundingBox.y+hands[j].m_boundingBox.height));
							(*numHands)++;
						}
						
						//Set likely values for hand to split it
						hands[j].m_boundingBox.width = m_Hand[i].m_boundingBox.width;
						hands[j].m_boundingBox.x = hands[j].m_boundingBox.x+hands[j].m_boundingBox.width-m_Hand[i].m_boundingBox.width;
					}
					else
					{
						//Rightside
						if (abs(hands[j].m_boundingBox.width - m_Hand[i].m_boundingBox.width) > 15)
						{
							hands[*numHands] = *(new CPianoHand(*numHands, 
							m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width,
							hands[j].m_boundingBox.y,
							hands[j].m_boundingBox.x+hands[j].m_boundingBox.width,
							hands[j].m_boundingBox.y+hands[j].m_boundingBox.height));
							(*numHands)++;
						}
						//Leftside
						if (abs(hands[j].m_boundingBox.x-m_Hand[i].m_boundingBox.x) > 15)
						{
							hands[*numHands] = *(new CPianoHand(*numHands, 
							hands[j].m_boundingBox.x,
							hands[j].m_boundingBox.y,
							m_Hand[i].m_boundingBox.x,
							hands[j].m_boundingBox.y+hands[j].m_boundingBox.height));
							(*numHands)++;
						}
						
						hands[j].m_boundingBox.x = m_Hand[i].m_boundingBox.x;
						hands[j].m_boundingBox.width = m_Hand[i].m_boundingBox.width;
					}

				}
			}
		}
	}
	//If there are more detected hands than previously detected
	else if (0 && (*numHands) > m_nNumHands)
	{
		bool matched;
		
		for (j=0; j < (*numHands); j++)	//Through discovered hands		
		{
			matched=false;

			for (i=0; i < m_nNumHands; i++)	//Through all existing hands
			{
				//Get coordinates of intersecting box between old hand box,
				//and new hand box
				if (m_Hand[i].m_boundingBox.x > hands[j].m_boundingBox.x)
					x1 = m_Hand[i].m_boundingBox.x;
				else
					x1 = hands[j].m_boundingBox.x;

				if (m_Hand[i].m_boundingBox.y > hands[j].m_boundingBox.y)
					y1 = m_Hand[i].m_boundingBox.y;
				else
					y1 = hands[j].m_boundingBox.y;

				if ((m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width) < (hands[j].m_boundingBox.x+hands[j].m_boundingBox.width))
					x2 = m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width;
				else
					x2 = hands[j].m_boundingBox.x+hands[j].m_boundingBox.width;

				if ((m_Hand[i].m_boundingBox.y+m_Hand[i].m_boundingBox.height) < (hands[j].m_boundingBox.y+hands[j].m_boundingBox.height))
					y2 = m_Hand[i].m_boundingBox.y+m_Hand[i].m_boundingBox.height;
				else
					y2 = hands[j].m_boundingBox.y+hands[j].m_boundingBox.height;

				//Calculate what percentage of old hand and new hand intersect
				//giving a probability that it is the same hand
				totalArea = m_Hand[i].m_boundingBox.width*m_Hand[i].m_boundingBox.height;
				totalIntersect = fabs((x2-x1)*(y2-y1))/totalArea;

				//IF most of hand intersects && they actually do intersect
				if (totalIntersect > 0.5 && (x1 < x2) && (y1 < y2))
				{
					//Create right part of box, from remainder of box
						if (abs(m_Hand[i].m_boundingBox.width - hands[j].m_boundingBox.width) > 15)
						{
							m_Hand[m_nNumHands] = *(new CPianoHand(m_nNumHands, 
							hands[j].m_boundingBox.x+m_Hand[i].m_boundingBox.width,
							hands[j].m_boundingBox.y,
							hands[j].m_boundingBox.x+hands[j].m_boundingBox.width,
							hands[j].m_boundingBox.y+hands[j].m_boundingBox.height));
							
							(m_nNumHands)++;
						}

						//Create left part of box, from remainder of box
						if (abs(hands[j].m_boundingBox.x - m_Hand[i].m_boundingBox.x) > 15)
						{
							m_Hand[m_nNumHands] = *(new CPianoHand(m_nNumHands, 
							hands[j].m_boundingBox.x,
							hands[j].m_boundingBox.y,
							m_Hand[i].m_boundingBox.x,
							hands[j].m_boundingBox.y+hands[j].m_boundingBox.height));
							
							(m_nNumHands)++;
						}

						//Set likely values for hand to split it
						m_Hand[i].m_boundingBox.width = hands[j].m_boundingBox.width;
			//			m_Hand[i].m_boundingBox.x = hands[j].m_boundingBox.x;

						matched=true;
				}
			}

			if (matched == false)
			{
				m_Hand[m_nNumHands] = *(new CPianoHand(m_nNumHands, 
							hands[j].m_boundingBox.x,
							hands[j].m_boundingBox.y,
							hands[j].m_boundingBox.x+hands[j].m_boundingBox.width,
							hands[j].m_boundingBox.y+hands[j].m_boundingBox.height));
				m_nNumHands++;
			}
		}
	}
	*/
}


//////////////////////////////////////////////////
///////////DRAWING FUNCTIONS/////////////////////
//////////////////////////////////////////////////
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
		if ((dx >= 0) && (dx < IMAGE_WIDTH) && (dy >= 0) && (dy < IMAGE_HEIGHT))
		{
			int q;
			for (q=0; q<bpp; q++)
				pImg->imageData[(dy*IMAGE_WIDTH*bpp) + (dx*bpp) + q] = (unsigned char)255;
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

void CPianoPlaying::DrawLineRad(IplImage *pImg, int x, float theta)
{
	int x2 = x + int(IMAGE_HEIGHT/tan(theta));
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
		if ((dx > 0) && (dx < IMAGE_WIDTH))
			pImg->imageData[(dy*IMAGE_WIDTH) + dx] = (unsigned char)255;
		else
			t = 1.0; //END LOOP
        t += s; 
    }
}

//////////////////////////////////////////////////
///////////FILLING FUNCTIONS/////////////////////
//////////////////////////////////////////////////
void CPianoPlaying::ClearImage(IplImage *pImg)
{
	int x, y;

	for (x=0; x < IMAGE_WIDTH; x++)
	{
		for (y=0; y < IMAGE_HEIGHT; y++)
			pImg->imageData[(y*IMAGE_WIDTH) + x] = 0;
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
				pImg->imageData[(j*IMAGE_WIDTH)+(i)] = 0;
		}
	}
}

//////////////////////////////////////////////////
///////////GENERAL PURPOSE FUNCTIONS/////////////////////
//////////////////////////////////////////////////
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

//SelectHand - Selects which Hand intersects with rect
int CPianoPlaying::SelectHand(CvRect *rect)
{
	int i;
	for (i=0; i < m_nNumHands; i++)
	{
		//IF Bounding Boxes Match
		if ((rect->x > m_Hand[i].m_boundingBox.x && rect->x < (m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width)) ||
			((rect->x+rect->width) > m_Hand[i].m_boundingBox.x && (rect->x+rect->width) < (m_Hand[i].m_boundingBox.x+m_Hand[i].m_boundingBox.width)))
		{
			//Colour In Selected Hand
			int x, y;
			for (x=0; x < IMAGE_WIDTH; x++)
			{
				for (y=0; y < IMAGE_HEIGHT; y++)
				{
					//IF Hand Mask is lit, and Finger Lines Mask is not
					if ((unsigned char)m_Hand[i].m_handsImage.getIplImage()->imageData[y*IMAGE_WIDTH+x] > 0 && ((unsigned char)m_Hand[i].m_edgeImage.getIplImage()->imageData[y*IMAGE_WIDTH+x] != 255))
					{
						m_imc_displayImage.getIplImage()->imageData[(y*3*IMAGE_WIDTH)+(x*3)+2] = (unsigned char)255;
						m_imc_displayImage.getIplImage()->imageData[(y*3*IMAGE_WIDTH)+(x*3)+1] = 0;
						m_imc_displayImage.getIplImage()->imageData[(y*3*IMAGE_WIDTH)+(x*3)+0] = 0;
					}

				}
			}
			//Return Matched Hand
			return i;
		}
	}

	//IF not Matched, Return Error -1
	return -1;
}